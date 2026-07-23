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
        spdlog::debug("[network.discovery] start ignored because discovery is already running");
        return true;
    }
    if (identity.deviceId.isEmpty() || identity.displayName.isEmpty() || tcpPort == 0)
    {
        spdlog::error("[network.discovery] invalid startup parameters tcp_port={}", tcpPort);
        setLastError(tr("局域网发现参数无效。"));
        return false;
    }

    const auto bindMode = QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint;
    if (!m_socket.bind(QHostAddress::AnyIPv4, DiscoveryPort, bindMode))
    {
        spdlog::error("[network.discovery] failed to bind UDP port={} reason={}", DiscoveryPort, m_socket.errorString().toUtf8().toStdString());
        setLastError(tr("无法监听 UDP 发现端口 %1：%2").arg(DiscoveryPort).arg(m_socket.errorString()));
        return false;
    }

    m_identity = identity;
    m_tcpPort = tcpPort;
    m_running = true;
    setLastError({});
    m_heartbeatTimer.start();
    announce();
    spdlog::info("[network.discovery] UDP discovery started port={} tcp_port={}", DiscoveryPort, tcpPort);
    return true;
}

void UdpPeerDiscovery::stop()
{
    if (!m_running)
    {
        return;
    }

    spdlog::info("[network.discovery] stopping UDP discovery tracked_peers={}", m_lastSeenByPeer.size());
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
    spdlog::info("[network.discovery] UDP discovery stopped");
}

void UdpPeerDiscovery::updateIdentity(const Network::LocalIdentity &identity)
{
    m_identity = identity;
    if (m_running)
    {
        announce();
    }
    spdlog::info("[network.discovery] local identity updated");
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
        spdlog::trace("[network.discovery] peer activity recorded peer_id={}", peerId.toUtf8().toStdString());
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
            spdlog::trace("[network.discovery] ignored malformed UDP datagram address={}", datagram.senderAddress().toString().toStdString());
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
            spdlog::trace("[network.discovery] ignored invalid or local presence address={}", datagram.senderAddress().toString().toStdString());
            continue;
        }

        const QString type = object.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("goodbye"))
        {
            if (m_lastSeenByPeer.remove(peer.peerId))
            {
                spdlog::info("[network.discovery] peer announced departure peer_id={}", peer.peerId.toUtf8().toStdString());
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
            spdlog::info("[network.discovery] peer presence discovered peer_id={} address={} port={}",
                         peer.peerId.toUtf8().toStdString(),
                         peer.address.toString().toStdString(),
                         peer.tcpPort);
        }
        else
        {
            spdlog::trace("[network.discovery] peer heartbeat peer_id={}", peer.peerId.toUtf8().toStdString());
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
            spdlog::warn("[network.discovery] UDP broadcast failed destination={} reason={}",
                         destination.toString().toStdString(),
                         m_socket.errorString().toUtf8().toStdString());
        }
    }
    spdlog::trace("[network.discovery] presence broadcast type={} destinations={}", type.toUtf8().toStdString(), destinations.size());
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
        spdlog::info("[network.discovery] peer expired peer_id={}", peerId.toUtf8().toStdString());
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
        spdlog::error("[network.discovery] service error: {}", error.toUtf8().toStdString());
        emit errorOccurred(error);
    }
}
