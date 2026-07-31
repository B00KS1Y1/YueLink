#include "desktopintegration.h"

#include "application/chatcoordinator.h"
#include "conversationviewmodel.h"
#include "ifilelauncher.h"
#include "inotificationservice.h"

#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QMimeData>
#include <QStandardPaths>
#include <QUuid>

#include <utility>

namespace
{
void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage)
    {
        *errorMessage = message;
    }
}
} // namespace

DesktopIntegration::DesktopIntegration(
    ChatCoordinator *coordinator,
    ConversationViewModel *conversation,
    std::unique_ptr<IFileLauncher> fileLauncher,
    std::unique_ptr<INotificationService> notificationService,
    QObject *parent)
: QObject(parent)
, m_coordinator(coordinator)
, m_conversation(conversation)
, m_fileLauncher(std::move(fileLauncher))
, m_notificationService(std::move(notificationService))
{
    Q_ASSERT(m_coordinator);
    Q_ASSERT(m_conversation);
    Q_ASSERT(m_fileLauncher);
    Q_ASSERT(m_notificationService);
    connectServices();
}

DesktopIntegration::~DesktopIntegration() = default;

QList<QUrl> DesktopIntegration::clipboardImageUrls(QString *errorMessage)
{
    setError(errorMessage, {});
    const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    if (!mimeData)
    {
        return {};
    }

    QList<QUrl> imageUrls;
    for (const QUrl &url : mimeData->urls())
    {
        if (url.isLocalFile()
            && !QImageReader::imageFormat(url.toLocalFile()).isEmpty())
        {
            imageUrls.append(url);
        }
    }
    if (!imageUrls.isEmpty() || !mimeData->hasImage())
    {
        return imageUrls;
    }

    const QImage image = QGuiApplication::clipboard()->image();
    if (image.isNull())
    {
        return {};
    }

    const QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    const QString imageDirectoryPath = QDir(cachePath).filePath(QStringLiteral("clipboard-images"));
    if (cachePath.isEmpty() || !QDir().mkpath(imageDirectoryPath))
    {
        setError(errorMessage, tr("无法保存剪贴板图片。"));
        return {};
    }

    const QString imagePath = QDir(imageDirectoryPath).filePath(
        QStringLiteral("clipboard-%1.png").arg(
            QUuid::createUuid().toString(QUuid::WithoutBraces)));
    if (!image.save(imagePath, "PNG"))
    {
        setError(errorMessage, tr("无法保存剪贴板图片。"));
        return {};
    }
    return {QUrl::fromLocalFile(imagePath)};
}

bool DesktopIntegration::openFile(const QString &filePath,
                                  QString *errorMessage)
{
    return m_fileLauncher->openFile(filePath, errorMessage);
}

bool DesktopIntegration::revealFile(const QString &filePath,
                                    QString *errorMessage)
{
    return m_fileLauncher->revealInFolder(filePath, errorMessage);
}

void DesktopIntegration::setNotificationsEnabled(bool enabled)
{
    m_notificationService->setEnabled(enabled);
}

void DesktopIntegration::connectServices()
{
    connect(m_notificationService.get(),
            &INotificationService::notificationActivated,
            this,
            &DesktopIntegration::notificationActivated);
    connect(m_coordinator,
            &ChatCoordinator::messageReceived,
            this,
            &DesktopIntegration::handleIncomingMessage);
    connect(m_coordinator,
            &ChatCoordinator::fileReceived,
            this,
            &DesktopIntegration::handleFileReceived);
    connect(m_coordinator,
            &ChatCoordinator::fileTransferFailed,
            this,
            &DesktopIntegration::handleFileTransferFailed);
}

void DesktopIntegration::handleIncomingMessage(const QString &peerId,
                                               const QString &text)
{
    if (peerId == m_conversation->currentPeerId()
        && QGuiApplication::applicationState() == Qt::ApplicationActive)
    {
        static_cast<void>(m_coordinator->markConversationRead(peerId));
        return;
    }
    showIncomingNotification(peerId, text);
}

void DesktopIntegration::handleFileReceived(const QString &peerId,
                                            const QString &filePath)
{
    if (QGuiApplication::applicationState() != Qt::ApplicationActive)
    {
        showIncomingNotification(peerId,
                                 tr("文件已接收：%1").arg(
                                     QFileInfo(filePath).fileName()));
    }
}

void DesktopIntegration::handleFileTransferFailed(const QString &peerId,
                                                  const QString &reason,
                                                  bool incoming)
{
    if (incoming
        && QGuiApplication::applicationState() != Qt::ApplicationActive)
    {
        showIncomingNotification(peerId, reason);
    }
}

void DesktopIntegration::showIncomingNotification(const QString &peerId,
                                                  const QString &message)
{
    if (QGuiApplication::applicationState() == Qt::ApplicationActive)
    {
        return;
    }
    Domain::Peer peer;
    QString title = m_coordinator->peer(peerId, &peer)
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
