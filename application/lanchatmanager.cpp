#include "lanchatmanager.h"

#include "discovery/ipeerdiscovery.h"
#include "discovery/udppeerdiscovery.h"
#include "platform/desktopfilelauncher.h"
#include "platform/desktopnotificationservice.h"
#include "platform/ifilelauncher.h"
#include "platform/inotificationservice.h"
#include "storage/ichatrepository.h"
#include "storage/sqlitechatrepository.h"
#include "transport/ichattransport.h"
#include "transport/tcpchattransport.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QSettings>
#include <QSysInfo>
#include <QUuid>

#include <spdlog/spdlog.h>

#include <utility>

LanChatManager::LanChatManager(QObject *parent)
: LanChatManager(std::make_unique<UdpPeerDiscovery>(),
                 std::make_unique<TcpChatTransport>(),
                 std::make_unique<SqliteChatRepository>(),
                 std::make_unique<DesktopFileLauncher>(),
                 parent)
{
}

LanChatManager::LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                               std::unique_ptr<IChatTransport> transport,
                               QObject *parent)
: LanChatManager(std::move(discovery),
                 std::move(transport),
                 std::make_unique<SqliteChatRepository>(),
                 std::make_unique<DesktopFileLauncher>(),
                 parent)
{
}

LanChatManager::LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                               std::unique_ptr<IChatTransport> transport,
                               std::unique_ptr<IChatRepository> repository,
                               QObject *parent)
: LanChatManager(std::move(discovery),
                 std::move(transport),
                 std::move(repository),
                 std::make_unique<DesktopFileLauncher>(),
                 parent)
{
}

LanChatManager::LanChatManager(std::unique_ptr<IPeerDiscovery> discovery,
                               std::unique_ptr<IChatTransport> transport,
                               std::unique_ptr<IChatRepository> repository,
                               std::unique_ptr<IFileLauncher> fileLauncher,
                               QObject *parent)
: LanChatManager(std::move(discovery),
                 std::move(transport),
                 std::move(repository),
                 std::move(fileLauncher),
                 std::make_unique<DesktopNotificationService>(),
                 parent)
{
}

LanChatManager::LanChatManager(
    std::unique_ptr<IPeerDiscovery> discovery,
    std::unique_ptr<IChatTransport> transport,
    std::unique_ptr<IChatRepository> repository,
    std::unique_ptr<IFileLauncher> fileLauncher,
    std::unique_ptr<INotificationService> notificationService,
    QObject *parent)
: QObject(parent)
, m_discovery(std::move(discovery))
, m_transport(std::move(transport))
, m_repository(std::move(repository))
, m_fileLauncher(std::move(fileLauncher))
, m_notificationService(std::move(notificationService))
{
    Q_ASSERT(m_discovery);
    Q_ASSERT(m_transport);
    Q_ASSERT(m_repository);
    Q_ASSERT(m_fileLauncher);
    Q_ASSERT(m_notificationService);
    loadIdentity();
    connectServices();
    initializeRepository();
}

LanChatManager::~LanChatManager()
{
    stop();
}

QAbstractItemModel *LanChatManager::peers()
{
    return &m_peerModel;
}

QAbstractItemModel *LanChatManager::messages()
{
    return &m_messageModel;
}

QString LanChatManager::localName() const
{
    return m_identity.displayName;
}

QString LanChatManager::localInitial() const
{
    return m_localInitial;
}

QString LanChatManager::currentPeerId() const
{
    return m_currentPeerId;
}

int LanChatManager::onlineCount() const
{
    return m_peerModel.onlineCount();
}

int LanChatManager::totalUnreadCount() const
{
    return m_peerModel.totalUnreadCount();
}

bool LanChatManager::running() const
{
    return m_running;
}

QString LanChatManager::lastError() const
{
    return m_lastError;
}

