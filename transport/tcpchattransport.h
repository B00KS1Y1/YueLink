#ifndef TCPCHATTRANSPORT_H
#define TCPCHATTRANSPORT_H

#include "ichattransport.h"

#include <QHash>
#include <QQueue>
#include <QSet>
#include <QTcpServer>
#include <QTimer>

class QJsonObject;
class QTcpSocket;

class TcpChatTransport final : public IChatTransport
{
    Q_OBJECT

public:
    explicit TcpChatTransport(QObject *parent = nullptr);
    ~TcpChatTransport() override;

    [[nodiscard]] bool start(const Network::LocalIdentity &identity) override;
    void stop() override;
    void updateIdentity(const Network::LocalIdentity &identity) override;
    [[nodiscard]] bool isRunning() const override;
    [[nodiscard]] quint16 listeningPort() const override;
    [[nodiscard]] QString lastError() const override;

    void sendText(const Network::PeerEndpoint &peer,
                  const QString &messageId,
                  const QString &text,
                  const QDateTime &timestamp) override;
    [[nodiscard]] bool sendFile(const Network::PeerEndpoint &peer,
                                const QUrl &fileUrl,
                                QString *errorMessage) override;
    [[nodiscard]] bool cancelFileTransfer(const QString &peerId,
                                          const QString &transferId) override;

private:
    static constexpr qint64 FileTransferTimeoutMs = 30000;

    struct OutgoingFileTransfer;
    struct IncomingFileTransfer;

    void acceptConnections();
    void readIncomingData(QTcpSocket *socket);
    void handleIncomingText(const QJsonObject &object, QTcpSocket *socket);
    bool handleIncomingFileHeader(const QJsonObject &object, QTcpSocket *socket);
    bool consumeIncomingFile(QTcpSocket *socket, QByteArray &buffer);
    void failIncomingFile(QTcpSocket *socket,
                          const QString &reason,
                          bool cancelled = false);

    void finishOutgoingText(QTcpSocket *socket);
    void failOutgoingText(QTcpSocket *socket, const QString &reason);
    void pumpOutgoingFile(QTcpSocket *socket);
    void finishOutgoingFile(QTcpSocket *socket);
    void failOutgoingFile(QTcpSocket *socket,
                          const QString &reason,
                          bool cancelled = false);
    void expireFileTransfers();

    [[nodiscard]] Network::PeerEndpoint incomingPeer(const QJsonObject &object,
                                                     QTcpSocket *socket) const;
    [[nodiscard]] bool rememberEventId(const QString &eventId);
    [[nodiscard]] QString uniqueReceivePath(const QString &fileName) const;
    static QDateTime timestampFrom(const QJsonObject &object);
    void setLastError(const QString &error);

    QTcpServer m_server;
    QTimer m_transferTimer;
    QHash<QTcpSocket *, QByteArray> m_incomingBuffers;
    QHash<QTcpSocket *, IncomingFileTransfer *> m_incomingFiles;
    QHash<QTcpSocket *, OutgoingFileTransfer *> m_outgoingFiles;
    QSet<QTcpSocket *> m_outgoingTextSockets;
    QSet<QString> m_receivedEventIds;
    QQueue<QString> m_receivedEventOrder;
    Network::LocalIdentity m_identity;
    QString m_lastError;
    bool m_running = false;
};

#endif // TCPCHATTRANSPORT_H
