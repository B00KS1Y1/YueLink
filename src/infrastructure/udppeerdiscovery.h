#ifndef UDPPEERDISCOVERY_H
#define UDPPEERDISCOVERY_H

#include "core/ipeerdiscovery.h"

#include <QHash>
#include <QTimer>
#include <QUdpSocket>

class UdpPeerDiscovery final : public IPeerDiscovery
{
    Q_OBJECT

public:
    explicit UdpPeerDiscovery(QObject *parent = nullptr);

    [[nodiscard]] bool start(const Network::LocalIdentity &identity, quint16 tcpPort) override;
    void stop() override;
    void updateIdentity(const Network::LocalIdentity &identity) override;
    void announce() override;
    void recordPeerActivity(const QString &peerId) override;
    [[nodiscard]] bool isRunning() const override;
    [[nodiscard]] QString lastError() const override;

private:
    static constexpr quint16 DiscoveryPort = 45454;
    static constexpr qint64 PeerTimeoutMs = 7000;

    void readPendingDatagrams();
    void sendPresence(const QString &type);
    void expirePeers();
    void setLastError(const QString &error);

    QUdpSocket m_socket;
    QTimer m_heartbeatTimer;
    QHash<QString, qint64> m_lastSeenByPeer;
    Network::LocalIdentity m_identity;
    QString m_lastError;
    quint16 m_tcpPort = 0;
    bool m_running = false;
};

#endif // UDPPEERDISCOVERY_H
