#ifndef IPEERDISCOVERY_H
#define IPEERDISCOVERY_H

#include "networktypes.h"

#include <QObject>

class IPeerDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit IPeerDiscovery(QObject *parent = nullptr)
    : QObject(parent)
    {
    }

    ~IPeerDiscovery() override = default;

    [[nodiscard]] virtual bool start(const Network::LocalIdentity &identity, quint16 tcpPort) = 0;
    virtual void stop() = 0;
    virtual void updateIdentity(const Network::LocalIdentity &identity) = 0;
    virtual void announce() = 0;
    virtual void recordPeerActivity(const QString &peerId) = 0;
    [[nodiscard]] virtual bool isRunning() const = 0;
    [[nodiscard]] virtual QString lastError() const = 0;

signals:
    void peerFound(const Network::PeerEndpoint &peer);
    void peerLost(const QString &peerId);
    void errorOccurred(const QString &message);
};

#endif // IPEERDISCOVERY_H
