#include "sqlitechatrepository.h"

#include "infrastructure/config/configapi.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QUuid>

#include <spdlog/spdlog.h>

#include <cstddef>
#include <utility>

namespace
{
constexpr int CurrentSchemaVersion = 3;

void setError(QString *target, const QString &message)
{
    if (target)
    {
        *target = message;
    }
}

QString timestampText(const QDateTime &timestamp)
{
    const QDateTime value = timestamp.isValid() ? timestamp : QDateTime::currentDateTimeUtc();
    return value.toUTC().toString(Qt::ISODateWithMs);
}

QDateTime timestampFromText(const QString &text)
{
    const QDateTime timestamp = QDateTime::fromString(text, Qt::ISODateWithMs);
    return timestamp.isValid() ? timestamp : QDateTime::currentDateTimeUtc();
}

QString normalizedSynchronousMode(QString mode)
{
    mode = mode.trimmed().toLower();
    if (mode == QLatin1String("off") || mode == QLatin1String("normal") || mode == QLatin1String("full") || mode == QLatin1String("extra"))
    {
        return mode.toUpper();
    }
    return QStringLiteral("NORMAL");
}

Domain::Message messageFromQuery(const QSqlQuery &query)
{
    Domain::Message message;
    message.messageId = query.value(0).toString();
    message.conversationId = query.value(1).toString();
    message.senderId = query.value(2).toString();
    message.text = query.value(3).toString();
    message.timestamp = timestampFromText(query.value(4).toString());
    message.deliveryState = Domain::deliveryStateFromName(query.value(5).toString());
    message.kind = Domain::messageKindFromName(query.value(6).toString());
    message.fileName = query.value(7).toString();
    message.legacyFileSizeText = query.value(8).toString();
    message.fileSize = qMax<qint64>(0, query.value(9).toLongLong());
    message.fileProgress = qBound(0.0, query.value(10).toDouble(), 1.0);
    message.filePath = query.value(11).toString();
    return message;
}

QString messageColumns()
{
    return QStringLiteral("message_id, conversation_id, sender_id, message_text, "
                          "timestamp_utc, delivery_status, message_kind, file_name, "
                          "file_size_text, file_size_bytes, file_progress, file_path");
}
} // namespace

