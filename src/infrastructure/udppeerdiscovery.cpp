#include "udppeerdiscovery.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QSet>
#include <QStringList>

#include <QsLog.h>

namespace
{
constexpr auto ProtocolName = "YueLink";
constexpr int ProtocolVersion = 2;

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
        QLOG_DEBUG() << QStringLiteral("已经在运行，忽略启动请求。");
        return true;
    }
    if (identity.deviceId.isEmpty() || identity.displayName.isEmpty() || tcpPort == 0)
    {
        QLOG_ERROR() << QStringLiteral("局域网发现参数无效。");
        setLastError(tr("局域网发现参数无效。"));
        return false;
    }

    const auto bindMode = QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint;
    if (!m_socket.bind(QHostAddress::AnyIPv4, DiscoveryPort, bindMode))
    {
        QLOG_ERROR() << QStringLiteral("[网络.发现] 绑定 UDP 端口失败 端口=") << DiscoveryPort
                               << QStringLiteral("原因=") << m_socket.errorString();
        setLastError(tr("无法监听 UDP 发现端口 %1：%2").arg(DiscoveryPort).arg(m_socket.errorString()));
        return false;
    }

    m_identity = identity;
    m_tcpPort = tcpPort;
    m_running = true;
    setLastError({});
    m_heartbeatTimer.start();
    announce();
    QLOG_INFO() << QStringLiteral("[网络.发现] UDP 发现服务已启动 端口=") << DiscoveryPort
                          << QStringLiteral("TCP端口=") << tcpPort;
    return true;
}

void UdpPeerDiscovery::stop()
{
    if (!m_running)
    {
        return;
    }

    QLOG_INFO() << QStringLiteral("[网络.发现] 正在停止 UDP 发现服务 已跟踪好友数=") << m_lastSeenByPeer.size();
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
    QLOG_INFO() << QStringLiteral("[网络.发现] UDP 发现服务已停止");
}

void UdpPeerDiscovery::updateIdentity(const Network::LocalIdentity &identity)
{
    m_identity = identity;
    if (m_running)
    {
        announce();
    }
    QLOG_INFO() << QStringLiteral("[网络.发现] 本机身份信息已更新");
}

void UdpPeerDiscovery::announce()
{
    sendPresence(QStringLiteral("presence"));
}

void UdpPeerDiscovery::probe()
{
    sendPresence(QStringLiteral("probe"));
}

void UdpPeerDiscovery::recordPeerActivity(const QString &peerId)
{
    if (m_running && !peerId.isEmpty() && peerId != m_identity.deviceId)
    {
        m_lastSeenByPeer.insert(peerId, QDateTime::currentMSecsSinceEpoch());
        QLOG_TRACE() << QStringLiteral("[网络.发现] 已记录好友活动 好友标识=") << peerId;
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
            QLOG_TRACE() << QStringLiteral("[网络.发现] 已忽略格式错误的 UDP 数据报 地址=")
                                   << datagram.senderAddress().toString();
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
            QLOG_TRACE() << QStringLiteral("[网络.发现] 已忽略无效或来自本机的在线通告 地址=")
                                   << datagram.senderAddress().toString();
            continue;
        }

        const QString type = object.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("goodbye"))
        {
            if (m_lastSeenByPeer.remove(peer.peerId))
            {
                QLOG_INFO() << QStringLiteral("[网络.发现] 好友已通告离线 好友标识=") << peer.peerId;
                emit peerLost(peer.peerId);
            }
            continue;
        }
        if (type == QLatin1String("probe"))
        {
            sendPresenceTo(datagram.senderAddress());
        }
        if (type != QLatin1String("presence")
            && type != QLatin1String("probe"))
        {
            continue;
        }

        const bool isNewPeer = !m_lastSeenByPeer.contains(peer.peerId);
        recordPeerActivity(peer.peerId);
        if (isNewPeer)
        {
            QLOG_INFO() << QStringLiteral("[网络.发现] 已发现在线好友 好友标识=") << peer.peerId
                                  << QStringLiteral("地址=") << peer.address.toString()
                                  << QStringLiteral("端口=") << peer.tcpPort;
        }
        else
        {
            QLOG_TRACE() << QStringLiteral("[网络.发现] 收到好友心跳 好友标识=") << peer.peerId;
        }
        emit peerFound(peer);
    }
}

void UdpPeerDiscovery::sendPresence(const QString &type)
{
    const QByteArray payload = presencePayload(type);
    if (payload.isEmpty())
    {
        return;
    }

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
            QLOG_WARN() << QStringLiteral("[网络.发现] UDP 广播失败 目标地址=") << destination.toString()
                                  << QStringLiteral("原因=") << m_socket.errorString();
        }
    }
    QLOG_TRACE() << QStringLiteral("[网络.发现] 在线状态已广播 类型=") << type
                           << QStringLiteral("目标数=") << destinations.size();
}

void UdpPeerDiscovery::sendPresenceTo(const QHostAddress &address)
{
    const QByteArray payload = presencePayload(QStringLiteral("presence"));
    if (payload.isEmpty())
    {
        return;
    }
    if (m_socket.writeDatagram(payload, address, DiscoveryPort) < 0)
    {
        QLOG_WARN() << QStringLiteral("[网络.发现] UDP 发现回应失败 目标地址=") << address.toString()
                              << QStringLiteral("原因=") << m_socket.errorString();
    }
}

QByteArray UdpPeerDiscovery::presencePayload(const QString &type) const
{
    if (!m_running || m_tcpPort == 0)
    {
        return {};
    }

    const QJsonObject object{{QStringLiteral("app"), QString::fromLatin1(ProtocolName)},
                             {QStringLiteral("version"), ProtocolVersion},
                             {QStringLiteral("type"), type},
                             {QStringLiteral("id"), m_identity.deviceId},
                             {QStringLiteral("name"), m_identity.displayName},
                             {QStringLiteral("tcpPort"), m_tcpPort},
                             {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}};
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
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
        QLOG_INFO() << QStringLiteral("[网络.发现] 好友已超时离线 好友标识=") << peerId;
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
        QLOG_ERROR() << QStringLiteral("[网络.发现] 服务错误 原因=") << error;
        emit errorOccurred(error);
    }
}