bool LanChatManager::start()
{
    if (m_running)
    {
        spdlog::debug("[network.application] start ignored because service is already running");
        return true;
    }
    spdlog::info("[network.application] starting LAN chat services");
    if (!m_transport->start(m_identity))
    {
        setLastError(m_transport->lastError());
        spdlog::error("[network.application] transport startup failed: {}",
                      m_transport->lastError().toUtf8().toStdString());
        return false;
    }
    if (!m_discovery->start(m_identity, m_transport->listeningPort()))
    {
        setLastError(m_discovery->lastError());
        spdlog::error("[network.application] peer discovery startup failed: {}",
                      m_discovery->lastError().toUtf8().toStdString());
        m_transport->stop();
        return false;
    }

    m_running = true;
    setLastError({});
    spdlog::info("[network.application] LAN chat services started tcp_port={}",
                 m_transport->listeningPort());
    emit runningChanged();
    return true;
}

void LanChatManager::stop()
{
    if (!m_running)
    {
        return;
    }
    spdlog::info("[network.application] stopping LAN chat services");
    m_discovery->stop();
    m_transport->stop();
    m_running = false;
    spdlog::info("[network.application] LAN chat services stopped");
    emit runningChanged();
}

bool LanChatManager::selectPeer(const QString &peerId)
{
    if (m_peerModel.indexOf(peerId) < 0)
    {
        spdlog::debug("[network.application] cannot select unknown peer peer_id={}",
                      peerId.toUtf8().toStdString());
        return false;
    }
    const bool changed = m_currentPeerId != peerId;
    loadConversation(peerId);
    m_currentPeerId = peerId;
    m_messageModel.selectPeer(peerId);
    markConversationRead(peerId);
    if (changed)
    {
        spdlog::debug("[network.application] selected peer peer_id={}",
                      peerId.toUtf8().toStdString());
        emit currentPeerIdChanged();
    }
    return true;
}

bool LanChatManager::markConversationRead(const QString &peerId)
{
    if (m_peerModel.indexOf(peerId) < 0)
    {
        return false;
    }

    if (m_repositoryReady)
    {
        QString error;
        if (!m_repository->clearUnread(peerId, &error))
        {
            logRepositoryError("clear unread", error);
            return false;
        }
    }
    m_peerModel.clearUnread(peerId);
    return true;
}

QVariantMap LanChatManager::peerInfo(const QString &peerId) const
{
    return m_peerModel.peerInfo(peerId);
}

bool LanChatManager::updateLocalProfile(const QString &displayName)
{
    const QString normalizedName = displayName.trimmed();
    if (normalizedName.isEmpty() || normalizedName.size() > 64)
    {
        setLastError(tr("昵称需要包含 1 到 64 个字符。"));
        return false;
    }
    if (normalizedName == m_identity.displayName)
    {
        setLastError({});
        return true;
    }

    QSettings settings;
    settings.setValue(QStringLiteral("profile/displayName"), normalizedName);
    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        setLastError(tr("无法保存个人信息。"));
        spdlog::error("[network.application] failed to persist local profile");
        return false;
    }

    m_identity.displayName = normalizedName;
    m_localInitial = normalizedName.left(1).toUpper();
    m_discovery->updateIdentity(m_identity);
    m_transport->updateIdentity(m_identity);
    setLastError({});
    emit localProfileChanged();
    spdlog::info("[network.application] local profile updated");
    return true;
}