SqliteChatRepository::SqliteChatRepository()
: m_connectionName(QStringLiteral("YueLink.Chat.%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
}

SqliteChatRepository::~SqliteChatRepository()
{
    if (!QSqlDatabase::contains(m_connectionName))
    {
        return;
    }
    {
        QSqlDatabase connection = QSqlDatabase::database(m_connectionName, false);
        connection.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool SqliteChatRepository::initialize(QString *errorMessage)
{
    setError(errorMessage, {});
    if (m_initialized)
    {
        return true;
    }

    const Config::DatabaseConfig config = Config::get<Config::DatabaseConfig>();
    m_databasePath = QString::fromStdString(config.sqlite.file_path);
    if (!QDir().mkpath(QFileInfo(m_databasePath).absolutePath()))
    {
        setError(errorMessage, QObject::tr("无法创建数据库目录。"));
        return false;
    }

    QSqlDatabase connection = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    connection.setDatabaseName(m_databasePath);
    if (!connection.open())
    {
        setError(errorMessage, connection.lastError().text());
        return false;
    }
    if (!configureDatabase(errorMessage) || !ensureSchema(errorMessage))
    {
        connection.close();
        return false;
    }

    m_initialized = true;
    spdlog::info("[存储.SQLite] 统一会话数据库初始化完成 路径={} 架构版本={}", m_databasePath.toUtf8().toStdString(), CurrentSchemaVersion);
    return true;
}

bool SqliteChatRepository::loadPeers(QList<Domain::Peer> *peers, QString *errorMessage)
{
    if (!m_initialized || !peers)
    {
        return false;
    }
    QSqlQuery query(database());
    if (!query.exec(QStringLiteral("SELECT peer_id, display_name, address, tcp_port "
                                   "FROM peers ORDER BY display_name COLLATE NOCASE")))
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }

    QList<Domain::Peer> result;
    while (query.next())
    {
        Domain::Peer peer;
        peer.endpoint.peerId = query.value(0).toString();
        peer.endpoint.displayName = query.value(1).toString();
        peer.endpoint.address = QHostAddress(query.value(2).toString());
        const int port = query.value(3).toInt();
        if (port > 0 && port <= 65535)
        {
            peer.endpoint.tcpPort = static_cast<quint16>(port);
        }
        if (!peer.endpoint.peerId.isEmpty())
        {
            result.append(std::move(peer));
        }
    }
    *peers = std::move(result);
    return true;
}

bool SqliteChatRepository::loadConversations(QList<Domain::Conversation> *conversations, QString *errorMessage)
{
    if (!m_initialized || !conversations)
    {
        return false;
    }
    QSqlQuery query(database());
    if (!query.exec(QStringLiteral("SELECT conversation_id, kind, peer_id, title, last_message, "
                                   "last_activity_utc, unread_count, member_count FROM conversations "
                                   "ORDER BY last_activity_utc DESC")))
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }

    QList<Domain::Conversation> result;
    while (query.next())
    {
        Domain::Conversation conversation;
        conversation.conversationId = query.value(0).toString();
        conversation.kind = Domain::conversationKindFromName(query.value(1).toString());
        conversation.peerId = query.value(2).toString();
        conversation.title = query.value(3).toString();
        conversation.lastMessage = query.value(4).toString();
        conversation.lastActivity = timestampFromText(query.value(5).toString());
        conversation.unreadCount = qMax(0, query.value(6).toInt());
        conversation.memberCount = qMax(0, query.value(7).toInt());
        if (!conversation.conversationId.isEmpty())
        {
            result.append(std::move(conversation));
        }
    }
    *conversations = std::move(result);
    return true;
}

bool SqliteChatRepository::loadGroups(QList<Domain::Group> *groups, QString *errorMessage)
{
    if (!m_initialized || !groups)
    {
        return false;
    }
    QSqlQuery query(database());
    if (!query.exec(QStringLiteral("SELECT group_id, name, owner_id, revision, "
                                   "created_at_utc FROM groups")))
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }

    QList<Domain::Group> result;
    while (query.next())
    {
        Domain::Group group;
        group.groupId = query.value(0).toString();
        group.name = query.value(1).toString();
        group.ownerId = query.value(2).toString();
        group.revision = query.value(3).toULongLong();
        group.createdAt = timestampFromText(query.value(4).toString());

        QSqlQuery memberQuery(database());
        memberQuery.prepare(QStringLiteral("SELECT peer_id, display_name, role FROM group_members "
                                           "WHERE group_id = :group_id ORDER BY role DESC, display_name COLLATE NOCASE"));
        memberQuery.bindValue(QStringLiteral(":group_id"), group.groupId);
        if (!memberQuery.exec())
        {
            setError(errorMessage, memberQuery.lastError().text());
            return false;
        }
        while (memberQuery.next())
        {
            Domain::GroupMember member;
            member.peerId = memberQuery.value(0).toString();
            member.displayName = memberQuery.value(1).toString();
            member.role = Domain::groupRoleFromName(memberQuery.value(2).toString());
            group.members.append(std::move(member));
        }
        result.append(std::move(group));
    }
    *groups = std::move(result);
    return true;
}

bool SqliteChatRepository::loadMessages(const QString &conversationId, int limit, QList<Domain::Message> *messages, QString *errorMessage)
{
    if (!m_initialized || !messages || conversationId.isEmpty())
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT %1 FROM messages WHERE id IN "
                                 "(SELECT id FROM messages WHERE conversation_id = :id "
                                 "ORDER BY id DESC LIMIT :limit) ORDER BY id ASC")
                      .arg(messageColumns()));
    query.bindValue(QStringLiteral(":id"), conversationId);
    query.bindValue(QStringLiteral(":limit"), qBound(1, limit, 5000));
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    QList<Domain::Message> result;
    while (query.next())
    {
        result.append(messageFromQuery(query));
    }
    *messages = std::move(result);
    return true;
}

bool SqliteChatRepository::loadMessage(const QString &messageId, Domain::Message *message, QString *errorMessage)
{
    if (!m_initialized || !message || messageId.isEmpty())
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT %1 FROM messages WHERE message_id = :id").arg(messageColumns()));
    query.bindValue(QStringLiteral(":id"), messageId);
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    if (!query.next())
    {
        return false;
    }
    *message = messageFromQuery(query);
    return true;
}

