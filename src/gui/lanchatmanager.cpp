#include "lanchatmanager.h"

#include "core/chatservice.h"
#include "desktopfilelauncher.h"
#include "desktopnotificationservice.h"
#include "ifilelauncher.h"
#include "inotificationservice.h"

#include <QCryptographicHash>
#include <QDate>
#include <QFileInfo>
#include <QGuiApplication>
#include <QStringList>

#include <utility>

ChatService *LanChatManager::s_service = nullptr;

void LanChatManager::setService(ChatService *service)
{
    s_service = service;
}

LanChatManager *LanChatManager::create(QQmlEngine *, QJSEngine *)
{
    Q_ASSERT(s_service);
    return new LanChatManager(s_service);
}

LanChatManager::LanChatManager(ChatService *service, QObject *parent)
: LanChatManager(service,
                 std::make_unique<DesktopFileLauncher>(),
                 std::make_unique<DesktopNotificationService>(),
                 parent)
{
}

LanChatManager::LanChatManager(
    ChatService *service,
    std::unique_ptr<IFileLauncher> fileLauncher,
    std::unique_ptr<INotificationService> notificationService,
    QObject *parent)
: QObject(parent)
, m_service(service)
, m_fileLauncher(std::move(fileLauncher))
, m_notificationService(std::move(notificationService))
{
    Q_ASSERT(m_service);
    Q_ASSERT(m_fileLauncher);
    Q_ASSERT(m_notificationService);
    connectService();
    synchronizePeers();
}

LanChatManager::~LanChatManager() = default;

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
    return m_service->localIdentity().displayName;
}

QString LanChatManager::localInitial() const
{
    return initialForName(localName());
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
    return m_service->running();
}

QString LanChatManager::lastError() const
{
    return m_service->lastError();
}

bool LanChatManager::start()
{
    return static_cast<bool>(m_service->start());
}

void LanChatManager::stop()
{
    m_service->stop();
}

bool LanChatManager::selectPeer(const QString &peerId)
{
    Domain::Peer peer;
    if (!m_service->peer(peerId, &peer))
    {
        return false;
    }
    const bool changed = m_currentPeerId != peerId;
    m_currentPeerId = peerId;
    synchronizeConversation(peerId);
    markConversationRead(peerId);
    if (changed)
    {
        emit currentPeerIdChanged();
    }
    return true;
}

bool LanChatManager::markConversationRead(const QString &peerId)
{
    return static_cast<bool>(m_service->markConversationRead(peerId));
}

QVariantMap LanChatManager::peerInfo(const QString &peerId) const
{
    return m_peerModel.peerInfo(peerId);
}

bool LanChatManager::updateLocalProfile(const QString &displayName)
{
    return static_cast<bool>(m_service->updateLocalProfile(displayName));
}

bool LanChatManager::sendMessage(const QString &peerId, const QString &text)
{
    return static_cast<bool>(m_service->sendText(peerId, text));
}

bool LanChatManager::sendFile(const QString &peerId, const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile())
    {
        emit fileTransferFailed(peerId, tr("仅支持发送本地文件。"));
        return false;
    }
    return static_cast<bool>(m_service->sendFile(peerId, fileUrl.toLocalFile()));
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
    return m_service->sendFiles(peerId, paths);
}

