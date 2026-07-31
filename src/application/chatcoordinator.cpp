#include "chatcoordinator.h"

#include "conversationstore.h"
#include "transfercoordinator.h"
#include "domain/ichatrepository.h"
#include "domain/ichattransport.h"
#include "domain/ipeerdiscovery.h"
#include "infrastructure/config/configstore.h"

#include <QColor>
#include <QDateTime>
#include <QFileInfo>
#include <QImageReader>
#include <QSysInfo>
#include <QUuid>

#include <spdlog/spdlog.h>

#include <utility>

ChatCoordinator::ChatCoordinator(std::unique_ptr<IPeerDiscovery> discovery,
                                 std::unique_ptr<IChatTransport> transport,
                                 std::unique_ptr<IChatRepository> repository,
                                 QObject *parent)
: QObject(parent)
, m_discovery(std::move(discovery))
, m_transport(std::move(transport))
, m_conversations(std::make_unique<ConversationStore>(std::move(repository)))
, m_transfers(std::make_unique<TransferCoordinator>(m_transport.get(),
                                                    m_conversations.get()))
{
    Q_ASSERT(m_discovery);
    Q_ASSERT(m_transport);
    Q_ASSERT(m_conversations);
    Q_ASSERT(m_transfers);
    connectServices();
    m_identityReady = initializeIdentity();
    static_cast<void>(m_conversations->initialize());
}

ChatCoordinator::~ChatCoordinator()
{
    stop();
}

Network::LocalIdentity ChatCoordinator::localIdentity() const
{
    return m_identity;
}

QString ChatCoordinator::localAvatarPath() const
{
    return m_localAvatarPath;
}

QString ChatCoordinator::localAvatarColor() const
{
    return m_localAvatarColor;
}

QList<Domain::Peer> ChatCoordinator::peers() const
{
    return m_conversations->peers();
}

bool ChatCoordinator::peer(const QString &peerId, Domain::Peer *result) const
{
    return m_conversations->peer(peerId, result);
}

QList<Domain::Message> ChatCoordinator::messages(const QString &peerId, int limit)
{
    return m_conversations->messages(peerId, limit);
}

int ChatCoordinator::onlineCount() const
{
    return m_conversations->onlineCount();
}

int ChatCoordinator::totalUnreadCount() const
{
    return m_conversations->totalUnreadCount();
}

bool ChatCoordinator::running() const
{
    return m_running;
}

QString ChatCoordinator::lastError() const
{
    return m_lastError;
}

Domain::OperationResult ChatCoordinator::start()
{
    if (m_running)
    {
        return Domain::OperationResult::success();
    }
    if (!m_identityReady || m_identity.deviceId.isEmpty()
        || m_identity.displayName.isEmpty())
    {
        const QString error = m_lastError.isEmpty() ? tr("本机身份信息无效。")
                                                    : m_lastError;
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("identity.invalid"), error);
    }

    spdlog::info("[网络.应用] 正在启动局域网聊天服务");
    if (!m_transport->start(m_identity))
    {
        const QString error = m_transport->lastError();
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("transport.start"), error);
    }
    if (!m_discovery->start(m_identity, m_transport->listeningPort()))
    {
        const QString error = m_discovery->lastError();
        m_transport->stop();
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("discovery.start"), error);
    }

    m_running = true;
    setLastError({});
    emit runningChanged();
    spdlog::info("[网络.应用] 局域网聊天服务已启动 TCP端口={}",
                 m_transport->listeningPort());
    return Domain::OperationResult::success();
}

void ChatCoordinator::stop()
{
    if (!m_running)
    {
        return;
    }
    m_discovery->stop();
    m_transport->stop();
    m_conversations->markAllPeersOffline();
    m_running = false;
    emit runningChanged();
    spdlog::info("[网络.应用] 局域网聊天服务已停止");
}

