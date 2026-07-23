#ifndef ICHATREPOSITORY_H
#define ICHATREPOSITORY_H

#include "networktypes.h"

#include <QDateTime>
#include <QList>
#include <QString>

namespace Storage
{

struct PeerRecord
{
    Network::PeerEndpoint endpoint;
    QString lastMessage;
    QDateTime lastActivity;
    int unreadCount = 0;
};

struct MessageRecord
{
    QString messageId;
    QString peerId;
    QString senderInitial;
    QString senderColor;
    QString text;
    QDateTime timestamp;
    QString deliveryStatus;
    QString messageKind = QStringLiteral("text");
    QString fileName;
    QString fileSizeText;
    QString filePath;
    qint64 fileSize = 0;
    qreal fileProgress = 0.0;
    bool fromMe = false;
};

} // namespace Storage

class IChatRepository
{
public:
    virtual ~IChatRepository() = default;

    [[nodiscard]] virtual bool initialize(QString *errorMessage) = 0;
    [[nodiscard]] virtual bool loadPeers(QList<Storage::PeerRecord> *peers,
                                         QString *errorMessage) = 0;
    [[nodiscard]] virtual bool loadMessages(const QString &peerId,
                                            int limit,
                                            QList<Storage::MessageRecord> *messages,
                                            QString *errorMessage) = 0;

    [[nodiscard]] virtual bool upsertPeer(const Network::PeerEndpoint &peer,
                                          QString *errorMessage) = 0;
    [[nodiscard]] virtual bool updateConversation(const QString &peerId,
                                                  const QString &lastMessage,
                                                  const QDateTime &timestamp,
                                                  bool incrementUnread,
                                                  QString *errorMessage) = 0;
    [[nodiscard]] virtual bool clearUnread(const QString &peerId,
                                           QString *errorMessage) = 0;

    [[nodiscard]] virtual bool saveMessage(const Storage::MessageRecord &message,
                                           QString *errorMessage) = 0;
    [[nodiscard]] virtual bool updateDeliveryStatus(const QString &peerId,
                                                    const QString &messageId,
                                                    const QString &status,
                                                    QString *errorMessage) = 0;
    [[nodiscard]] virtual bool updateFileTransfer(const QString &peerId,
                                                  const QString &messageId,
                                                  qreal progress,
                                                  const QString &status,
                                                  const QString &filePath,
                                                  QString *errorMessage) = 0;
};

#endif // ICHATREPOSITORY_H
