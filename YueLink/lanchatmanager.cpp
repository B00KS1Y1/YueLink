#include "lanchatmanager.h"

#include "application/chatcoordinator.h"
#include "YueLink/conversationlistviewmodel.h"
#include "YueLink/conversationviewmodel.h"
#include "YueLink/desktopfilelauncher.h"
#include "YueLink/desktopintegration.h"
#include "YueLink/desktopnotificationservice.h"
#include "YueLink/peerlistviewmodel.h"

#include <utility>

ChatCoordinator *LanChatManager::s_coordinator = nullptr;

void LanChatManager::setCoordinator(ChatCoordinator *coordinator)
{
    s_coordinator = coordinator;
}

LanChatManager *LanChatManager::create(QQmlEngine *, QJSEngine *)
{
    Q_ASSERT(s_coordinator);
    return new LanChatManager(s_coordinator);
}

LanChatManager::LanChatManager(ChatCoordinator *coordinator, QObject *parent)
: LanChatManager(coordinator, std::make_unique<DesktopFileLauncher>(), std::make_unique<DesktopNotificationService>(), parent)
{
}

LanChatManager::LanChatManager(ChatCoordinator *coordinator,
                               std::unique_ptr<IFileLauncher> fileLauncher,
                               std::unique_ptr<INotificationService> notificationService,
                               QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
, m_conversations(std::make_unique<ConversationListViewModel>(coordinator))
, m_peers(std::make_unique<PeerListViewModel>(coordinator))
, m_conversation(std::make_unique<ConversationViewModel>(coordinator))
, m_desktop(std::make_unique<DesktopIntegration>(coordinator, m_conversation.get(), std::move(fileLauncher), std::move(notificationService)))
{
    Q_ASSERT(m_coordinator);
    Q_ASSERT(m_conversations);
    Q_ASSERT(m_peers);
    Q_ASSERT(m_conversation);
    Q_ASSERT(m_desktop);
    connectComponents();
}

LanChatManager::~LanChatManager() = default;

QAbstractItemModel *LanChatManager::conversations()
{
    return m_conversations->model();
}

QAbstractItemModel *LanChatManager::groups()
{
    return m_conversations->groupsModel();
}

QAbstractItemModel *LanChatManager::peers()
{
    return m_peers->model();
}

QAbstractItemModel *LanChatManager::messages()
{
    return m_conversation->model();
}

QAbstractItemModel *LanChatManager::messageHistory()
{
    return m_conversation->historyModel();
}

QString LanChatManager::conversationSearchText() const
{
    return m_conversations->searchText();
}

void LanChatManager::setConversationSearchText(const QString &text)
{
    m_conversations->setSearchText(text);
}

QString LanChatManager::groupSearchText() const
{
    return m_conversations->groupSearchText();
}

void LanChatManager::setGroupSearchText(const QString &text)
{
    m_conversations->setGroupSearchText(text);
}

QString LanChatManager::peerSearchText() const
{
    return m_peers->searchText();
}

void LanChatManager::setPeerSearchText(const QString &text)
{
    m_peers->setSearchText(text);
}

QString LanChatManager::messageSearchText() const
{
    return m_conversation->searchText();
}

void LanChatManager::setMessageSearchText(const QString &text)
{
    m_conversation->setSearchText(text);
}

QString LanChatManager::messageHistorySearchText() const
{
    return m_conversation->historySearchText();
}

void LanChatManager::setMessageHistorySearchText(const QString &text)
{
    m_conversation->setHistorySearchText(text);
}

QString LanChatManager::messageHistoryCategory() const
{
    return m_conversation->historyCategory();
}

void LanChatManager::setMessageHistoryCategory(const QString &category)
{
    m_conversation->setHistoryCategory(category);
}

QString LanChatManager::localName() const
{
    return m_coordinator->localIdentity().displayName;
}

QString LanChatManager::localInitial() const
{
    return m_conversation->localInitial();
}

QUrl LanChatManager::localAvatarUrl() const
{
    const QString avatarPath = m_coordinator->localAvatarPath();
    return avatarPath.isEmpty() ? QUrl{} : QUrl::fromLocalFile(avatarPath);
}

QString LanChatManager::localAvatarColor() const
{
    return m_coordinator->localAvatarColor();
}

QString LanChatManager::currentConversationId() const
{
    return m_conversation->currentConversationId();
}

int LanChatManager::onlineCount() const
{
    return m_peers->onlineCount();
}

int LanChatManager::totalUnreadCount() const
{
    return m_conversations->totalUnreadCount();
}

bool LanChatManager::running() const
{
    return m_coordinator->running();
}

QString LanChatManager::lastError() const
{
    return m_coordinator->lastError();
}

bool LanChatManager::start()
{
    return static_cast<bool>(m_coordinator->start());
}

void LanChatManager::stop()
{
    m_coordinator->stop();
}

bool LanChatManager::refreshPeers()
{
    return static_cast<bool>(m_coordinator->refreshPeerDiscovery());
}

bool LanChatManager::selectConversation(const QString &conversationId)
{
    return m_conversation->selectConversation(conversationId);
}

bool LanChatManager::markConversationRead(const QString &conversationId)
{
    return static_cast<bool>(m_coordinator->markConversationRead(conversationId));
}

bool LanChatManager::setConversationPinned(const QString &conversationId, bool pinned)
{
    const Domain::OperationResult result = m_coordinator->setConversationPinned(conversationId, pinned);
    if (!result)
    {
        if (!result.code.startsWith(QLatin1String("storage.")))
        {
            emit operationFailed(result.message);
        }
        return false;
    }
    return true;
}

bool LanChatManager::removeConversation(const QString &conversationId)
{
    const Domain::OperationResult result = m_coordinator->removeConversation(conversationId);
    if (!result)
    {
        if (!result.code.startsWith(QLatin1String("storage.")))
        {
            emit operationFailed(result.message);
        }
        return false;
    }
    return true;
}

QVariantMap LanChatManager::conversationInfo(const QString &conversationId) const
{
    return m_conversations->conversationInfo(conversationId);
}

QVariantList LanChatManager::groupMembers(const QString &groupId) const
{
    QVariantList result;
    const QString localId = m_coordinator->localIdentity().deviceId;
    for (const Domain::GroupMember &member : m_coordinator->groupMembers(groupId))
    {
        Domain::Peer peer;
        const bool local = member.peerId == localId;
        const bool online = local ? m_coordinator->running() : m_coordinator->peer(member.peerId, &peer) && peer.online;
        const QString displayName = member.displayName.trimmed();
        const QVariantMap contact = m_peers->peerInfo(member.peerId);
        QString avatarColor = local ? m_coordinator->localAvatarColor() : contact.value(QStringLiteral("avatarColor")).toString();
        if (avatarColor.isEmpty())
        {
            avatarColor = QStringLiteral("#65758B");
        }
        result.append(QVariantMap{{QStringLiteral("peerId"), member.peerId},
                                  {QStringLiteral("displayName"), displayName},
                                  {QStringLiteral("initial"), displayName.isEmpty() ? QStringLiteral("?") : displayName.left(1).toUpper()},
                                  {QStringLiteral("avatarColor"), avatarColor},
                                  {QStringLiteral("role"), Domain::groupRoleName(member.role)},
                                  {QStringLiteral("owner"), member.role == Domain::GroupRole::Owner},
                                  {QStringLiteral("online"), online},
                                  {QStringLiteral("local"), local}});
    }
    return result;
}

QString LanChatManager::createGroup(const QString &name, const QStringList &memberIds)
{
    const Domain::OperationResult result = m_coordinator->createGroup(name, memberIds);
    if (!result)
    {
        emit operationFailed(result.message);
        return {};
    }
    return result.value;
}

bool LanChatManager::updateLocalProfile(const QString &displayName, const QUrl &avatarUrl, const QString &avatarColor)
{
    if (!avatarUrl.isEmpty() && !avatarUrl.isLocalFile())
    {
        emit operationFailed(tr("仅支持使用本地头像图片。"));
        return false;
    }
    return static_cast<bool>(m_coordinator->updateLocalProfile(displayName, avatarUrl.isEmpty() ? QString{} : avatarUrl.toLocalFile(), avatarColor));
}

bool LanChatManager::sendMessage(const QString &conversationId, const QString &text)
{
    return static_cast<bool>(m_coordinator->sendText(conversationId, text));
}

bool LanChatManager::sendWindowShake(const QString &conversationId)
{
    const Domain::OperationResult result = m_coordinator->sendWindowShake(conversationId);
    if (!result)
    {
        emit operationFailed(result.message);
        return false;
    }
    return true;
}

bool LanChatManager::sendFile(const QString &conversationId, const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile())
    {
        emit fileTransferFailed(conversationId, tr("仅支持发送本地文件。"));
        return false;
    }
    return static_cast<bool>(m_coordinator->sendFile(conversationId, fileUrl.toLocalFile()));
}