Domain::OperationResult ChatCoordinator::refreshPeerDiscovery()
{
    if (!m_running)
    {
        const QString error = tr("局域网服务未启动。");
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("discovery.not_running"), error);
    }

    m_discovery->probe();
    setLastError({});
    return Domain::OperationResult::success();
}

Domain::OperationResult ChatCoordinator::markConversationRead(const QString &peerId)
{
    return m_conversations->markConversationRead(peerId);
}

Domain::OperationResult ChatCoordinator::updateLocalProfile(const QString &displayName,
                                                            const QString &avatarPath,
                                                            const QString &avatarColor)
{
    const QString normalizedName = displayName.trimmed();
    if (normalizedName.isEmpty() || normalizedName.size() > 64)
    {
        const QString error = tr("昵称需要包含 1 到 64 个字符。");
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("profile.invalid_name"), error);
    }
    QString normalizedAvatarPath = avatarPath.trimmed();
    if (!normalizedAvatarPath.isEmpty())
    {
        const QFileInfo avatarFileInfo(normalizedAvatarPath);
        if (!avatarFileInfo.isFile()
            || QImageReader::imageFormat(avatarFileInfo.absoluteFilePath()).isEmpty())
        {
            const QString error = tr("头像图片无效或不可读取。");
            setLastError(error);
            return Domain::OperationResult::failure(QStringLiteral("profile.invalid_avatar"), error);
        }
        normalizedAvatarPath = avatarFileInfo.absoluteFilePath();
    }

    const QColor parsedAvatarColor(avatarColor.trimmed());
    if (!parsedAvatarColor.isValid())
    {
        const QString error = tr("头像颜色无效。");
        setLastError(error);
        return Domain::OperationResult::failure(QStringLiteral("profile.invalid_avatar_color"), error);
    }
    const QString normalizedAvatarColor = parsedAvatarColor.name(QColor::HexRgb);

    if (normalizedName == m_identity.displayName
        && normalizedAvatarPath == m_localAvatarPath
        && normalizedAvatarColor == m_localAvatarColor)
    {
        setLastError({});
        return Domain::OperationResult::success();
    }

    const Config::IdentityConfig previous = Config::identity.get();
    Config::IdentityConfig updated = previous;
    updated.device_id = m_identity.deviceId.toStdString();
    updated.display_name = normalizedName.toStdString();
    updated.avatar_path = normalizedAvatarPath.toStdString();
    updated.avatar_color = normalizedAvatarColor.toStdString();
    Config::identity.set(updated);
    const Config::Result result = Config::identity.save();
    if (!result)
    {
        Config::identity.set(previous);
        setLastError(result.errorMessage);
        return Domain::OperationResult::failure(QStringLiteral("identity.save"),
                                                result.errorMessage);
    }

    m_identity.displayName = normalizedName;
    m_localAvatarPath = normalizedAvatarPath;
    m_localAvatarColor = normalizedAvatarColor;
    m_discovery->updateIdentity(m_identity);
    m_transport->updateIdentity(m_identity);
    setLastError({});
    emit localIdentityChanged();
    spdlog::info("[身份] 本机资料已更新");
    return Domain::OperationResult::success();
}

Domain::OperationResult ChatCoordinator::sendText(const QString &peerId,
                                                  const QString &text)
{
    const QString content = text.trimmed();
    if (content.isEmpty())
    {
        return Domain::OperationResult::failure(QStringLiteral("message.empty"),
                                                tr("消息不能为空。"));
    }
    if (content.size() > 2000)
    {
        const QString error = tr("消息不能超过 2000 个字符。");
        emit sendFailed(peerId, error);
        return Domain::OperationResult::failure(QStringLiteral("message.too_long"), error);
    }

    Domain::Peer peerRecord;
    if (!resolveOnlinePeer(peerId, &peerRecord, false))
    {
        return Domain::OperationResult::failure(QStringLiteral("peer.offline"),
                                                tr("好友当前不在线。"));
    }

    Domain::Message message;
    message.messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    message.peerId = peerId;
    message.text = content;
    message.timestamp = QDateTime::currentDateTimeUtc();
    message.deliveryState = Domain::DeliveryState::Sending;
    message.fromMe = true;
    const QString messageId = message.messageId;
    const QDateTime timestamp = message.timestamp;
    m_conversations->appendMessage(std::move(message), content, false);
    m_transport->sendText(peerRecord.endpoint, messageId, content, timestamp);
    return Domain::OperationResult::success(messageId);
}

