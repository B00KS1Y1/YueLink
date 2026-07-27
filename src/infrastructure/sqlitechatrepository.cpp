#include "sqlitechatrepository.h"

#include "config/configstore.h"
#include "path.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QUuid>

#include <spdlog/spdlog.h>

#include <utility>

namespace
{
constexpr int CurrentSchemaVersion = 2;

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
    if (m_initialized)
    {
        spdlog::info("数据初始化完成，路径：{}，模式：{}", m_databasePath.toUtf8().toStdString(), CurrentSchemaVersion);
        return true;
    }

    const Config::DatabaseConfig config = Config::database.get();
    if (QString::fromStdString(config.driver).compare(QStringLiteral("sqlite"), Qt::CaseInsensitive) != 0)
    {
        spdlog::error("不支持的数据库驱动：{}", config.driver);
        return false;
    }

    const QString configuredPath = QString::fromStdString(config.sqlite.file_path).trimmed();
    if (configuredPath.isEmpty())
    {
        spdlog::error("SQLite 数据库路径不能为空。");
        return false;
    }
    m_databasePath = QDir::isAbsolutePath(configuredPath) ? QDir::cleanPath(configuredPath) : Utils::Path::dataFile(configuredPath);
    const QString directory = QFileInfo(m_databasePath).absolutePath();
    if (!QDir().mkpath(directory))
    {
        spdlog::error("无法创建数据库目录：{}", directory.toUtf8().toStdString());
        return false;
    }

    QSqlDatabase connection = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    connection.setDatabaseName(m_databasePath);
    if (!connection.open())
    {
        spdlog::error("无法打开 SQLite 数据库：{}", connection.lastError().text().toUtf8().toStdString());
        return false;
    }

    if (!configureDatabase(errorMessage) || !migrateSchema(errorMessage))
    {
        connection.close();
        return false;
    }

    m_initialized = true;
    spdlog::info("[存储.SQLite] 聊天数据库初始化完成 路径={} 架构版本={}", m_databasePath.toUtf8().toStdString(), CurrentSchemaVersion);
    return true;
}