bool LanChatManager::sendMessage(const QString &peerId, const QString &text)
{
    const QString content = text.trimmed();
    if (content.isEmpty())
    {
        spdlog::debug("[network.application] ignored empty outgoing message");
        return false;
    }
    if (content.size() > 2000)
    {
        spdlog::warn("[network.application] rejected oversized message peer_id={} length={}",
                     peerId.toUtf8().toStdString(),
                     content.size());
        emit sendFailed(peerId, tr("消息不能超过 2000 个字符。"));
        return false;
    }
    if (!m_running || !m_peerModel.isOnline(peerId))
    {
        spdlog::warn("[network.application] cannot send message to offline peer peer_id={}",
                     peerId.toUtf8().toStdString());
        emit sendFailed(peerId, tr("好友当前不在线。"));
        return false;
    }

    const Network::PeerEndpoint peer = m_peerModel.endpoint(peerId);
    const QString messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QDateTime timestamp = QDateTime::currentDateTimeUtc();
    const QString time = displayTime(timestamp);

    ChatMessageModel::Message message;
    message.messageId = messageId;
    message.senderInitial = m_localInitial;
    message.senderColor = QStringLiteral("#4F7CFF");
    message.messageText = content;
    message.messageTime = time;
    message.deliveryStatus = QStringLiteral("sending");
    message.fromMe = true;
    persistMessage(peerId, message, timestamp);
    m_messageModel.append(peerId, std::move(message));
    m_peerModel.updateConversation(peerId, content, time, false);
    persistConversation(peerId, content, timestamp, false);
    spdlog::debug(
        "[network.application] queued outgoing message peer_id={} message_id={} length={}",
        peerId.toUtf8().toStdString(),
        messageId.toUtf8().toStdString(),
        content.size());
    m_transport->sendText(peer, messageId, content, timestamp);
    return true;
}

bool LanChatManager::sendFile(const QString &peerId, const QUrl &fileUrl)
{
    if (!m_running || !m_peerModel.isOnline(peerId))
    {
        spdlog::warn("[network.application] cannot send file to offline peer peer_id={}",
                     peerId.toUtf8().toStdString());
        emit fileTransferFailed(peerId, tr("好友当前不在线。"));
        return false;
    }

    QString error;
    if (!m_transport->sendFile(m_peerModel.endpoint(peerId), fileUrl, &error))
    {
        spdlog::warn("[network.application] file send request rejected peer_id={} reason={}",
                     peerId.toUtf8().toStdString(),
                     error.toUtf8().toStdString());
        emit fileTransferFailed(peerId, error);
        return false;
    }
    return true;
}

int LanChatManager::sendFiles(const QString &peerId,
                              const QList<QUrl> &fileUrls)
{
    constexpr qsizetype MaximumBatchSize = 100;
    if (fileUrls.isEmpty())
    {
        return 0;
    }
    if (fileUrls.size() > MaximumBatchSize)
    {
        emit operationFailed(tr("单次最多发送 %1 个文件。").arg(MaximumBatchSize));
    }

    int startedCount = 0;
    const qsizetype count = qMin(fileUrls.size(), MaximumBatchSize);
    for (qsizetype index = 0; index < count; ++index)
    {
        startedCount += sendFile(peerId, fileUrls.at(index)) ? 1 : 0;
    }
    spdlog::info("[network.application] file batch queued peer_id={} requested={} started={}",
                 peerId.toUtf8().toStdString(),
                 fileUrls.size(),
                 startedCount);
    return startedCount;
}

bool LanChatManager::cancelFileTransfer(const QString &peerId,
                                        const QString &transferId)
{
    if (!m_transport->cancelFileTransfer(peerId, transferId))
    {
        emit fileTransferFailed(peerId, tr("该文件传输已经结束或不存在。"));
        return false;
    }
    spdlog::info("[network.application] file transfer cancellation requested peer_id={} transfer_id={}",
                 peerId.toUtf8().toStdString(),
                 transferId.toUtf8().toStdString());
    return true;
}

bool LanChatManager::openFile(const QString &filePath)
{
    QString error;
    if (!m_fileLauncher->openFile(filePath, &error))
    {
        emit operationFailed(error);
        return false;
    }
    return true;
}

bool LanChatManager::revealFile(const QString &filePath)
{
    QString error;
    if (!m_fileLauncher->revealInFolder(filePath, &error))
    {
        emit operationFailed(error);
        return false;
    }
    return true;
}

void LanChatManager::setNotificationsEnabled(bool enabled)
{
    m_notificationService->setEnabled(enabled);
}