bool LanChatManager::cancelFileTransfer(const QString &peerId,
                                        const QString &transferId)
{
    return static_cast<bool>(m_service->cancelFileTransfer(peerId, transferId));
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

void LanChatManager::connectService()
{
    connect(&m_peerModel,
            &PeerListModel::unreadCountChanged,
            this,
            &LanChatManager::totalUnreadCountChanged);
    connect(m_notificationService.get(),
            &INotificationService::notificationActivated,
            this,
            &LanChatManager::notificationActivated);
    connect(m_service,
            &ChatService::localIdentityChanged,
            this,
            &LanChatManager::localProfileChanged);
    connect(m_service,
            &ChatService::peersChanged,
            this,
            &LanChatManager::synchronizePeers);
    connect(m_service,
            &ChatService::conversationChanged,
            this,
            [this](const QString &peerId) {
                if (peerId == m_currentPeerId)
                {
                    synchronizeConversation(peerId);
                    if (QGuiApplication::applicationState()
                        == Qt::ApplicationActive)
                    {
                        static_cast<void>(
                            m_service->markConversationRead(peerId));
                    }
                }
            });
    connect(m_service,
            &ChatService::runningChanged,
            this,
            &LanChatManager::runningChanged);
    connect(m_service,
            &ChatService::lastErrorChanged,
            this,
            &LanChatManager::lastErrorChanged);
    connect(m_service,
            &ChatService::peerDiscovered,
            this,
            [this](const QString &peerId) {
                synchronizePeers();
                emit peerDiscovered(peerId);
            });
    connect(m_service,
            &ChatService::peerUpdated,
            this,
            [this](const QString &peerId) {
                synchronizePeers();
                emit peerUpdated(peerId);
            });
    connect(m_service,
            &ChatService::messageReceived,
            this,
            &LanChatManager::handleIncomingMessage);
    connect(m_service,
            &ChatService::sendFailed,
            this,
            &LanChatManager::sendFailed);
    connect(m_service,
            &ChatService::fileReceived,
            this,
            [this](const QString &peerId, const QString &filePath) {
                if (QGuiApplication::applicationState() != Qt::ApplicationActive)
                {
                    showIncomingNotification(
                        peerId,
                        tr("文件已接收：%1").arg(QFileInfo(filePath).fileName()));
                }
                emit fileReceived(peerId, filePath);
            });
    connect(m_service,
            &ChatService::fileTransferFailed,
            this,
            [this](const QString &peerId,
                   const QString &reason,
                   bool incoming) {
                if (incoming
                    && QGuiApplication::applicationState()
                           != Qt::ApplicationActive)
                {
                    showIncomingNotification(peerId, reason);
                }
                emit fileTransferFailed(peerId, reason);
            });
    connect(m_service,
            &ChatService::operationFailed,
            this,
            &LanChatManager::operationFailed);
}

void LanChatManager::synchronizePeers()
{
    const int previousOnlineCount = onlineCount();
    QList<PeerListModel::Item> items;
    const QList<Domain::Peer> peers = m_service->peers();
    items.reserve(peers.size());
    for (const Domain::Peer &peer : peers)
    {
        PeerListModel::Item item;
        item.endpoint = peer.endpoint;
        item.initial = initialForName(peer.endpoint.displayName);
        item.statusText = peer.online ? tr("在线 · 局域网")
                                      : tr("离线 · 局域网");
        item.lastMessage = peer.lastMessage.isEmpty()
                               ? tr("已通过局域网发现")
                               : peer.lastMessage;
        item.lastTime = displayTime(peer.lastActivity);
        item.avatarColor = colorForId(peer.endpoint.peerId);
        item.online = peer.online;
        item.unread = peer.unreadCount;
        items.append(std::move(item));
    }
    m_peerModel.setItems(std::move(items));
    if (previousOnlineCount != onlineCount())
    {
        emit onlineCountChanged();
    }
}

void LanChatManager::synchronizeConversation(const QString &peerId)
{
    QList<ChatMessageModel::Message> viewMessages;
    const QList<Domain::Message> messages = m_service->messages(peerId);
    viewMessages.reserve(messages.size());
    for (const Domain::Message &message : messages)
    {
        viewMessages.append(toViewMessage(message));
    }
    m_messageModel.setConversation(peerId, std::move(viewMessages));
    m_messageModel.selectPeer(peerId);
}

void LanChatManager::handleIncomingMessage(const QString &peerId,
                                           const QString &text)
{
    if (peerId == m_currentPeerId
        && QGuiApplication::applicationState() == Qt::ApplicationActive)
    {
        static_cast<void>(m_service->markConversationRead(peerId));
    }
    else
    {
        showIncomingNotification(peerId, text);
    }
    emit messageReceived(peerId, text);
}

void LanChatManager::showIncomingNotification(const QString &peerId,
                                              const QString &message)
{
    if (QGuiApplication::applicationState() == Qt::ApplicationActive)
    {
        return;
    }
    Domain::Peer peer;
    QString title = m_service->peer(peerId, &peer)
                        ? peer.endpoint.displayName.trimmed()
                        : QString();
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

ChatMessageModel::Message LanChatManager::toViewMessage(
    const Domain::Message &message) const
{
    Domain::Peer peer;
    static_cast<void>(m_service->peer(message.peerId, &peer));
    ChatMessageModel::Message view;
    view.messageId = message.messageId;
    view.senderInitial = message.fromMe
                             ? localInitial()
                             : initialForName(peer.endpoint.displayName);
    view.senderColor = message.fromMe ? QStringLiteral("#4F7CFF")
                                      : colorForId(message.peerId);
    view.messageText = message.text;
    view.messageTime = displayTime(message.timestamp);
    view.deliveryStatus = Domain::deliveryStateName(message.deliveryState);
    view.messageKind = Domain::messageKindName(message.kind);
    view.fileName = message.fileName;
    view.fileSizeText = displayFileSize(message.fileSize,
                                        message.legacyFileSizeText);
    view.fileProgress = message.fileProgress;
    view.filePath = message.filePath;
    view.fromMe = message.fromMe;
    return view;
}

QString LanChatManager::displayFileSize(qint64 bytes, const QString &fallback)
{
    if (bytes <= 0)
    {
        return fallback;
    }
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
    if (!timestamp.isValid())
    {
        return {};
    }
    const QDateTime localTimestamp = timestamp.toLocalTime();
    const QDate currentDate = QDate::currentDate();
    if (localTimestamp.date() == currentDate)
    {
        return localTimestamp.toString(QStringLiteral("HH:mm"));
    }
    if (localTimestamp.date() == currentDate.addDays(-1))
    {
        return tr("昨天 %1").arg(localTimestamp.toString(QStringLiteral("HH:mm")));
    }
    return localTimestamp.toString(QStringLiteral("MM-dd HH:mm"));
}

QString LanChatManager::initialForName(const QString &name)
{
    const QString trimmed = name.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("?") : trimmed.left(1).toUpper();
}

QString LanChatManager::colorForId(const QString &peerId)
{
    static const QStringList colors = {QStringLiteral("#7C6EE6"),
                                       QStringLiteral("#36A18B"),
                                       QStringLiteral("#D86B87"),
                                       QStringLiteral("#4F8EC9"),
                                       QStringLiteral("#C17A3A"),
                                       QStringLiteral("#6C8D3F")};
    const QByteArray digest = QCryptographicHash::hash(peerId.toUtf8(),
                                                       QCryptographicHash::Sha256);
    if (digest.isEmpty())
    {
        return colors.first();
    }
    const qsizetype index = static_cast<unsigned char>(digest.at(0)) % colors.size();
    return colors.at(index);
}
