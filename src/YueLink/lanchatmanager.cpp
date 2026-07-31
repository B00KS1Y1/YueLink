#include "lanchatmanager.h"

#include "application/chatcoordinator.h"
#include "YueLink/conversationviewmodel.h"
#include "YueLink/desktopfilelauncher.h"
#include "YueLink/desktopintegration.h"
#include "YueLink/desktopnotificationservice.h"
#include "YueLink/peerlistviewmodel.h"

#include <QStringList>

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
: LanChatManager(coordinator,
                 std::make_unique<DesktopFileLauncher>(),
                 std::make_unique<DesktopNotificationService>(),
                 parent)
{
}

LanChatManager::LanChatManager(
    ChatCoordinator *coordinator,
    std::unique_ptr<IFileLauncher> fileLauncher,
    std::unique_ptr<INotificationService> notificationService,
    QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
, m_peers(std::make_unique<PeerListViewModel>(coordinator))
, m_conversation(std::make_unique<ConversationViewModel>(coordinator))
, m_desktop(std::make_unique<DesktopIntegration>(coordinator,
                                                 m_conversation.get(),
                                                 std::move(fileLauncher),
                                                 std::move(notificationService)))
{
    Q_ASSERT(m_coordinator);
    Q_ASSERT(m_peers);
    Q_ASSERT(m_conversation);
    Q_ASSERT(m_desktop);
    connectComponents();
}

LanChatManager::~LanChatManager() = default;

QAbstractItemModel *LanChatManager::peers()
{
    return m_peers->model();
}

QAbstractItemModel *LanChatManager::messages()
{
    return m_conversation->model();
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

QString LanChatManager::currentPeerId() const
{
    return m_conversation->currentPeerId();
}

int LanChatManager::onlineCount() const
{
    return m_peers->onlineCount();
}

int LanChatManager::totalUnreadCount() const
{
    return m_peers->totalUnreadCount();
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

bool LanChatManager::selectPeer(const QString &peerId)
{
    return m_conversation->selectPeer(peerId);
}

bool LanChatManager::markConversationRead(const QString &peerId)
{
    return static_cast<bool>(m_coordinator->markConversationRead(peerId));
}

QVariantMap LanChatManager::peerInfo(const QString &peerId) const
{
    return m_peers->peerInfo(peerId);
}

bool LanChatManager::updateLocalProfile(const QString &displayName,
                                        const QUrl &avatarUrl,
                                        const QString &avatarColor)
{
    if (!avatarUrl.isEmpty() && !avatarUrl.isLocalFile())
    {
        emit operationFailed(tr("仅支持使用本地头像图片。"));
        return false;
    }
    return static_cast<bool>(m_coordinator->updateLocalProfile(
        displayName,
        avatarUrl.isEmpty() ? QString{} : avatarUrl.toLocalFile(),
        avatarColor));
}

bool LanChatManager::sendMessage(const QString &peerId, const QString &text)
{
    return static_cast<bool>(m_coordinator->sendText(peerId, text));
}

bool LanChatManager::sendFile(const QString &peerId, const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile())
    {
        emit fileTransferFailed(peerId, tr("仅支持发送本地文件。"));
        return false;
    }
    return static_cast<bool>(m_coordinator->sendFile(peerId,
                                                     fileUrl.toLocalFile()));
}

int LanChatManager::sendFiles(const QString &peerId,
                              const QList<QUrl> &fileUrls)
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
    return m_coordinator->sendFiles(peerId, paths);
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

bool LanChatManager::cancelFileTransfer(const QString &peerId,
                                        const QString &transferId)
{
    return static_cast<bool>(m_coordinator->cancelFileTransfer(peerId,
                                                               transferId));
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
    connect(m_peers.get(),
            &PeerListViewModel::searchTextChanged,
            this,
            &LanChatManager::peerSearchTextChanged);
    connect(m_peers.get(),
            &PeerListViewModel::onlineCountChanged,
            this,
            &LanChatManager::onlineCountChanged);
    connect(m_peers.get(),
            &PeerListViewModel::totalUnreadCountChanged,
            this,
            &LanChatManager::totalUnreadCountChanged);
    connect(m_peers.get(),
            &PeerListViewModel::peerDiscovered,
            this,
            &LanChatManager::peerDiscovered);
    connect(m_peers.get(),
            &PeerListViewModel::peerUpdated,
            this,
            &LanChatManager::peerUpdated);
    connect(m_conversation.get(),
            &ConversationViewModel::searchTextChanged,
            this,
            &LanChatManager::messageSearchTextChanged);
    connect(m_conversation.get(),
            &ConversationViewModel::currentPeerIdChanged,
            this,
            &LanChatManager::currentPeerIdChanged);
    connect(m_desktop.get(),
            &DesktopIntegration::notificationActivated,
            this,
            &LanChatManager::notificationActivated);

    connect(m_coordinator,
            &ChatCoordinator::localIdentityChanged,
            this,
            &LanChatManager::localProfileChanged);
    connect(m_coordinator,
            &ChatCoordinator::runningChanged,
            this,
            &LanChatManager::runningChanged);
    connect(m_coordinator,
            &ChatCoordinator::lastErrorChanged,
            this,
            &LanChatManager::lastErrorChanged);
    connect(m_coordinator,
            &ChatCoordinator::messageReceived,
            this,
            &LanChatManager::messageReceived);
    connect(m_coordinator,
            &ChatCoordinator::sendFailed,
            this,
            &LanChatManager::sendFailed);
    connect(m_coordinator,
            &ChatCoordinator::fileReceived,
            this,
            &LanChatManager::fileReceived);
    connect(m_coordinator,
            &ChatCoordinator::fileTransferFailed,
            this,
            [this](const QString &peerId,
                   const QString &reason,
                   bool) {
                emit fileTransferFailed(peerId, reason);
            });
    connect(m_coordinator,
            &ChatCoordinator::operationFailed,
            this,
            &LanChatManager::operationFailed);
}
