#include "udppeerdiscovery.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QSet>
#include <QStringList>

#include <spdlog/spdlog.h>

namespace
{
constexpr auto ProtocolName = "YueLink";
constexpr int ProtocolVersion = 1;

bool isUsableInterface(QNetworkInterface::InterfaceFlags flags)
{
    return flags.testFlag(QNetworkInterface::IsUp) && flags.testFlag(QNetworkInterface::IsRunning) && !flags.testFlag(QNetworkInterface::IsLoopBack);
}
} // namespace

UdpPeerDiscovery::UdpPeerDiscovery(QObject *parent)
: IPeerDiscovery(parent)
{
    m_heartbeatTimer.setInterval(2000);
    m_heartbeatTimer.setTimerType(Qt::CoarseTimer);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, [this]() {
        announce();
        expirePeers();
    });
    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpPeerDiscovery::readPendingDatagrams);
}

bool UdpPeerDiscovery::start(const Network::LocalIdentity &identity, quint16 tcpPort)
{
    if (m_running)
    {
        spdlog::debug("已经在运行，忽略启动请求。");
        return true;
    }
    if (identity.deviceId.isEmpty() || identity.displayName.isEmpty() || tcpPort == 0)
    {
        spdlog::error("局域网发现参数无效。");
        setLastError(tr("局域网发现参数无效。"));
        return false;
    }

    const auto bindMode = QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint;
    if (!m_socket.bind(QHostAddress::AnyIPv4, DiscoveryPort, bindMode))
    {
        spdlog::error("[网络.发现] 绑定 UDP 端口失败 端口={} 原因={}", DiscoveryPort, m_socket.errorString().toUtf8().toStdString());
        setLastError(tr("无法监听 UDP 发现端口 %1：%2").arg(DiscoveryPort).arg(m_socket.errorString()));
        return false;
    }

    m_identity = identity;
    m_tcpPort = tcpPort;
    m_running = true;
    setLastError({});
    m_heartbeatTimer.start();
    announce();
    spdlog::info("[网络.发现] UDP 发现服务已启动 端口={} TCP端口={}", DiscoveryPort, tcpPort);
    return true;
}

void UdpPeerDiscovery::stop()
{
    if (!m_running)
    {
        return;
    }

    spdlog::info("[网络.发现] 正在停止 UDP 发现服务 已跟踪好友数={}", m_lastSeenByPeer.size());
    m_heartbeatTimer.stop();
    sendPresence(QStringLiteral("goodbye"));
    m_socket.close();
    const QStringList peerIds = m_lastSeenByPeer.keys();
    m_lastSeenByPeer.clear();
    for (const QString &peerId : peerIds)
    {
        emit peerLost(peerId);
    }
    m_tcpPort = 0;
    m_running = false;
    spdlog::info("[网络.发现] UDP 发现服务已停止");
}

void UdpPeerDiscovery::updateIdentity(const Network::LocalIdentity &identity)
{
    m_identity = identity;
    if (m_running)
    {
        announce();
    }
    spdlog::info("[网络.发现] 本机身份信息已更新");
}

void UdpPeerDiscovery::announce()
{
    sendPresence(QStringLiteral("presence"));
}

void UdpPeerDiscovery::recordPeerActivity(const QString &peerId)
{
    if (m_running && !peerId.isEmpty() && peerId != m_identity.deviceId)
    {
        m_lastSeenByPeer.insert(peerId, QDateTime::currentMSecsSinceEpoch());
        spdlog::trace("[网络.发现] 已记录好友活动 好友标识={}", peerId.toUtf8().toStdString());
    }
}

bool UdpPeerDiscovery::isRunning() const
{
    return m_running;
}

QString UdpPeerDiscovery::lastError() const
{
    return m_lastError;
}