Domain::OperationResult ChatCoordinator::sendFile(const QString &peerId,
                                                  const QString &filePath)
{
    Domain::Peer peerRecord;
    if (!resolveOnlinePeer(peerId, &peerRecord, true))
    {
        return Domain::OperationResult::failure(QStringLiteral("peer.offline"),
                                                tr("好友当前不在线。"));
    }
    return m_transfers->sendFile(peerRecord, filePath);
}

int ChatCoordinator::sendFiles(const QString &peerId,
                               const QStringList &filePaths)
{
    Domain::Peer peerRecord;
    if (!resolveOnlinePeer(peerId, &peerRecord, true))
    {
        return 0;
    }
    return m_transfers->sendFiles(peerRecord, filePaths);
}

Domain::OperationResult ChatCoordinator::cancelFileTransfer(const QString &peerId,
                                                            const QString &transferId)
{
    return m_transfers->cancel(peerId, transferId);
}

void ChatCoordinator::connectServices()
{
    connect(m_discovery.get(),
            &IPeerDiscovery::peerFound,
            m_conversations.get(),
            &ConversationStore::observePeer);
    connect(m_discovery.get(),
            &IPeerDiscovery::peerLost,
            m_conversations.get(),
            &ConversationStore::markPeerOffline);
    connect(m_discovery.get(),
            &IPeerDiscovery::errorOccurred,
            this,
            &ChatCoordinator::setLastError);
    connect(m_transport.get(),
            &IChatTransport::peerObserved,
            this,
            [this](const Network::PeerEndpoint &peer) {
                m_discovery->recordPeerActivity(peer.peerId);
                m_conversations->observePeer(peer);
            });
    connect(m_transport.get(),
            &IChatTransport::textReceived,
            this,
            &ChatCoordinator::handleTextMessage);
    connect(m_transport.get(),
            &IChatTransport::textSent,
            this,
            [this](const QString &peerId, const QString &messageId) {
                m_conversations->updateMessageState(peerId,
                                                    messageId,
                                                    Domain::DeliveryState::Sent);
            });
    connect(m_transport.get(),
            &IChatTransport::textSendFailed,
            this,
            [this](const QString &peerId,
                   const QString &messageId,
                   const QString &reason) {
                m_conversations->updateMessageState(peerId,
                                                    messageId,
                                                    Domain::DeliveryState::Failed);
                emit sendFailed(peerId, tr("消息发送失败：%1").arg(reason));
            });
    connect(m_transport.get(),
            &IChatTransport::errorOccurred,
            this,
            &ChatCoordinator::setLastError);

    connect(m_conversations.get(),
            &ConversationStore::peersChanged,
            this,
            &ChatCoordinator::peersChanged);
    connect(m_conversations.get(),
            &ConversationStore::peerDiscovered,
            this,
            &ChatCoordinator::peerDiscovered);
    connect(m_conversations.get(),
            &ConversationStore::peerUpdated,
            this,
            &ChatCoordinator::peerUpdated);
    connect(m_conversations.get(),
            &ConversationStore::messageAdded,
            this,
            &ChatCoordinator::messageAdded);
    connect(m_conversations.get(),
            &ConversationStore::messageStateChanged,
            this,
            &ChatCoordinator::messageStateChanged);
    connect(m_conversations.get(),
            &ConversationStore::fileTransferChanged,
            this,
            &ChatCoordinator::fileTransferChanged);
    connect(m_conversations.get(),
            &ConversationStore::operationFailed,
            this,
            &ChatCoordinator::operationFailed);

    connect(m_transfers.get(),
            &TransferCoordinator::fileReceived,
            this,
            &ChatCoordinator::fileReceived);
    connect(m_transfers.get(),
            &TransferCoordinator::fileTransferFailed,
            this,
            &ChatCoordinator::fileTransferFailed);
    connect(m_transfers.get(),
            &TransferCoordinator::operationFailed,
            this,
            &ChatCoordinator::operationFailed);
}