bool SqliteChatRepository::loadDeliveries(QList<Domain::MessageDelivery> *deliveries, QString *errorMessage)
{
    if (!m_initialized || !deliveries)
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT message_id, conversation_id, recipient_id, state, error_message, "
                                 "last_attempt_utc FROM message_deliveries"));
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    QList<Domain::MessageDelivery> result;
    while (query.next())
    {
        Domain::MessageDelivery delivery;
        delivery.messageId = query.value(0).toString();
        delivery.conversationId = query.value(1).toString();
        delivery.recipientId = query.value(2).toString();
        delivery.state = Domain::deliveryStateFromName(query.value(3).toString());
        delivery.errorMessage = query.value(4).toString();
        delivery.lastAttempt = timestampFromText(query.value(5).toString());
        result.append(std::move(delivery));
    }
    *deliveries = std::move(result);
    return true;
}

bool SqliteChatRepository::upsertPeer(const Domain::Peer &peer, QString *errorMessage)
{
    if (!m_initialized || !peer.endpoint.isValid())
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO peers(peer_id, display_name, address, tcp_port, updated_at_utc) "
                                 "VALUES(:id, :name, :address, :port, :updated) "
                                 "ON CONFLICT(peer_id) DO UPDATE SET display_name=excluded.display_name, "
                                 "address=excluded.address, tcp_port=excluded.tcp_port, "
                                 "updated_at_utc=excluded.updated_at_utc"));
    query.bindValue(QStringLiteral(":id"), peer.endpoint.peerId);
    query.bindValue(QStringLiteral(":name"), peer.endpoint.displayName);
    query.bindValue(QStringLiteral(":address"), peer.endpoint.address.toString());
    query.bindValue(QStringLiteral(":port"), peer.endpoint.tcpPort);
    query.bindValue(QStringLiteral(":updated"), timestampText(QDateTime{}));
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::saveConversation(const Domain::Conversation &conversation, QString *errorMessage)
{
    if (!m_initialized || conversation.conversationId.isEmpty() || conversation.title.isEmpty())
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO conversations(conversation_id, kind, peer_id, title, "
                                 "last_message, last_activity_utc, unread_count, member_count) "
                                 "VALUES(:id, :kind, :peer, :title, :last_message, :activity, :unread, :members) "
                                 "ON CONFLICT(conversation_id) DO UPDATE SET kind=excluded.kind, "
                                 "peer_id=excluded.peer_id, title=excluded.title, "
                                 "last_message=excluded.last_message, last_activity_utc=excluded.last_activity_utc, "
                                 "unread_count=excluded.unread_count, member_count=excluded.member_count"));
    query.bindValue(QStringLiteral(":id"), conversation.conversationId);
    query.bindValue(QStringLiteral(":kind"), Domain::conversationKindName(conversation.kind));
    query.bindValue(QStringLiteral(":peer"), conversation.peerId);
    query.bindValue(QStringLiteral(":title"), conversation.title);
    query.bindValue(QStringLiteral(":last_message"), conversation.lastMessage);
    query.bindValue(QStringLiteral(":activity"), timestampText(conversation.lastActivity));
    query.bindValue(QStringLiteral(":unread"), qMax(0, conversation.unreadCount));
    query.bindValue(QStringLiteral(":members"), qMax(0, conversation.memberCount));
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::saveGroup(const Domain::Group &group, QString *errorMessage)
{
    if (!m_initialized || group.groupId.isEmpty() || group.name.isEmpty() || group.ownerId.isEmpty() || group.members.size() < 2)
    {
        return false;
    }
    QSqlDatabase connection = database();
    if (!connection.transaction())
    {
        setError(errorMessage, connection.lastError().text());
        return false;
    }

    QSqlQuery groupQuery(connection);
    groupQuery.prepare(QStringLiteral("INSERT INTO groups(group_id, name, owner_id, revision, created_at_utc) "
                                      "VALUES(:id, :name, :owner, :revision, :created) "
                                      "ON CONFLICT(group_id) DO UPDATE SET name=excluded.name, "
                                      "owner_id=excluded.owner_id, revision=excluded.revision"));
    groupQuery.bindValue(QStringLiteral(":id"), group.groupId);
    groupQuery.bindValue(QStringLiteral(":name"), group.name);
    groupQuery.bindValue(QStringLiteral(":owner"), group.ownerId);
    groupQuery.bindValue(QStringLiteral(":revision"), static_cast<qulonglong>(group.revision));
    groupQuery.bindValue(QStringLiteral(":created"), timestampText(group.createdAt));
    if (!groupQuery.exec())
    {
        connection.rollback();
        setError(errorMessage, groupQuery.lastError().text());
        return false;
    }

    QSqlQuery deleteMembers(connection);
    deleteMembers.prepare(QStringLiteral("DELETE FROM group_members WHERE group_id=:id"));
    deleteMembers.bindValue(QStringLiteral(":id"), group.groupId);
    if (!deleteMembers.exec())
    {
        connection.rollback();
        setError(errorMessage, deleteMembers.lastError().text());
        return false;
    }
    for (const Domain::GroupMember &member : group.members)
    {
        QSqlQuery memberQuery(connection);
        memberQuery.prepare(QStringLiteral("INSERT INTO group_members(group_id, peer_id, display_name, role) "
                                           "VALUES(:group, :peer, :name, :role)"));
        memberQuery.bindValue(QStringLiteral(":group"), group.groupId);
        memberQuery.bindValue(QStringLiteral(":peer"), member.peerId);
        memberQuery.bindValue(QStringLiteral(":name"), member.displayName);
        memberQuery.bindValue(QStringLiteral(":role"), Domain::groupRoleName(member.role));
        if (!memberQuery.exec())
        {
            connection.rollback();
            setError(errorMessage, memberQuery.lastError().text());
            return false;
        }
    }
    if (!connection.commit())
    {
        setError(errorMessage, connection.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::updateConversation(
    const QString &conversationId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread, QString *errorMessage)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE conversations SET last_message=:message, last_activity_utc=:activity, "
                                 "unread_count=MAX(0, unread_count+:delta) WHERE conversation_id=:id"));
    query.bindValue(QStringLiteral(":message"), lastMessage);
    query.bindValue(QStringLiteral(":activity"), timestampText(timestamp));
    query.bindValue(QStringLiteral(":delta"), incrementUnread ? 1 : 0);
    query.bindValue(QStringLiteral(":id"), conversationId);
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::clearUnread(const QString &conversationId, QString *errorMessage)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE conversations SET unread_count=0 WHERE conversation_id=:id"));
    query.bindValue(QStringLiteral(":id"), conversationId);
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::saveMessage(const Domain::Message &message, QString *errorMessage)
{
    if (!m_initialized || message.messageId.isEmpty() || message.conversationId.isEmpty() || message.senderId.isEmpty())
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO messages(message_id, conversation_id, sender_id, message_text, "
                                 "timestamp_utc, delivery_status, message_kind, file_name, file_size_text, "
                                 "file_size_bytes, file_progress, file_path) VALUES(:message, :conversation, "
                                 ":sender, :text, :timestamp, :status, :kind, :file_name, :file_size_text, "
                                 ":file_size, :progress, :path) ON CONFLICT(message_id) DO NOTHING"));
    query.bindValue(QStringLiteral(":message"), message.messageId);
    query.bindValue(QStringLiteral(":conversation"), message.conversationId);
    query.bindValue(QStringLiteral(":sender"), message.senderId);
    query.bindValue(QStringLiteral(":text"), message.text);
    query.bindValue(QStringLiteral(":timestamp"), timestampText(message.timestamp));
    query.bindValue(QStringLiteral(":status"), Domain::deliveryStateName(message.deliveryState));
    query.bindValue(QStringLiteral(":kind"), Domain::messageKindName(message.kind));
    query.bindValue(QStringLiteral(":file_name"), message.fileName);
    query.bindValue(QStringLiteral(":file_size_text"), message.legacyFileSizeText);
    query.bindValue(QStringLiteral(":file_size"), qMax<qint64>(0, message.fileSize));
    query.bindValue(QStringLiteral(":progress"), qBound(0.0, message.fileProgress, 1.0));
    query.bindValue(QStringLiteral(":path"), message.filePath);
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::updateMessageState(const QString &conversationId, const QString &messageId, Domain::DeliveryState state, QString *errorMessage)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE messages SET delivery_status=:status "
                                 "WHERE conversation_id=:conversation AND message_id=:message"));
    query.bindValue(QStringLiteral(":status"), Domain::deliveryStateName(state));
    query.bindValue(QStringLiteral(":conversation"), conversationId);
    query.bindValue(QStringLiteral(":message"), messageId);
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::updateFileTransfer(
    const QString &conversationId, const QString &messageId, qreal progress, Domain::DeliveryState state, const QString &filePath, QString *errorMessage)
{
    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE messages SET file_progress=:progress, delivery_status=:status, "
                                 "file_path=COALESCE(NULLIF(:path, ''), file_path) "
                                 "WHERE conversation_id=:conversation AND message_id=:message"));
    query.bindValue(QStringLiteral(":progress"), qBound(0.0, progress, 1.0));
    query.bindValue(QStringLiteral(":status"), Domain::deliveryStateName(state));
    query.bindValue(QStringLiteral(":path"), filePath);
    query.bindValue(QStringLiteral(":conversation"), conversationId);
    query.bindValue(QStringLiteral(":message"), messageId);
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteChatRepository::saveDelivery(const Domain::MessageDelivery &delivery, QString *errorMessage)
{
    if (!m_initialized || delivery.messageId.isEmpty() || delivery.conversationId.isEmpty() || delivery.recipientId.isEmpty())
    {
        return false;
    }
    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO message_deliveries(message_id, conversation_id, recipient_id, "
                                 "state, error_message, last_attempt_utc) VALUES(:message, :conversation, "
                                 ":recipient, :state, :error, :attempt) ON CONFLICT(message_id, recipient_id) "
                                 "DO UPDATE SET state=excluded.state, error_message=excluded.error_message, "
                                 "last_attempt_utc=excluded.last_attempt_utc"));
    query.bindValue(QStringLiteral(":message"), delivery.messageId);
    query.bindValue(QStringLiteral(":conversation"), delivery.conversationId);
    query.bindValue(QStringLiteral(":recipient"), delivery.recipientId);
    query.bindValue(QStringLiteral(":state"), Domain::deliveryStateName(delivery.state));
    query.bindValue(QStringLiteral(":error"), delivery.errorMessage);
    query.bindValue(QStringLiteral(":attempt"), timestampText(delivery.lastAttempt));
    if (!query.exec())
    {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

QSqlDatabase SqliteChatRepository::database() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

bool SqliteChatRepository::configureDatabase(QString *errorMessage)
{
    const Config::SqliteConfig config = Config::get<Config::DatabaseConfig>().sqlite;
    return executeStatement(QStringLiteral("PRAGMA busy_timeout=%1").arg(static_cast<qulonglong>(qBound<std::size_t>(1, config.busy_timeout_ms, 600000))),
                            errorMessage) &&
           executeStatement(QStringLiteral("PRAGMA foreign_keys=%1").arg(config.foreign_keys_enabled ? QStringLiteral("ON") : QStringLiteral("OFF")),
                            errorMessage) &&
           (!config.wal_enabled || executeStatement(QStringLiteral("PRAGMA journal_mode=WAL"), errorMessage)) &&
           executeStatement(QStringLiteral("PRAGMA synchronous=%1").arg(normalizedSynchronousMode(QString::fromStdString(config.synchronous))), errorMessage);
}

bool SqliteChatRepository::ensureSchema(QString *errorMessage)
{
    QSqlDatabase connection = database();
    int version = 0;
    {
        QSqlQuery versionQuery(connection);
        if (!versionQuery.exec(QStringLiteral("PRAGMA user_version")))
        {
            setError(errorMessage, QObject::tr("读取数据库架构版本失败：%1").arg(versionQuery.lastError().text()));
            return false;
        }
        if (!versionQuery.next())
        {
            setError(errorMessage, QObject::tr("读取数据库架构版本失败：查询未返回版本号。"));
            return false;
        }
        version = versionQuery.value(0).toInt();
    }
    if (!connection.transaction())
    {
        setError(errorMessage, QObject::tr("启动数据库架构事务失败：%1").arg(connection.lastError().text()));
        return false;
    }

    if (version != CurrentSchemaVersion)
    {
        const QStringList drops{QStringLiteral("DROP TABLE IF EXISTS message_deliveries"),
                                QStringLiteral("DROP TABLE IF EXISTS messages"),
                                QStringLiteral("DROP TABLE IF EXISTS group_members"),
                                QStringLiteral("DROP TABLE IF EXISTS groups"),
                                QStringLiteral("DROP TABLE IF EXISTS conversations"),
                                QStringLiteral("DROP TABLE IF EXISTS peers")};
        for (const QString &statement : drops)
        {
            QSqlQuery query(connection);
            if (!query.exec(statement))
            {
                connection.rollback();
                setError(errorMessage, query.lastError().text());
                return false;
            }
        }
    }

    const QStringList creates{QStringLiteral("CREATE TABLE IF NOT EXISTS peers("
                                             "peer_id TEXT PRIMARY KEY NOT NULL, display_name TEXT NOT NULL, "
                                             "address TEXT NOT NULL, tcp_port INTEGER NOT NULL, "
                                             "updated_at_utc TEXT NOT NULL)"),
                              QStringLiteral("CREATE TABLE IF NOT EXISTS conversations("
                                             "conversation_id TEXT PRIMARY KEY NOT NULL, kind TEXT NOT NULL, "
                                             "peer_id TEXT NOT NULL DEFAULT '', title TEXT NOT NULL, "
                                             "last_message TEXT NOT NULL DEFAULT '', last_activity_utc TEXT NOT NULL, "
                                             "unread_count INTEGER NOT NULL DEFAULT 0, member_count INTEGER NOT NULL DEFAULT 0)"),
                              QStringLiteral("CREATE TABLE IF NOT EXISTS groups("
                                             "group_id TEXT PRIMARY KEY NOT NULL, name TEXT NOT NULL, owner_id TEXT NOT NULL, "
                                             "revision INTEGER NOT NULL, created_at_utc TEXT NOT NULL, "
                                             "FOREIGN KEY(group_id) REFERENCES conversations(conversation_id) ON DELETE CASCADE)"),
                              QStringLiteral("CREATE TABLE IF NOT EXISTS group_members("
                                             "group_id TEXT NOT NULL, peer_id TEXT NOT NULL, display_name TEXT NOT NULL, "
                                             "role TEXT NOT NULL, PRIMARY KEY(group_id, peer_id), "
                                             "FOREIGN KEY(group_id) REFERENCES groups(group_id) ON DELETE CASCADE)"),
                              QStringLiteral("CREATE TABLE IF NOT EXISTS messages("
                                             "id INTEGER PRIMARY KEY AUTOINCREMENT, message_id TEXT NOT NULL UNIQUE, "
                                             "conversation_id TEXT NOT NULL, sender_id TEXT NOT NULL, "
                                             "message_text TEXT NOT NULL DEFAULT '', timestamp_utc TEXT NOT NULL, "
                                             "delivery_status TEXT NOT NULL, message_kind TEXT NOT NULL DEFAULT 'text', "
                                             "file_name TEXT NOT NULL DEFAULT '', file_size_text TEXT NOT NULL DEFAULT '', "
                                             "file_size_bytes INTEGER NOT NULL DEFAULT 0, file_progress REAL NOT NULL DEFAULT 0, "
                                             "file_path TEXT NOT NULL DEFAULT '', FOREIGN KEY(conversation_id) "
                                             "REFERENCES conversations(conversation_id) ON DELETE CASCADE)"),
                              QStringLiteral("CREATE INDEX IF NOT EXISTS idx_messages_conversation "
                                             "ON messages(conversation_id, id)"),
                              QStringLiteral("CREATE TABLE IF NOT EXISTS message_deliveries("
                                             "message_id TEXT NOT NULL, conversation_id TEXT NOT NULL, recipient_id TEXT NOT NULL, "
                                             "state TEXT NOT NULL, error_message TEXT NOT NULL DEFAULT '', "
                                             "last_attempt_utc TEXT NOT NULL, PRIMARY KEY(message_id, recipient_id), "
                                             "FOREIGN KEY(message_id) REFERENCES messages(message_id) ON DELETE CASCADE, "
                                             "FOREIGN KEY(conversation_id) REFERENCES conversations(conversation_id) ON DELETE CASCADE)"),
                              QStringLiteral("CREATE INDEX IF NOT EXISTS idx_deliveries_recipient "
                                             "ON message_deliveries(recipient_id, state)")};
    for (const QString &statement : creates)
    {
        QSqlQuery query(connection);
        if (!query.exec(statement))
        {
            connection.rollback();
            setError(errorMessage, query.lastError().text());
            return false;
        }
    }

    {
        QSqlQuery updateVersion(connection);
        if (!updateVersion.exec(QStringLiteral("PRAGMA user_version=%1").arg(CurrentSchemaVersion)))
        {
            connection.rollback();
            setError(errorMessage, QObject::tr("更新数据库架构版本失败：%1").arg(updateVersion.lastError().text()));
            return false;
        }
    }
    if (!connection.commit())
    {
        const QString commitError = connection.lastError().text();
        connection.rollback();
        setError(errorMessage, QObject::tr("提交数据库架构事务失败：%1").arg(commitError));
        return false;
    }
    return true;
}

bool SqliteChatRepository::executeStatement(const QString &statement, QString *errorMessage)
{
    QSqlQuery query(database());
    if (query.exec(statement))
    {
        return true;
    }
    setError(errorMessage, query.lastError().text());
    return false;
}
