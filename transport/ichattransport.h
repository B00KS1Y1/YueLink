#ifndef ICHATTRANSPORT_H
#define ICHATTRANSPORT_H

#include "domain/networktypes.h"

#include <QDateTime>
#include <QObject>
#include <QUrl>

class IChatTransport : public QObject
{
    Q_OBJECT

public:
    explicit IChatTransport(QObject *parent = nullptr)
    : QObject(parent)
    {
    }

    ~IChatTransport() override = default;

    [[nodiscard]] virtual bool start(const Network::LocalIdentity &identity) = 0;
    virtual void stop() = 0;
    virtual void updateIdentity(const Network::LocalIdentity &identity) = 0;
    [[nodiscard]] virtual bool isRunning() const = 0;
    [[nodiscard]] virtual quint16 listeningPort() const = 0;
    [[nodiscard]] virtual QString lastError() const = 0;

    virtual void sendText(const Network::PeerEndpoint &peer, const QString &messageId, const QString &text, const QDateTime &timestamp) = 0;
    [[nodiscard]] virtual bool sendFile(const Network::PeerEndpoint &peer, const QUrl &fileUrl, QString *errorMessage) = 0;
    [[nodiscard]] virtual bool cancelFileTransfer(const QString &peerId, const QString &transferId) = 0;

signals:
    void peerObserved(const Network::PeerEndpoint &peer);
    void textReceived(const Network::TextMessage &message);
    void textSent(const QString &peerId, const QString &messageId);
    void textSendFailed(const QString &peerId, const QString &messageId, const QString &reason);
    void fileTransferStarted(const Network::FileTransferInfo &transfer);
    void fileTransferProgressed(const Network::FileTransferProgress &progress);
    void fileTransferFinished(const Network::FileTransferResult &result);
    void errorOccurred(const QString &message);
};

#endif // ICHATTRANSPORT_H
