#include "transfercoordinator.h"

#include "conversationstore.h"
#include "domain/ichattransport.h"

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
    connect(m_transport, &IChatTransport::attachmentTransferStarted, this, &TransferCoordinator::handleStarted);
    connect(m_transport, &IChatTransport::attachmentTransferProgressed, this, &TransferCoordinator::handleProgress);
    connect(m_transport, &IChatTransport::attachmentTransferFinished, this, &TransferCoordinator::handleFinished);
}

TransferCoordinator::~TransferCoordinator() = default;

Domain::OperationResult TransferCoordinator::sendAttachment(const Domain::Peer &peer, const Domain::Message &message)
{
    QString error;
    if (!m_transport->sendMessage(peer.endpoint, message, &error))
    {
        emit fileTransferFailed(peer.endpoint.peerId, error, false);
        return Domain::OperationResult::failure(QStringLiteral("file.rejected"), error);
    }
    return Domain::OperationResult::success(message.metadata.messageId);
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

Domain::OperationResult TransferCoordinator::accept(const QString &peerId, const QString &transferId)
{
    QString error;
    if (!m_transport->acceptFileTransfer(peerId, transferId, &error))
    {
        emit fileTransferFailed(peerId, error, true);
        return Domain::OperationResult::failure(QStringLiteral("transfer.not_found"), error);
    }
    return Domain::OperationResult::success();
}

void TransferCoordinator::handleStarted(const Domain::AttachmentTransferInfo &transfer)
{
    const bool outgoing = transfer.direction == Network::TransferDirection::Outgoing;
    if (!outgoing)
    {
        m_conversations->observePeer(transfer.peer);
    }

    Domain::Message message = transfer.message;
    message.metadata.conversationId = Domain::directConversationId(transfer.peer.peerId);
    message.metadata.senderId = outgoing ? m_localIdentity->deviceId : transfer.peer.peerId;
    const bool requiresAcceptance = Domain::messageKind(message) == Domain::MessageKind::File;
    message.deliveryState = requiresAcceptance ? Domain::DeliveryState::AwaitingAcceptance
                            : outgoing         ? Domain::DeliveryState::Transferring
                                               : Domain::DeliveryState::Receiving;
    static_cast<void>(m_conversations->appendMessage(message, Domain::messageSummary(message), !outgoing));
}

void TransferCoordinator::handleProgress(const Domain::AttachmentTransferProgress &progress)
{
    const Domain::DeliveryState state =
        progress.direction == Network::TransferDirection::Outgoing ? Domain::DeliveryState::Transferring : Domain::DeliveryState::Receiving;
    m_conversations->updateFileTransfer(Domain::directConversationId(progress.peerId), progress.messageId, progress.progress, state);
}

void TransferCoordinator::handleFinished(const Domain::AttachmentTransferResult &result)
{
    if (result.success)
    {
        const bool outgoing = result.direction == Network::TransferDirection::Outgoing;
        const Domain::DeliveryState state = outgoing ? Domain::DeliveryState::Sent : Domain::DeliveryState::Received;
        m_conversations->updateFileTransfer(Domain::directConversationId(result.peerId), result.messageId, 1.0, state, result.filePath);
        if (!outgoing)
        {
            emit fileReceived(result.peerId, result.filePath);
        }
        return;
    }

    const Domain::DeliveryState state = result.cancelled ? Domain::DeliveryState::Cancelled : Domain::DeliveryState::Failed;
    m_conversations->updateMessageState(Domain::directConversationId(result.peerId), result.messageId, state);
    if (!result.cancelled)
    {
        const bool incoming = result.direction == Network::TransferDirection::Incoming;
        const QString reason = incoming ? tr("文件接收失败：%1").arg(result.errorMessage) : tr("文件发送失败：%1").arg(result.errorMessage);
        emit fileTransferFailed(result.peerId, reason, incoming);
    }
}