void LanChatManager::connectServices()
{
    connect(&m_peerModel,
            &PeerListModel::unreadCountChanged,
            this,
            &LanChatManager::totalUnreadCountChanged);
    connect(m_notificationService.get(),
            &INotificationService::notificationActivated,
            this,
            &LanChatManager::notificationActivated);
    connect(m_discovery.get(),
            &IPeerDiscovery::peerFound,
            this,
            &LanChatManager::observePeer);
    connect(m_discovery.get(),
            &IPeerDiscovery::peerLost,
            this,
            &LanChatManager::markPeerOffline);
    connect(m_discovery.get(),
            &IPeerDiscovery::errorOccurred,
            this,
            &LanChatManager::setLastError);
    connect(m_transport.get(),
            &IChatTransport::peerObserved,
            this,
            [this](const Network::PeerEndpoint &peer) {
                m_discovery->recordPeerActivity(peer.peerId);
                observePeer(peer);
            });
    connect(m_transport.get(),
            &IChatTransport::textReceived,
            this,
            &LanChatManager::handleTextMessage);
    connect(m_transport.get(),
            &IChatTransport::textSent,
            this,
            [this](const QString &peerId, const QString &messageId) {
                spdlog::debug(
                    "[network.application] outgoing message delivered peer_id={} message_id={}",
                    peerId.toUtf8().toStdString(),
                    messageId.toUtf8().toStdString());
                m_messageModel.setDeliveryStatus(peerId,
                                                 messageId,
                                                 QStringLiteral("sent"));
                persistDeliveryStatus(peerId,
                                      messageId,
                                      QStringLiteral("sent"));
            });
    connect(m_transport.get(),
            &IChatTransport::textSendFailed,
            this,
            [this](const QString &peerId,
                   const QString &messageId,
                   const QString &reason) {
                spdlog::warn(
                    "[network.application] outgoing message failed peer_id={} message_id={} reason={}",
                    peerId.toUtf8().toStdString(),
                    messageId.toUtf8().toStdString(),
                    reason.toUtf8().toStdString());
                m_messageModel.setDeliveryStatus(peerId,
                                                 messageId,
                                                 QStringLiteral("failed"));
                persistDeliveryStatus(peerId,
                                      messageId,
                                      QStringLiteral("failed"));
                emit sendFailed(peerId, tr("消息发送失败：%1").arg(reason));
            });
    connect(m_transport.get(),
            &IChatTransport::fileTransferStarted,
            this,
            &LanChatManager::handleFileTransferStarted);
    connect(m_transport.get(),
            &IChatTransport::fileTransferProgressed,
            this,
            &LanChatManager::handleFileTransferProgress);
    connect(m_transport.get(),
            &IChatTransport::fileTransferFinished,
            this,
            &LanChatManager::handleFileTransferResult);
    connect(m_transport.get(),
            &IChatTransport::errorOccurred,
            this,
            &LanChatManager::setLastError);
}

void LanChatManager::loadIdentity()
{
    QSettings settings;
    m_identity.deviceId = settings.value(QStringLiteral("network/deviceId")).toString();
    if (m_identity.deviceId.isEmpty())
    {
        m_identity.deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue(QStringLiteral("network/deviceId"), m_identity.deviceId);
    }

    m_identity.displayName = settings.value(QStringLiteral("profile/displayName"))
                                 .toString()
                                 .trimmed();
    if (m_identity.displayName.isEmpty())
    {
        m_identity.displayName = QSysInfo::machineHostName().trimmed();
    }
    if (m_identity.displayName.isEmpty())
    {
        m_identity.displayName = tr("YueLink 用户");
    }
    m_identity.displayName = m_identity.displayName.left(64);
    m_localInitial = m_identity.displayName.left(1).toUpper();
}

void LanChatManager::initializeRepository()
{
    QString error;
    if (!m_repository->initialize(&error))
    {
        logRepositoryError("initialize", error);
        return;
    }
    m_repositoryReady = true;

    QList<Storage::PeerRecord> peers;
    if (!m_repository->loadPeers(&peers, &error))
    {
        logRepositoryError("load peers", error);
        return;
    }
    for (const Storage::PeerRecord &record : peers)
    {
        m_peerModel.restore(record.endpoint,
                            record.lastMessage,
                            displayTime(record.lastActivity),
                            record.unreadCount);
    }
    spdlog::info("[network.application] restored persisted peers count={}",
                 peers.size());
}

