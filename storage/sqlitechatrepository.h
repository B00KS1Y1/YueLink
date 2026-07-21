#ifndef SQLITECHATREPOSITORY_H
#define SQLITECHATREPOSITORY_H

#include "ichatrepository.h"

class QSqlDatabase;

class SqliteChatRepository final : public IChatRepository
{
public:
    SqliteChatRepository();
    ~SqliteChatRepository() override;

    [[nodiscard]] bool initialize(QString *errorMessage) override;
    [[nodiscard]] bool loadPeers(QList<Storage::PeerRecord> *peers,
                                 QString *errorMessage) override;
    [[nodiscard]] bool loadMessages(const QString &peerId,
                                    int limit,
                                    QList<Storage::MessageRecord> *messages,
                                    QString *errorMessage) override;

    [[nodiscard]] bool upsertPeer(const Network::PeerEndpoint &peer,
                                  QString *errorMessage) override;
    [[nodiscard]] bool updateConversation(const QString &peerId,
                                          const QString &lastMessage,
                                          const QDateTime &timestamp,
                                          bool incrementUnread,
                                          QString *errorMessage) override;
    [[nodiscard]] bool clearUnread(const QString &peerId,
                                   QString *errorMessage) override;

    [[nodiscard]] bool saveMessage(const Storage::MessageRecord &message,
                                   QString *errorMessage) override;
    [[nodiscard]] bool updateDeliveryStatus(const QString &peerId,
                                            const QString &messageId,
                                            const QString &status,
                                            QString *errorMessage) override;
    [[nodiscard]] bool updateFileTransfer(const QString &peerId,
                                          const QString &messageId,
                                          qreal progress,
                                          const QString &status,
                                          const QString &filePath,
                                          QString *errorMessage) override;

private:
    [[nodiscard]] QSqlDatabase database() const;
    [[nodiscard]] bool configureDatabase(QString *errorMessage);
    [[nodiscard]] bool migrateSchema(QString *errorMessage);
    [[nodiscard]] bool executeStatement(const QString &statement,
                                        QString *errorMessage) const;

    QString m_connectionName;
    QString m_databasePath;
    bool m_initialized = false;
};

#endif // SQLITECHATREPOSITORY_H