bool ChatCoordinator::initializeIdentity()
{
    const Config::Result loadResult = Config::identity.load();
    if (!loadResult)
    {
        setLastError(loadResult.errorMessage);
        return false;
    }

    Config::IdentityConfig config = Config::identity.get();
    QString deviceId = QString::fromStdString(config.device_id).trimmed();
    QString displayName = QString::fromStdString(config.display_name).trimmed();
    QString avatarPath = QString::fromStdString(config.avatar_path).trimmed();
    QString avatarColor = QString::fromStdString(config.avatar_color).trimmed();
    bool changed = false;
    if (deviceId.isEmpty())
    {
        deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        changed = true;
    }
    if (displayName.isEmpty())
    {
        displayName = QSysInfo::machineHostName().trimmed();
        if (displayName.isEmpty())
        {
            displayName = tr("YueLink 用户");
        }
        changed = true;
    }
    if (displayName.size() > 64)
    {
        displayName = displayName.left(64);
        changed = true;
    }
    if (!avatarPath.isEmpty())
    {
        const QFileInfo avatarFileInfo(avatarPath);
        if (!avatarFileInfo.isFile()
            || QImageReader::imageFormat(avatarFileInfo.absoluteFilePath()).isEmpty())
        {
            avatarPath.clear();
            changed = true;
        }
        else
        {
            avatarPath = avatarFileInfo.absoluteFilePath();
        }
    }
    const QColor parsedAvatarColor(avatarColor);
    if (!parsedAvatarColor.isValid())
    {
        avatarColor = QStringLiteral("#4f7cff");
        changed = true;
    }
    else
    {
        avatarColor = parsedAvatarColor.name(QColor::HexRgb);
    }

    m_identity.deviceId = deviceId;
    m_identity.displayName = displayName;
    m_localAvatarPath = avatarPath;
    m_localAvatarColor = avatarColor;
    if (!changed)
    {
        return true;
    }

    config.device_id = deviceId.toStdString();
    config.display_name = displayName.toStdString();
    config.avatar_path = avatarPath.toStdString();
    config.avatar_color = avatarColor.toStdString();
    Config::identity.set(config);
    const Config::Result result = Config::identity.save();
    if (!result)
    {
        setLastError(result.errorMessage);
        return false;
    }
    return true;
}

void ChatCoordinator::handleTextMessage(const Network::TextMessage &message)
{
    m_conversations->observePeer(message.sender);
    Domain::Message received;
    received.messageId = message.messageId;
    received.peerId = message.sender.peerId;
    received.text = message.text;
    received.timestamp = message.timestamp;
    received.deliveryState = Domain::DeliveryState::Received;
    m_conversations->appendMessage(std::move(received),
                                   message.text,
                                   true);
    emit messageReceived(message.sender.peerId, message.text);
}

void ChatCoordinator::setLastError(const QString &error)
{
    if (m_lastError == error)
    {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

bool ChatCoordinator::resolveOnlinePeer(const QString &peerId,
                                        Domain::Peer *peerRecord,
                                        bool fileOperation)
{
    Domain::Peer peer;
    if (m_running && m_conversations->peer(peerId, &peer) && peer.online)
    {
        if (peerRecord)
        {
            *peerRecord = peer;
        }
        return true;
    }

    const QString error = tr("好友当前不在线。");
    if (fileOperation)
    {
        emit fileTransferFailed(peerId, error, false);
    }
    else
    {
        emit sendFailed(peerId, error);
    }
    return false;
}