void LanChatManager::loadConversation(const QString &peerId)
{
    if (!m_repositoryReady || m_loadedConversations.contains(peerId))
    {
        return;
    }

    constexpr int RecentMessageLimit = 500;
    QString error;
    QList<Storage::MessageRecord> records;
    if (!m_repository->loadMessages(peerId,
                                    RecentMessageLimit,
                                    &records,
                                    &error))
    {
        logRepositoryError("load messages", error);
        return;
    }

    QList<ChatMessageModel::Message> messages;
    messages.reserve(records.size());
    for (Storage::MessageRecord &record : records)
    {
        if (record.deliveryStatus == QLatin1String("sending")
            || record.deliveryStatus == QLatin1String("transferring")
            || record.deliveryStatus == QLatin1String("receiving"))
        {
            record.deliveryStatus = QStringLiteral("failed");
            if (!m_repository->updateDeliveryStatus(record.peerId,
                                                    record.messageId,
                                                    record.deliveryStatus,
                                                    &error))
            {
                logRepositoryError("recover interrupted message", error);
            }
        }

        ChatMessageModel::Message message;
        message.messageId = record.messageId;
        message.senderInitial = record.senderInitial;
        message.senderColor = record.senderColor;
        message.messageText = record.text;
        message.messageTime = displayTime(record.timestamp);
        message.deliveryStatus = record.deliveryStatus;
        message.messageKind = record.messageKind.isEmpty()
                                  ? QStringLiteral("text")
                                  : record.messageKind;
        message.fileName = record.fileName;
        message.fileSizeText = record.fileSizeText;
        message.fileProgress = record.fileProgress;
        message.filePath = record.filePath;
        message.fromMe = record.fromMe;
        messages.append(std::move(message));
    }

    m_messageModel.setConversation(peerId, std::move(messages));
    m_loadedConversations.insert(peerId);
    spdlog::debug("[network.application] restored conversation peer_id={} messages={}",
                  peerId.toUtf8().toStdString(),
                  records.size());
}

void LanChatManager::persistPeer(const Network::PeerEndpoint &peer)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->upsertPeer(peer, &error))
    {
        logRepositoryError("upsert peer", error);
    }
}

void LanChatManager::persistConversation(const QString &peerId,
                                         const QString &lastMessage,
                                         const QDateTime &timestamp,
                                         bool incrementUnread)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->updateConversation(peerId,
                                          lastMessage,
                                          timestamp,
                                          incrementUnread,
                                          &error))
    {
        logRepositoryError("update conversation", error);
    }
}

void LanChatManager::persistMessage(const QString &peerId,
                                    const ChatMessageModel::Message &message,
                                    const QDateTime &timestamp)
{
    if (!m_repositoryReady)
    {
        return;
    }

    Storage::MessageRecord record;
    record.messageId = message.messageId;
    record.peerId = peerId;
    record.senderInitial = message.senderInitial;
    record.senderColor = message.senderColor;
    record.text = message.messageText;
    record.timestamp = timestamp;
    record.deliveryStatus = message.deliveryStatus;
    record.messageKind = message.messageKind;
    record.fileName = message.fileName;
    record.fileSizeText = message.fileSizeText;
    record.filePath = message.filePath;
    record.fileProgress = message.fileProgress;
    record.fromMe = message.fromMe;

    QString error;
    if (!m_repository->saveMessage(record, &error))
    {
        logRepositoryError("save message", error);
    }
}

void LanChatManager::persistDeliveryStatus(const QString &peerId,
                                           const QString &messageId,
                                           const QString &status)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->updateDeliveryStatus(peerId,
                                            messageId,
                                            status,
                                            &error))
    {
        logRepositoryError("update delivery status", error);
    }
}