bool LanChatManager::sendImage(const QString &conversationId, const QUrl &imageUrl)
{
    if (!imageUrl.isLocalFile())
    {
        emit fileTransferFailed(conversationId, tr("仅支持发送本地图片。"));
        return false;
    }
    return static_cast<bool>(m_coordinator->sendImage(conversationId, imageUrl.toLocalFile()));
}

int LanChatManager::sendImages(const QString &conversationId, const QList<QUrl> &imageUrls)
{
    QStringList paths;
    paths.reserve(imageUrls.size());
    for (const QUrl &url : imageUrls)
    {
        if (url.isLocalFile())
        {
            paths.append(url.toLocalFile());
        }
    }
    return m_coordinator->sendImages(conversationId, paths);
}

bool LanChatManager::sendEmoji(const QString &conversationId, const QString &packageId, const QString &emojiId, const QString &fallbackText)
{
    return static_cast<bool>(m_coordinator->sendEmoji(conversationId, packageId, emojiId, fallbackText));
}

int LanChatManager::sendFiles(const QString &conversationId, const QList<QUrl> &fileUrls)
{
    QStringList paths;
    paths.reserve(fileUrls.size());
    for (const QUrl &url : fileUrls)
    {
        if (url.isLocalFile())
        {
            paths.append(url.toLocalFile());
        }
    }
    return m_coordinator->sendFiles(conversationId, paths);
}