bool SqliteChatRepository::loadPeers(QList<Storage::PeerRecord> *peers, QString *errorMessage)
{
    if (!m_initialized || !peers)
    {
        spdlog::error("聊天仓储尚未初始化。");
        return false;
    }

    QSqlQuery query(database());
    if (!query.exec(QStringLiteral("SELECT peer_id, display_name, address, tcp_port, last_message, "
                                   "COALESCE(NULLIF(last_activity_utc, ''), updated_at_utc), unread_count "
                                   "FROM peers ORDER BY COALESCE(NULLIF(last_activity_utc, ''), "
                                   "updated_at_utc) DESC")))
    {
        spdlog::error("查询好友列表失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }

    QList<Storage::PeerRecord> records;
    while (query.next())
    {
        Storage::PeerRecord record;
        record.endpoint.peerId = query.value(0).toString();
        record.endpoint.displayName = query.value(1).toString();
        record.endpoint.address = QHostAddress(query.value(2).toString());
        const int port = query.value(3).toInt();
        if (port > 0 && port <= 65535)
        {
            record.endpoint.tcpPort = static_cast<quint16>(port);
        }
        record.lastMessage = query.value(4).toString();
        record.lastActivity = timestampFromText(query.value(5).toString());
        record.unreadCount = qMax(0, query.value(6).toInt());
        if (!record.endpoint.peerId.isEmpty() && !record.endpoint.displayName.isEmpty())
        {
            records.append(std::move(record));
        }
    }
    *peers = std::move(records);
    spdlog::info("好友列表加载完成，共 {} 条记录。", records.size());
    return true;
}

bool SqliteChatRepository::loadMessages(const QString &peerId, int limit, QList<Storage::MessageRecord> *messages, QString *errorMessage)
{
    if (!m_initialized || !messages || peerId.isEmpty())
    {
        spdlog::error("聊天记录查询参数无效。");
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("SELECT message_id, peer_id, from_me, sender_initial, sender_color, "
                                 "message_text, timestamp_utc, delivery_status, message_kind, file_name, "
                                 "file_size_text, file_size_bytes, file_progress, file_path FROM messages "
                                 "WHERE id IN (SELECT id FROM messages WHERE peer_id = :peer_id "
                                 "ORDER BY id DESC LIMIT :limit) ORDER BY id ASC"));
    query.bindValue(QStringLiteral(":peer_id"), peerId);
    query.bindValue(QStringLiteral(":limit"), qBound(1, limit, 5000));
    if (!query.exec())
    {
        spdlog::error("查询聊天记录失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }

    QList<Storage::MessageRecord> records;
    while (query.next())
    {
        Storage::MessageRecord record;
        record.messageId = query.value(0).toString();
        record.peerId = query.value(1).toString();
        record.fromMe = query.value(2).toBool();
        record.senderInitial = query.value(3).toString();
        record.senderColor = query.value(4).toString();
        record.text = query.value(5).toString();
        record.timestamp = timestampFromText(query.value(6).toString());
        record.deliveryStatus = query.value(7).toString();
        record.messageKind = query.value(8).toString();
        record.fileName = query.value(9).toString();
        record.fileSizeText = query.value(10).toString();
        record.fileSize = qMax<qint64>(0, query.value(11).toLongLong());
        record.fileProgress = qBound(0.0, query.value(12).toDouble(), 1.0);
        record.filePath = query.value(13).toString();
        records.append(std::move(record));
    }
    *messages = std::move(records);
    spdlog::info("聊天记录加载完成，共 {} 条记录。", records.size());
    return true;
}

bool SqliteChatRepository::upsertPeer(const Network::PeerEndpoint &peer, QString *errorMessage)
{
    if (!m_initialized || !peer.isValid())
    {
        spdlog::error("无法保存无效的好友信息。");
        return false;
    }

    const QString now = timestampText(QDateTime::currentDateTimeUtc());
    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO peers (peer_id, display_name, address, tcp_port, "
                                 "last_message, last_activity_utc, unread_count, updated_at_utc) "
                                 "VALUES (:peer_id, :display_name, :address, :tcp_port, '', :created_at, "
                                 "0, :updated_at) "
                                 "ON CONFLICT(peer_id) DO UPDATE SET display_name = excluded.display_name, "
                                 "address = excluded.address, tcp_port = excluded.tcp_port, "
                                 "updated_at_utc = excluded.updated_at_utc"));
    query.bindValue(QStringLiteral(":peer_id"), peer.peerId);
    query.bindValue(QStringLiteral(":display_name"), peer.displayName);
    query.bindValue(QStringLiteral(":address"), peer.address.toString());
    query.bindValue(QStringLiteral(":tcp_port"), peer.tcpPort);
    query.bindValue(QStringLiteral(":created_at"), now);
    query.bindValue(QStringLiteral(":updated_at"), now);
    if (!query.exec())
    {
        spdlog::error("保存好友信息失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("好友信息保存完成。");
    return true;
}

bool SqliteChatRepository::updateConversation(
    const QString &peerId, const QString &lastMessage, const QDateTime &timestamp, bool incrementUnread, QString *errorMessage)
{
    if (!m_initialized || peerId.isEmpty())
    {
        spdlog::error("会话更新参数无效。");
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE peers SET last_message = :last_message, "
                                 "last_activity_utc = :last_activity, "
                                 "unread_count = MAX(0, unread_count + :unread_delta), "
                                 "updated_at_utc = :updated_at WHERE peer_id = :peer_id"));
    query.bindValue(QStringLiteral(":last_message"), lastMessage);
    query.bindValue(QStringLiteral(":last_activity"), timestampText(timestamp));
    query.bindValue(QStringLiteral(":updated_at"), timestampText(timestamp));
    query.bindValue(QStringLiteral(":unread_delta"), incrementUnread ? 1 : 0);
    query.bindValue(QStringLiteral(":peer_id"), peerId);
    if (!query.exec())
    {
        spdlog::error("更新会话信息失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("会话信息更新完成。");
    return true;
}

bool SqliteChatRepository::clearUnread(const QString &peerId, QString *errorMessage)
{
    if (!m_initialized || peerId.isEmpty())
    {
        spdlog::error("好友标识不能为空。");
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE peers SET unread_count = 0 WHERE peer_id = :peer_id"));
    query.bindValue(QStringLiteral(":peer_id"), peerId);
    if (!query.exec())
    {
        spdlog::error("清除未读消息失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("未读消息清除完成。");
    return true;
}

bool SqliteChatRepository::saveMessage(const Storage::MessageRecord &message, QString *errorMessage)
{
    if (!m_initialized || message.messageId.isEmpty() || message.peerId.isEmpty())
    {
        spdlog::error("消息持久化参数无效。");
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("INSERT INTO messages (message_id, peer_id, from_me, sender_initial, "
                                 "sender_color, message_text, timestamp_utc, delivery_status, message_kind, "
                                 "file_name, file_size_text, file_size_bytes, file_progress, file_path) VALUES "
                                 "(:message_id, :peer_id, :from_me, :sender_initial, :sender_color, "
                                 ":message_text, :timestamp_utc, :delivery_status, :message_kind, "
                                 ":file_name, :file_size_text, :file_size_bytes, :file_progress, :file_path) "
                                 "ON CONFLICT(message_id) DO UPDATE SET peer_id = excluded.peer_id, "
                                 "from_me = excluded.from_me, sender_initial = excluded.sender_initial, "
                                 "sender_color = excluded.sender_color, message_text = excluded.message_text, "
                                 "timestamp_utc = excluded.timestamp_utc, "
                                 "delivery_status = excluded.delivery_status, message_kind = excluded.message_kind, "
                                 "file_name = excluded.file_name, file_size_text = excluded.file_size_text, "
                                 "file_size_bytes = excluded.file_size_bytes, "
                                 "file_progress = excluded.file_progress, file_path = excluded.file_path"));
    query.bindValue(QStringLiteral(":message_id"), message.messageId);
    query.bindValue(QStringLiteral(":peer_id"), message.peerId);
    query.bindValue(QStringLiteral(":from_me"), message.fromMe ? 1 : 0);
    query.bindValue(QStringLiteral(":sender_initial"), message.senderInitial);
    query.bindValue(QStringLiteral(":sender_color"), message.senderColor);
    query.bindValue(QStringLiteral(":message_text"), message.text);
    query.bindValue(QStringLiteral(":timestamp_utc"), timestampText(message.timestamp));
    query.bindValue(QStringLiteral(":delivery_status"), message.deliveryStatus);
    query.bindValue(QStringLiteral(":message_kind"), message.messageKind);
    query.bindValue(QStringLiteral(":file_name"), message.fileName);
    query.bindValue(QStringLiteral(":file_size_text"), message.fileSizeText);
    query.bindValue(QStringLiteral(":file_size_bytes"), qMax<qint64>(0, message.fileSize));
    query.bindValue(QStringLiteral(":file_progress"), qBound(0.0, message.fileProgress, 1.0));
    query.bindValue(QStringLiteral(":file_path"), message.filePath);
    if (!query.exec())
    {
        spdlog::error("保存消息失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("消息保存完成。");
    return true;
}

bool SqliteChatRepository::updateDeliveryStatus(const QString &peerId, const QString &messageId, const QString &status, QString *errorMessage)
{
    if (!m_initialized || peerId.isEmpty() || messageId.isEmpty())
    {
        spdlog::error("消息状态更新参数无效。");
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE messages SET delivery_status = :status "
                                 "WHERE peer_id = :peer_id AND message_id = :message_id"));
    query.bindValue(QStringLiteral(":status"), status);
    query.bindValue(QStringLiteral(":peer_id"), peerId);
    query.bindValue(QStringLiteral(":message_id"), messageId);
    if (!query.exec())
    {
        spdlog::error("更新消息状态失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("消息状态更新完成。");
    return true;
}

bool SqliteChatRepository::updateFileTransfer(
    const QString &peerId, const QString &messageId, qreal progress, const QString &status, const QString &filePath, QString *errorMessage)
{
    if (!m_initialized || peerId.isEmpty() || messageId.isEmpty())
    {
        spdlog::error("文件传输状态更新参数无效。");
        return false;
    }

    QSqlQuery query(database());
    query.prepare(QStringLiteral("UPDATE messages SET file_progress = :progress, delivery_status = :status, "
                                 "file_path = COALESCE(NULLIF(:file_path, ''), file_path) "
                                 "WHERE peer_id = :peer_id AND message_id = :message_id"));
    query.bindValue(QStringLiteral(":progress"), qBound(0.0, progress, 1.0));
    query.bindValue(QStringLiteral(":status"), status);
    query.bindValue(QStringLiteral(":file_path"), filePath);
    query.bindValue(QStringLiteral(":peer_id"), peerId);
    query.bindValue(QStringLiteral(":message_id"), messageId);
    if (!query.exec())
    {
        spdlog::error("更新文件传输状态失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("文件传输状态更新完成。");
    return true;
}

QSqlDatabase SqliteChatRepository::database() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

bool SqliteChatRepository::configureDatabase(QString *errorMessage)
{
    const Config::SqliteConfig config = Config::database.get().sqlite;
    if (!executeStatement(QStringLiteral("PRAGMA busy_timeout = %1").arg(static_cast<qulonglong>(qBound<std::size_t>(1, config.busy_timeout_ms, 600000))),
                          errorMessage))
    {
        return false;
    }
    if (!executeStatement(QStringLiteral("PRAGMA foreign_keys = %1").arg(config.foreign_keys_enabled ? QStringLiteral("ON") : QStringLiteral("OFF")),
                          errorMessage))
    {
        return false;
    }
    if (config.wal_enabled && !executeStatement(QStringLiteral("PRAGMA journal_mode = WAL"), errorMessage))
    {
        return false;
    }
    return executeStatement(QStringLiteral("PRAGMA synchronous = %1").arg(normalizedSynchronousMode(QString::fromStdString(config.synchronous))), errorMessage);
}

bool SqliteChatRepository::migrateSchema(QString *errorMessage)
{
    QSqlDatabase connection = database();
    QSqlQuery versionQuery(connection);
    if (!versionQuery.exec(QStringLiteral("PRAGMA user_version")) || !versionQuery.next())
    {
        spdlog::error("查询数据库版本失败：{}", versionQuery.lastError().text().toUtf8().toStdString());
        return false;
    }
    const int version = versionQuery.value(0).toInt();
    if (version > CurrentSchemaVersion)
    {
        spdlog::error("数据库版本 {} 高于当前支持版本 {}。", version, CurrentSchemaVersion);
        return false;
    }

    if (!connection.transaction())
    {
        spdlog::error("开始事务失败：{}", connection.lastError().text().toUtf8().toStdString());
        return false;
    }

    const QStringList statements{QStringLiteral("CREATE TABLE IF NOT EXISTS peers ("
                                                "peer_id TEXT PRIMARY KEY NOT NULL, "
                                                "display_name TEXT NOT NULL, "
                                                "address TEXT NOT NULL DEFAULT '', "
                                                "tcp_port INTEGER NOT NULL DEFAULT 0, "
                                                "last_message TEXT NOT NULL DEFAULT '', "
                                                "last_activity_utc TEXT NOT NULL DEFAULT '', "
                                                "unread_count INTEGER NOT NULL DEFAULT 0, "
                                                "updated_at_utc TEXT NOT NULL)"),
                                 QStringLiteral("CREATE TABLE IF NOT EXISTS messages ("
                                                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                "message_id TEXT NOT NULL UNIQUE, "
                                                "peer_id TEXT NOT NULL, "
                                                "from_me INTEGER NOT NULL, "
                                                "sender_initial TEXT NOT NULL DEFAULT '', "
                                                "sender_color TEXT NOT NULL DEFAULT '', "
                                                "message_text TEXT NOT NULL DEFAULT '', "
                                                "timestamp_utc TEXT NOT NULL, "
                                                "delivery_status TEXT NOT NULL, "
                                                "message_kind TEXT NOT NULL DEFAULT 'text', "
                                                "file_name TEXT NOT NULL DEFAULT '', "
                                                "file_size_text TEXT NOT NULL DEFAULT '', "
                                                "file_size_bytes INTEGER NOT NULL DEFAULT 0, "
                                                "file_progress REAL NOT NULL DEFAULT 0, "
                                                "file_path TEXT NOT NULL DEFAULT '', "
                                                "FOREIGN KEY(peer_id) REFERENCES peers(peer_id) ON DELETE CASCADE)"),
                                 QStringLiteral("CREATE INDEX IF NOT EXISTS idx_messages_peer_id "
                                                "ON messages(peer_id, id)")};
    for (const QString &statement : statements)
    {
        QSqlQuery query(connection);
        if (!query.exec(statement))
        {
            connection.rollback();
            spdlog::error("创建表失败：{}", query.lastError().text().toUtf8().toStdString());
            return false;
        }
    }

    if (version == 1)
    {
        QSqlQuery migration(connection);
        if (!migration.exec(QStringLiteral("ALTER TABLE messages ADD COLUMN file_size_bytes INTEGER NOT NULL DEFAULT 0")))
        {
            connection.rollback();
            spdlog::error("迁移表失败：{}", migration.lastError().text().toUtf8().toStdString());
            return false;
        }
    }

    QSqlQuery versionUpdate(connection);
    if (!versionUpdate.exec(QStringLiteral("PRAGMA user_version = %1").arg(CurrentSchemaVersion)))
    {
        connection.rollback();
        spdlog::error("更新数据库版本失败：{}", versionUpdate.lastError().text().toUtf8().toStdString());
        return false;
    }
    if (!connection.commit())
    {
        spdlog::error("提交事务失败：{}", connection.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("数据库模式迁移完成。");
    return true;
}

bool SqliteChatRepository::executeStatement(const QString &statement, QString *errorMessage) const
{
    QSqlQuery query(database());
    if (!query.exec(statement))
    {
        spdlog::error("执行 SQL 语句失败：{}", query.lastError().text().toUtf8().toStdString());
        return false;
    }
    spdlog::info("SQL 语句执行完成。");
    return true;
}