void UdpPeerDiscovery::readPendingDatagrams()
{
    while (m_socket.hasPendingDatagrams())
    {
        const QNetworkDatagram datagram = m_socket.receiveDatagram();
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(datagram.data(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            spdlog::trace("[网络.发现] 已忽略格式错误的 UDP 数据报 地址={}", datagram.senderAddress().toString().toStdString());
            continue;
        }

        const QJsonObject object = document.object();
        if (object.value(QStringLiteral("app")).toString() != QLatin1String(ProtocolName) || object.value(QStringLiteral("version")).toInt() != ProtocolVersion)
        {
            continue;
        }

        Network::PeerEndpoint peer;
        peer.peerId = object.value(QStringLiteral("id")).toString();
        peer.displayName = object.value(QStringLiteral("name")).toString().trimmed().left(64);
        peer.address = datagram.senderAddress();
        const int port = object.value(QStringLiteral("tcpPort")).toInt();
        if (port > 0 && port <= 65535)
        {
            peer.tcpPort = static_cast<quint16>(port);
        }
        if (!peer.isValid() || peer.peerId == m_identity.deviceId)
        {
            spdlog::trace("[网络.发现] 已忽略无效或来自本机的在线通告 地址={}", datagram.senderAddress().toString().toStdString());
            continue;
        }

        const QString type = object.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("goodbye"))
        {
            if (m_lastSeenByPeer.remove(peer.peerId))
            {
                spdlog::info("[网络.发现] 好友已通告离线 好友标识={}", peer.peerId.toUtf8().toStdString());
                emit peerLost(peer.peerId);
            }
            continue;
        }
        if (type != QLatin1String("presence"))
        {
            continue;
        }

        const bool isNewPeer = !m_lastSeenByPeer.contains(peer.peerId);
        recordPeerActivity(peer.peerId);
        if (isNewPeer)
        {
            spdlog::info("[网络.发现] 已发现在线好友 好友标识={} 地址={} 端口={}",
                         peer.peerId.toUtf8().toStdString(),
                         peer.address.toString().toStdString(),
                         peer.tcpPort);
        }
        else
        {
            spdlog::trace("[网络.发现] 收到好友心跳 好友标识={}", peer.peerId.toUtf8().toStdString());
        }
        emit peerFound(peer);
    }
}

void UdpPeerDiscovery::sendPresence(const QString &type)
{
    if (!m_running || m_tcpPort == 0)
    {
        return;
    }

    const QJsonObject object{{QStringLiteral("app"), QString::fromLatin1(ProtocolName)},
                             {QStringLiteral("version"), ProtocolVersion},
                             {QStringLiteral("type"), type},
                             {QStringLiteral("id"), m_identity.deviceId},
                             {QStringLiteral("name"), m_identity.displayName},
                             {QStringLiteral("tcpPort"), m_tcpPort},
                             {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}};
    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Compact);

    QSet<QHostAddress> destinations;
    destinations.insert(QHostAddress::Broadcast);
    for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces())
    {
        if (!isUsableInterface(interface.flags()))
        {
            continue;
        }
        for (const QNetworkAddressEntry &entry : interface.addressEntries())
        {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.broadcast().isNull())
            {
                destinations.insert(entry.broadcast());
            }
        }
    }

    for (const QHostAddress &destination : destinations)
    {
        if (m_socket.writeDatagram(payload, destination, DiscoveryPort) < 0)
        {
            spdlog::warn("[网络.发现] UDP 广播失败 目标地址={} 原因={}",
                         destination.toString().toStdString(),
                         m_socket.errorString().toUtf8().toStdString());
        }
    }
    spdlog::trace("[网络.发现] 在线状态已广播 类型={} 目标数={}", type.toUtf8().toStdString(), destinations.size());
}

void UdpPeerDiscovery::expirePeers()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    QStringList expired;
    for (auto iterator = m_lastSeenByPeer.cbegin(); iterator != m_lastSeenByPeer.cend(); ++iterator)
    {
        if (nowMs - iterator.value() > PeerTimeoutMs)
        {
            expired.append(iterator.key());
        }
    }
    for (const QString &peerId : expired)
    {
        m_lastSeenByPeer.remove(peerId);
        spdlog::info("[网络.发现] 好友已超时离线 好友标识={}", peerId.toUtf8().toStdString());
        emit peerLost(peerId);
    }
}

void UdpPeerDiscovery::setLastError(const QString &error)
{
    if (m_lastError == error)
    {
        return;
    }
    m_lastError = error;
    if (!error.isEmpty())
    {
        spdlog::error("[网络.发现] 服务错误 原因={}", error.toUtf8().toStdString());
        emit errorOccurred(error);
    }
}