QList<QUrl> LanChatManager::clipboardImageUrls()
{
    QString error;
    const QList<QUrl> urls = m_desktop->clipboardImageUrls(&error);
    if (!error.isEmpty())
    {
        emit operationFailed(error);
    }
    return urls;
}

bool LanChatManager::cancelFileTransfer(const QString &conversationId, const QString &transferId)
{
    return static_cast<bool>(m_coordinator->cancelFileTransfer(conversationId, transferId));
}

bool LanChatManager::acceptFileTransfer(const QString &conversationId, const QString &transferId)
{
    return static_cast<bool>(m_coordinator->acceptFileTransfer(conversationId, transferId));
}

bool LanChatManager::openFile(const QString &filePath)
{
    QString error;
    if (!m_desktop->openFile(filePath, &error))
    {
        emit operationFailed(error);
        return false;
    }
    return true;
}

bool LanChatManager::revealFile(const QString &filePath)
{
    QString error;
    if (!m_desktop->revealFile(filePath, &error))
    {
        emit operationFailed(error);
        return false;
    }
    return true;
}

bool LanChatManager::copyLocalDeviceId()
{
    QString error;
    if (!m_desktop->copyText(m_coordinator->localIdentity().deviceId, &error))
    {
        emit operationFailed(error);
        return false;
    }
    return true;
}

void LanChatManager::setNotificationsEnabled(bool enabled)
{
    m_desktop->setNotificationsEnabled(enabled);
}

void LanChatManager::connectComponents()
{
    connect(m_conversations.get(), &ConversationListViewModel::searchTextChanged, this, &LanChatManager::conversationSearchTextChanged);
    connect(m_conversations.get(), &ConversationListViewModel::groupSearchTextChanged, this, &LanChatManager::groupSearchTextChanged);
    connect(m_conversations.get(), &ConversationListViewModel::totalUnreadCountChanged, this, &LanChatManager::totalUnreadCountChanged);
    connect(m_peers.get(), &PeerListViewModel::searchTextChanged, this, &LanChatManager::peerSearchTextChanged);
    connect(m_peers.get(), &PeerListViewModel::onlineCountChanged, this, &LanChatManager::onlineCountChanged);
    connect(m_peers.get(), &PeerListViewModel::peerDiscovered, this, &LanChatManager::peerDiscovered);
    connect(m_peers.get(), &PeerListViewModel::peerUpdated, this, &LanChatManager::peerUpdated);
    connect(m_conversation.get(), &ConversationViewModel::searchTextChanged, this, &LanChatManager::messageSearchTextChanged);
    connect(m_conversation.get(),
            &ConversationViewModel::historySearchTextChanged,
            this,
            &LanChatManager::messageHistorySearchTextChanged);
    connect(m_conversation.get(),
            &ConversationViewModel::historyCategoryChanged,
            this,
            &LanChatManager::messageHistoryCategoryChanged);
    connect(m_conversation.get(), &ConversationViewModel::currentConversationIdChanged, this, &LanChatManager::currentConversationIdChanged);
    connect(m_desktop.get(), &DesktopIntegration::notificationActivated, this, &LanChatManager::notificationActivated);

    connect(m_coordinator, &ChatCoordinator::localIdentityChanged, this, &LanChatManager::localProfileChanged);
    connect(m_coordinator, &ChatCoordinator::runningChanged, this, &LanChatManager::runningChanged);
    connect(m_coordinator, &ChatCoordinator::lastErrorChanged, this, &LanChatManager::lastErrorChanged);
    connect(m_coordinator, &ChatCoordinator::messageReceived, this, &LanChatManager::messageReceived);
    connect(m_coordinator, &ChatCoordinator::windowShakeReceived, this, &LanChatManager::windowShakeReceived);
    connect(m_coordinator, &ChatCoordinator::conversationRemoved, this, &LanChatManager::conversationRemoved);
    connect(m_coordinator, &ChatCoordinator::sendFailed, this, &LanChatManager::sendFailed);
    connect(m_coordinator, &ChatCoordinator::fileReceived, this, &LanChatManager::fileReceived);
    connect(m_coordinator, &ChatCoordinator::fileTransferFailed, this, [this](const QString &conversationId, const QString &reason, bool) {
        emit fileTransferFailed(conversationId, reason);
    });
    connect(m_coordinator, &ChatCoordinator::operationFailed, this, &LanChatManager::operationFailed);
}
