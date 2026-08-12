#include "transfercoordinator.h"

#include "conversationstore.h"
#include "domain/ichattransport.h"

#include <QFileInfo>
#include <QUrl>

#include <utility>

TransferCoordinator::TransferCoordinator(IChatTransport *transport,
                                         ConversationStore *conversations,
                                         const Network::LocalIdentity *localIdentity,
                                         QObject *parent)
: QObject(parent)
, m_transport(transport)
, m_conversations(conversations)
, m_localIdentity(localIdentity)
{
    Q_ASSERT(m_transport);
    Q_ASSERT(m_conversations);
    Q_ASSERT(m_localIdentity);
    connect(m_transport, &IChatTransport::fileTransferStarted, this, &TransferCoordinator::handleStarted);
    connect(m_transport, &IChatTransport::fileTransferProgressed, this, &TransferCoordinator::handleProgress);
    connect(m_transport, &IChatTransport::fileTransferFinished, this, &TransferCoordinator::handleFinished);
}

TransferCoordinator::~TransferCoordinator() = default;

Domain::OperationResult TransferCoordinator::sendFile(const Domain::Peer &peer, const QString &filePath)
{
    const QString normalizedPath = QFileInfo(filePath).absoluteFilePath();
    QString error;
    if (!m_transport->sendFile(peer.endpoint, QUrl::fromLocalFile(normalizedPath), &error))
    {
        emit fileTransferFailed(peer.endpoint.peerId, error, false);
        return Domain::OperationResult::failure(QStringLiteral("file.rejected"), error);
    }
    return Domain::OperationResult::success();
}

int TransferCoordinator::sendFiles(const Domain::Peer &peer, const QStringList &filePaths)
{
    constexpr qsizetype MaximumBatchSize = 100;
    if (filePaths.size() > MaximumBatchSize)
    {
        emit operationFailed(tr("单次最多发送 %1 个文件。").arg(MaximumBatchSize));
    }

    int accepted = 0;
    const qsizetype count = qMin(filePaths.size(), MaximumBatchSize);
    for (qsizetype index = 0; index < count; ++index)
    {
        accepted += sendFile(peer, filePaths.at(index)) ? 1 : 0;
    }
    return accepted;
}

Domain::OperationResult TransferCoordinator::cancel(const QString &peerId, const QString &transferId)
{
    if (!m_transport->cancelFileTransfer(peerId, transferId))
    {
        const QString error = tr("该文件传输已经结束或不存在。");
        emit fileTransferFailed(peerId, error, false);
        return Domain::OperationResult::failure(QStringLiteral("transfer.not_found"), error);
    }
    return Domain::OperationResult::success();
}

void TransferCoordinator::handleStarted(const Network::FileTransferInfo &transfer)
{
    const bool outgoing = transfer.direction == Network::TransferDirection::Outgoing;
    if (!outgoing)
    {
        m_conversations->observePeer(transfer.peer);
    }

    Domain::Message message;
    message.messageId = transfer.transferId;
    message.conversationId = Domain::directConversationId(transfer.peer.peerId);
    message.senderId = outgoing ? m_localIdentity->deviceId : transfer.peer.peerId;
    message.timestamp = transfer.timestamp;
    message.deliveryState = outgoing ? Domain::DeliveryState::Transferring : Domain::DeliveryState::Receiving;
    message.kind = Domain::MessageKind::File;
    message.fileName = transfer.fileName;
    message.filePath = transfer.filePath;
    message.fileSize = transfer.fileSize;
    static_cast<void>(m_conversations->appendMessage(std::move(message), tr("[文件] %1").arg(transfer.fileName), !outgoing));
}

void TransferCoordinator::handleProgress(const Network::FileTransferProgress &progress)
{
    const Domain::DeliveryState state =
        progress.direction == Network::TransferDirection::Outgoing ? Domain::DeliveryState::Transferring : Domain::DeliveryState::Receiving;
    m_conversations->updateFileTransfer(Domain::directConversationId(progress.peerId), progress.transferId, progress.progress, state);
}

void TransferCoordinator::handleFinished(const Network::FileTransferResult &result)
{
    if (result.success)
    {
        const bool outgoing = result.direction == Network::TransferDirection::Outgoing;
        const Domain::DeliveryState state = outgoing ? Domain::DeliveryState::Sent : Domain::DeliveryState::Received;
        m_conversations->updateFileTransfer(Domain::directConversationId(result.peerId), result.transferId, 1.0, state, result.filePath);
        if (!outgoing)
        {
            emit fileReceived(result.peerId, result.filePath);
        }
        return;
    }

    const Domain::DeliveryState state = result.cancelled ? Domain::DeliveryState::Cancelled : Domain::DeliveryState::Failed;
    m_conversations->updateMessageState(Domain::directConversationId(result.peerId), result.transferId, state);
    if (!result.cancelled)
    {
        const bool incoming = result.direction == Network::TransferDirection::Incoming;
        const QString reason = incoming ? tr("文件接收失败：%1").arg(result.errorMessage) : tr("文件发送失败：%1").arg(result.errorMessage);
        emit fileTransferFailed(result.peerId, reason, incoming);
    }
}