void LanChatManager::persistFileTransfer(const QString &peerId,
                                         const QString &messageId,
                                         qreal progress,
                                         const QString &status,
                                         const QString &filePath)
{
    if (!m_repositoryReady)
    {
        return;
    }
    QString error;
    if (!m_repository->updateFileTransfer(peerId,
                                          messageId,
                                          progress,
                                          status,
                                          filePath,
                                          &error))
    {
        logRepositoryError("update file transfer", error);
    }
}

void LanChatManager::logRepositoryError(const char *operation,
                                        const QString &error) const
{
    spdlog::warn("[storage.repository] operation failed operation={} reason={}",
                 operation,
                 error.toUtf8().toStdString());
}

void LanChatManager::setLastError(const QString &error)
{
    if (m_lastError == error)
    {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

void LanChatManager::observePeer(const Network::PeerEndpoint &peer)
{
    persistPeer(peer);
    const int previousOnlineCount = onlineCount();
    bool inserted = false;
    const bool changed = m_peerModel.upsert(peer, &inserted);
    if (inserted)
    {
        spdlog::info("[network.application] peer discovered peer_id={} address={} port={}",
                     peer.peerId.toUtf8().toStdString(),
                     peer.address.toString().toStdString(),
                     peer.tcpPort);
        emit peerDiscovered(peer.peerId);
    }
    else if (changed)
    {
        spdlog::debug("[network.application] peer endpoint updated peer_id={} address={} port={}",
                      peer.peerId.toUtf8().toStdString(),
                      peer.address.toString().toStdString(),
                      peer.tcpPort);
        emit peerUpdated(peer.peerId);
    }
    if (previousOnlineCount != onlineCount())
    {
        emit onlineCountChanged();
    }
}

void LanChatManager::markPeerOffline(const QString &peerId)
{
    const int previousOnlineCount = onlineCount();
    if (m_peerModel.setOffline(peerId))
    {
        spdlog::info("[network.application] peer offline peer_id={}",
                     peerId.toUtf8().toStdString());
        emit peerUpdated(peerId);
    }
    if (previousOnlineCount != onlineCount())
    {
        emit onlineCountChanged();
    }
}

void LanChatManager::handleTextMessage(const Network::TextMessage &message)
{
    spdlog::debug(
        "[network.application] incoming message peer_id={} message_id={} length={}",
        message.sender.peerId.toUtf8().toStdString(),
        message.messageId.toUtf8().toStdString(),
        message.text.size());
    const QVariantMap peer = m_peerModel.peerInfo(message.sender.peerId);
    const bool incrementUnread = shouldMarkIncomingUnread(message.sender.peerId);
    ChatMessageModel::Message modelMessage;
    modelMessage.messageId = message.messageId;
    modelMessage.senderInitial = peer.value(QStringLiteral("initial")).toString();
    modelMessage.senderColor = peer.value(QStringLiteral("avatarColor")).toString();
    modelMessage.messageText = message.text;
    modelMessage.messageTime = displayTime(message.timestamp);
    modelMessage.deliveryStatus = QStringLiteral("received");
    modelMessage.fromMe = false;
    persistMessage(message.sender.peerId, modelMessage, message.timestamp);
    m_messageModel.append(message.sender.peerId, std::move(modelMessage));
    m_peerModel.updateConversation(message.sender.peerId,
                                   message.text,
                                   displayTime(message.timestamp),
                                   incrementUnread);
    persistConversation(message.sender.peerId,
                        message.text,
                        message.timestamp,
                        incrementUnread);
    showIncomingNotification(message.sender.peerId, message.text);
    emit messageReceived(message.sender.peerId, message.text);
}

void LanChatManager::handleFileTransferStarted(const Network::FileTransferInfo &transfer)
{
    const bool outgoing = transfer.direction == Network::TransferDirection::Outgoing;
    const bool incrementUnread = !outgoing
                                 && shouldMarkIncomingUnread(transfer.peer.peerId);
    spdlog::info(
        "[network.application] file transfer started direction={} peer_id={} transfer_id={} file_name={} size={}",
        outgoing ? "outgoing" : "incoming",
        transfer.peer.peerId.toUtf8().toStdString(),
        transfer.transferId.toUtf8().toStdString(),
        transfer.fileName.toUtf8().toStdString(),
        transfer.fileSize);
    const QVariantMap peer = m_peerModel.peerInfo(transfer.peer.peerId);
    ChatMessageModel::Message message;
    message.messageId = transfer.transferId;
    message.senderInitial = outgoing
                                ? m_localInitial
                                : peer.value(QStringLiteral("initial")).toString();
    message.senderColor = outgoing
                              ? QStringLiteral("#4F7CFF")
                              : peer.value(QStringLiteral("avatarColor")).toString();
    message.messageText = tr("文件");
    message.messageTime = displayTime(transfer.timestamp);
    message.deliveryStatus = outgoing ? QStringLiteral("transferring")
                                      : QStringLiteral("receiving");
    message.messageKind = QStringLiteral("file");
    message.fileName = transfer.fileName;
    message.fileSizeText = displayFileSize(transfer.fileSize);
    message.filePath = transfer.filePath;
    message.fromMe = outgoing;
    persistMessage(transfer.peer.peerId, message, transfer.timestamp);
    m_messageModel.append(transfer.peer.peerId, std::move(message));
    m_peerModel.updateConversation(transfer.peer.peerId,
                                   tr("[文件] %1").arg(transfer.fileName),
                                   displayTime(transfer.timestamp),
                                   incrementUnread);
    persistConversation(transfer.peer.peerId,
                        tr("[文件] %1").arg(transfer.fileName),
                        transfer.timestamp,
                        incrementUnread);
}

void LanChatManager::handleFileTransferProgress(
    const Network::FileTransferProgress &progress)
{
    spdlog::trace(
        "[network.application] file transfer progress direction={} peer_id={} transfer_id={} progress={:.0f}%",
        progress.direction == Network::TransferDirection::Outgoing ? "outgoing"
                                                                   : "incoming",
        progress.peerId.toUtf8().toStdString(),
        progress.transferId.toUtf8().toStdString(),
        progress.progress * 100.0);
    const QString status = progress.direction == Network::TransferDirection::Outgoing
                               ? QStringLiteral("transferring")
                               : QStringLiteral("receiving");
    m_messageModel.updateFileTransfer(progress.peerId,
                                      progress.transferId,
                                      progress.progress,
                                      status);
    persistFileTransfer(progress.peerId,
                        progress.transferId,
                        progress.progress,
                        status);
}

void LanChatManager::handleFileTransferResult(const Network::FileTransferResult &result)
{
    if (result.success)
    {
        const bool outgoing = result.direction == Network::TransferDirection::Outgoing;
        spdlog::info(
            "[network.application] file transfer completed direction={} peer_id={} transfer_id={}",
            outgoing ? "outgoing" : "incoming",
            result.peerId.toUtf8().toStdString(),
            result.transferId.toUtf8().toStdString());
        m_messageModel.updateFileTransfer(result.peerId,
                                          result.transferId,
                                          1.0,
                                          outgoing ? QStringLiteral("sent")
                                                   : QStringLiteral("received"),
                                          result.filePath);
        persistFileTransfer(result.peerId,
                            result.transferId,
                            1.0,
                            outgoing ? QStringLiteral("sent")
                                     : QStringLiteral("received"),
                            result.filePath);
        if (!outgoing)
        {
            showIncomingNotification(
                result.peerId,
                tr("文件已接收：%1").arg(QFileInfo(result.filePath).fileName()));
            emit fileReceived(result.peerId, result.filePath);
        }
        return;
    }

    if (result.cancelled)
    {
        m_messageModel.setDeliveryStatus(result.peerId,
                                         result.transferId,
                                         QStringLiteral("cancelled"));
        persistDeliveryStatus(result.peerId,
                              result.transferId,
                              QStringLiteral("cancelled"));
        spdlog::info(
            "[network.application] file transfer cancelled direction={} peer_id={} transfer_id={}",
            result.direction == Network::TransferDirection::Outgoing ? "outgoing"
                                                                      : "incoming",
            result.peerId.toUtf8().toStdString(),
            result.transferId.toUtf8().toStdString());
        return;
    }

    m_messageModel.setDeliveryStatus(result.peerId,
                                     result.transferId,
                                     QStringLiteral("failed"));
    persistDeliveryStatus(result.peerId,
                          result.transferId,
                          QStringLiteral("failed"));
    spdlog::warn(
        "[network.application] file transfer failed direction={} peer_id={} transfer_id={} reason={}",
        result.direction == Network::TransferDirection::Outgoing ? "outgoing"
                                                                  : "incoming",
        result.peerId.toUtf8().toStdString(),
        result.transferId.toUtf8().toStdString(),
        result.errorMessage.toUtf8().toStdString());
    const QString reason = result.direction == Network::TransferDirection::Outgoing
                               ? tr("文件发送失败：%1").arg(result.errorMessage)
                               : tr("文件接收失败：%1").arg(result.errorMessage);
    if (result.direction == Network::TransferDirection::Incoming)
    {
        showIncomingNotification(result.peerId, reason);
    }
    emit fileTransferFailed(result.peerId, reason);
}

bool LanChatManager::shouldMarkIncomingUnread(const QString &peerId) const
{
    return peerId != m_currentPeerId
           || QGuiApplication::applicationState() != Qt::ApplicationActive;
}

void LanChatManager::showIncomingNotification(const QString &peerId,
                                              const QString &message)
{
    if (QGuiApplication::applicationState() == Qt::ApplicationActive)
    {
        return;
    }

    const QVariantMap peer = m_peerModel.peerInfo(peerId);
    QString title = peer.value(QStringLiteral("friendName")).toString().trimmed();
    if (title.isEmpty())
    {
        title = tr("YueLink 新消息");
    }

    QString preview = message.simplified();
    constexpr qsizetype MaximumPreviewLength = 160;
    if (preview.size() > MaximumPreviewLength)
    {
        preview = tr("%1…").arg(preview.left(MaximumPreviewLength - 1));
    }
    m_notificationService->showNotification(title, preview, peerId);
}

QString LanChatManager::displayFileSize(qint64 bytes)
{
    constexpr qreal Kilobyte = 1024.0;
    constexpr qreal Megabyte = Kilobyte * 1024.0;
    constexpr qreal Gigabyte = Megabyte * 1024.0;
    if (bytes < 1024)
    {
        return tr("%1 B").arg(bytes);
    }
    if (bytes < static_cast<qint64>(Megabyte))
    {
        return tr("%1 KB").arg(QString::number(bytes / Kilobyte, 'f', 1));
    }
    if (bytes < static_cast<qint64>(Gigabyte))
    {
        return tr("%1 MB").arg(QString::number(bytes / Megabyte, 'f', 1));
    }
    return tr("%1 GB").arg(QString::number(bytes / Gigabyte, 'f', 1));
}

QString LanChatManager::displayTime(const QDateTime &timestamp)
{
    const QDateTime localTimestamp = timestamp.isValid()
                                         ? timestamp.toLocalTime()
                                         : QDateTime::currentDateTime();
    const QDate currentDate = QDate::currentDate();
    if (localTimestamp.date() == currentDate)
    {
        return localTimestamp.toString(QStringLiteral("HH:mm"));
    }
    if (localTimestamp.date() == currentDate.addDays(-1))
    {
        return tr("昨天 %1").arg(localTimestamp.toString(QStringLiteral("HH:mm")));
    }
    if (localTimestamp.date().year() == currentDate.year())
    {
        return localTimestamp.toString(QStringLiteral("MM-dd HH:mm"));
    }
    return localTimestamp.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
}
