#include "tcpchattransport.h"

#include "config/configapi.h"
#include "infrastructure/path.h"
#include "wireprotocol.h"

#include <QDir>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QNetworkProxy>
#include <QSaveFile>
#include <QTcpSocket>
#include <QUuid>

#include <QsLog.h>

namespace
{
constexpr qint64 FileChunkSize = 64 * 1024;
constexpr qint64 MaximumPendingBytes = 256 * 1024;
constexpr qreal ProgressStep = 0.01;

struct FileTransferData
{
    Domain::Message message;
    Network::PeerEndpoint peer;
    QString transferId;
    QString fileName;
    QString filePath;
    qint64 fileSize = 0;
    QDateTime timestamp;
    Network::TransferDirection direction = Network::TransferDirection::Outgoing;
};
} // namespace

struct TcpChatTransport::OutgoingFileTransfer
{
    FileTransferData info;
    QFile file;
    qint64 headerSize = 0;
    qint64 socketBytesWritten = 0;
    qint64 lastActivityMs = 0;
    qreal reportedProgress = 0.0;
};

struct TcpChatTransport::IncomingFileTransfer
{
    FileTransferData info;
    QSaveFile file;
    QCryptographicHash hash{QCryptographicHash::Sha256};
    qint64 receivedBytes = 0;
    qint64 lastActivityMs = 0;
    qreal reportedProgress = 0.0;
};

bool TcpChatTransport::sendAttachment(const Network::PeerEndpoint &peer,
                                      const Domain::Message &message,
                                      QString *errorMessage)
{
    const auto fail = [errorMessage](const QString &message) {
        QLOG_WARN() << QStringLiteral("[网络.文件] 已拒绝文件发送请求 原因=") << message;
        if (errorMessage)
        {
            *errorMessage = message;
        }
        return false;
    };

    if (!m_running)
    {
        return fail(tr("TCP 服务尚未启动。"));
    }
    if (!peer.isValid())
    {
        return fail(tr("好友的网络地址无效。"));
    }
    const Domain::AttachmentDescriptor *attachment = Domain::messageAttachment(message);
    if (!attachment || message.metadata.messageId.isEmpty()
        || message.localAttachment.filePath.isEmpty())
    {
        return fail(tr("附件消息无效。"));
    }

    const QFileInfo fileInfo(message.localAttachment.filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        return fail(tr("要发送的文件不存在。"));
    }
    if (fileInfo.size() < 0 || fileInfo.size() > Network::WireProtocol::MaximumFileSize)
    {
        return fail(tr("文件大小不能超过 2 GB。"));
    }
    if (attachment->fileName != fileInfo.fileName()
        || attachment->fileSize != fileInfo.size())
    {
        return fail(tr("附件信息与本地文件不一致。"));
    }

    auto *transfer = new OutgoingFileTransfer;
    transfer->info.message = message;
    transfer->info.transferId = message.metadata.messageId;
    transfer->info.peer = peer;
    transfer->info.fileName = fileInfo.fileName();
    transfer->info.filePath = fileInfo.absoluteFilePath();
    transfer->info.fileSize = fileInfo.size();
    transfer->info.timestamp = message.metadata.timestamp;
    transfer->info.direction = Network::TransferDirection::Outgoing;
    transfer->file.setFileName(transfer->info.filePath);
    if (!transfer->file.open(QIODevice::ReadOnly))
    {
        const QString reason = transfer->file.errorString();
        delete transfer;
        return fail(tr("无法读取文件：%1").arg(reason));
    }
    transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();

    auto *socket = new QTcpSocket(this);
    socket->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    m_outgoingFiles.insert(socket, transfer);
    QLOG_INFO() << QStringLiteral("[网络.文件] 文件发送已开始 好友标识=") << peer.peerId
                          << QStringLiteral("传输标识=") << transfer->info.transferId
                          << QStringLiteral("文件名=") << transfer->info.fileName
                          << QStringLiteral("大小=") << transfer->info.fileSize;

    connect(socket, &QTcpSocket::connected, this, [this, socket]() {
        OutgoingFileTransfer *transfer = m_outgoingFiles.value(socket);
        if (!transfer)
        {
            return;
        }
        QLOG_DEBUG() << QStringLiteral("[网络.文件] 文件发送连接已建立 好友标识=") << transfer->info.peer.peerId
                               << QStringLiteral("传输标识=") << transfer->info.transferId;

        const QByteArray header = Network::WireProtocol::attachmentHeaderFrame(
            m_identity, listeningPort(), transfer->info.peer,
            transfer->info.message);
        transfer->headerSize = header.size();
        transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();
        if (socket->write(header) != header.size())
        {
            failOutgoingFile(socket, socket->errorString());
            return;
        }
        pumpOutgoingFile(socket);
    });
    connect(socket, &QTcpSocket::bytesWritten, this, [this, socket](qint64 bytes) {
        OutgoingFileTransfer *transfer = m_outgoingFiles.value(socket);
        if (!transfer)
        {
            return;
        }

        transfer->socketBytesWritten += bytes;
        transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();
        const qint64 payloadBytes = qBound<qint64>(0, transfer->socketBytesWritten - transfer->headerSize, transfer->info.fileSize);
        const qreal progress = transfer->info.fileSize == 0 ? 1.0 : static_cast<qreal>(payloadBytes) / static_cast<qreal>(transfer->info.fileSize);
        if (progress >= 1.0 || progress - transfer->reportedProgress >= ProgressStep)
        {
            transfer->reportedProgress = progress;
            QLOG_TRACE() << QStringLiteral("[网络.文件] 文件发送进度 好友标识=") << transfer->info.peer.peerId
                                   << QStringLiteral("传输标识=") << transfer->info.transferId
                                   << QStringLiteral("进度=") << QString::number(progress * 100.0, 'f', 0) << QLatin1Char('%');
            emit attachmentTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Outgoing, progress});
        }
        pumpOutgoingFile(socket);
    });
    connect(socket, &QTcpSocket::errorOccurred, this, [this, socket](QAbstractSocket::SocketError) {
        failOutgoingFile(socket, socket->errorString());
    });
    connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
        if (m_outgoingFiles.contains(socket))
        {
            failOutgoingFile(socket, tr("文件传输连接已中断。"));
        }
        socket->deleteLater();
    });

    emit attachmentTransferStarted({transfer->info.message,
                                    transfer->info.peer,
                                    transfer->info.direction});
    socket->connectToHost(peer.address, peer.tcpPort);
    if (errorMessage)
    {
        errorMessage->clear();
    }
    return true;
}

bool TcpChatTransport::cancelFileTransfer(const QString &peerId, const QString &transferId)
{
    if (peerId.isEmpty() || transferId.isEmpty())
    {
        return false;
    }

    for (QTcpSocket *socket : m_outgoingFiles.keys())
    {
        const OutgoingFileTransfer *transfer = m_outgoingFiles.value(socket);
        if (transfer && transfer->info.peer.peerId == peerId && transfer->info.transferId == transferId)
        {
            failOutgoingFile(socket, tr("文件发送已由用户取消。"), true);
            return true;
        }
    }
    for (QTcpSocket *socket : m_incomingFiles.keys())
    {
        const IncomingFileTransfer *transfer = m_incomingFiles.value(socket);
        if (transfer && transfer->info.peer.peerId == peerId && transfer->info.transferId == transferId)
        {
            failIncomingFile(socket, tr("文件接收已由用户取消。"), true);
            return true;
        }
    }
    return false;
}

bool TcpChatTransport::handleIncomingAttachmentHeader(const QJsonObject &object,
                                                      QTcpSocket *socket)
{
    if (!Network::WireProtocol::isEnvelopeFor(object, m_identity))
    {
        QLOG_DEBUG() << QStringLiteral("[网络.文件] 已忽略发给其他接收方的文件信封 地址=")
                               << socket->peerAddress().toString();
        return false;
    }

    const Network::PeerEndpoint peer = incomingPeer(object, socket);
    const QString transferId = object.value(QStringLiteral("messageId")).toString().trimmed();
    const QString conversationId = object.value(QStringLiteral("conversationId")).toString();
    const Domain::MessageKind kind = Domain::messageKindFromName(
        object.value(QStringLiteral("kind")).toString());
    const auto payload = Domain::messagePayloadFromJson(
        kind, object.value(QStringLiteral("payload")).toObject());
    if (!payload || (kind != Domain::MessageKind::Image && kind != Domain::MessageKind::File))
    {
        return false;
    }
    Domain::Message message;
    message.metadata = {transferId, Domain::directConversationId(peer.peerId),
                        peer.peerId, timestampFrom(object)};
    message.payload = *payload;
    message.deliveryState = Domain::DeliveryState::Receiving;
    const Domain::AttachmentDescriptor *attachment = Domain::messageAttachment(message);
    Q_ASSERT(attachment);
    QString fileName = attachment->fileName.trimmed();
    QString safeFileName = fileName;
    safeFileName.replace(QLatin1Char('\\'), QLatin1Char('/'));
    safeFileName = safeFileName.section(QLatin1Char('/'), -1);
    const qint64 fileSize = attachment->fileSize;
    if (!peer.isValid() || peer.peerId == m_identity.deviceId || transferId.isEmpty() || transferId.size() > 128 || fileName != safeFileName || fileName.isEmpty() ||
        fileName == QLatin1String(".") || fileName == QLatin1String("..") || fileName.size() > 255 || fileName.contains(QChar::Null) || fileSize < 0 ||
        fileSize > Network::WireProtocol::MaximumFileSize || !conversationId.isEmpty()
        || !rememberEventId(transferId))
    {
        QLOG_WARN() << QStringLiteral("[网络.文件] 接收文件头无效或重复 地址=") << socket->peerAddress().toString()
                              << QStringLiteral("传输标识=") << transferId
                              << QStringLiteral("文件大小=") << fileSize;
        return false;
    }

    const QString receivePath = uniqueReceivePath(fileName);
    if (receivePath.isEmpty())
    {
        QLOG_ERROR() << QStringLiteral("[网络.文件] 创建文件接收目录失败 传输标识=") << transferId;
        setLastError(tr("无法创建文件接收目录。"));
        return false;
    }

    auto *transfer = new IncomingFileTransfer;
    transfer->info.message = std::move(message);
    transfer->info.transferId = transferId;
    transfer->info.peer = peer;
    transfer->info.fileName = fileName;
    transfer->info.filePath = receivePath;
    transfer->info.fileSize = fileSize;
    transfer->info.timestamp = timestampFrom(object);
    transfer->info.direction = Network::TransferDirection::Incoming;
    transfer->info.message.localAttachment.filePath = receivePath;
    transfer->file.setFileName(receivePath);
    if (!transfer->file.open(QIODevice::WriteOnly))
    {
        const QString reason = transfer->file.errorString();
        QLOG_ERROR() << QStringLiteral("[网络.文件] 打开接收文件失败 传输标识=") << transferId
                               << QStringLiteral("原因=") << reason;
        delete transfer;
        setLastError(tr("无法接收文件：%1").arg(reason));
        return false;
    }
    transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();
    m_incomingFiles.insert(socket, transfer);

    QLOG_INFO() << QStringLiteral("[网络.文件] 文件接收已开始 好友标识=") << peer.peerId
                          << QStringLiteral("传输标识=") << transferId
                          << QStringLiteral("文件名=") << fileName
                          << QStringLiteral("大小=") << fileSize;
    emit peerObserved(peer);
    emit attachmentTransferStarted({transfer->info.message,
                                    transfer->info.peer,
                                    transfer->info.direction});
    return true;
}

bool TcpChatTransport::consumeIncomingFile(QTcpSocket *socket, QByteArray &buffer)
{
    IncomingFileTransfer *transfer = m_incomingFiles.value(socket);
    if (!transfer)
    {
        return false;
    }

    const qint64 remainingBytes = transfer->info.fileSize - transfer->receivedBytes;
    const qsizetype consumedBytes = static_cast<qsizetype>(qMin<qint64>(remainingBytes, buffer.size()));
    if (consumedBytes > 0)
    {
        const QByteArray chunk(buffer.constData(), consumedBytes);
        const qint64 writtenBytes = transfer->file.write(buffer.constData(), consumedBytes);
        if (writtenBytes != consumedBytes)
        {
            const QString reason = transfer->file.errorString();
            failIncomingFile(socket, tr("写入接收文件失败：%1").arg(reason));
            return false;
        }
        transfer->receivedBytes += writtenBytes;
        transfer->hash.addData(chunk);
        buffer.remove(0, consumedBytes);
        transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();

        const qreal progress = transfer->info.fileSize == 0 ? 1.0 : static_cast<qreal>(transfer->receivedBytes) / static_cast<qreal>(transfer->info.fileSize);
        if (progress >= 1.0 || progress - transfer->reportedProgress >= ProgressStep)
        {
            transfer->reportedProgress = progress;
            QLOG_TRACE() << QStringLiteral("[网络.文件] 文件接收进度 好友标识=") << transfer->info.peer.peerId
                                   << QStringLiteral("传输标识=") << transfer->info.transferId
                                   << QStringLiteral("进度=") << QString::number(progress * 100.0, 'f', 0) << QLatin1Char('%');
            emit attachmentTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Incoming, progress});
        }
    }

    if (transfer->receivedBytes < transfer->info.fileSize)
    {
        return false;
    }

    const Domain::AttachmentDescriptor *attachment =
        Domain::messageAttachment(transfer->info.message);
    if (!attachment || transfer->hash.result() != attachment->sha256)
    {
        failIncomingFile(socket, tr("文件校验失败。"));
        return false;
    }

    if (!transfer->file.commit())
    {
        const QString reason = transfer->file.errorString();
        failIncomingFile(socket, tr("保存接收文件失败：%1").arg(reason));
        return false;
    }

    transfer = m_incomingFiles.take(socket);
    if (transfer->reportedProgress < 1.0)
    {
        emit attachmentTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Incoming, 1.0});
    }
    QLOG_INFO() << QStringLiteral("[网络.文件] 文件接收已完成 好友标识=") << transfer->info.peer.peerId
                          << QStringLiteral("传输标识=") << transfer->info.transferId
                          << QStringLiteral("文件名=") << transfer->info.fileName
                          << QStringLiteral("大小=") << transfer->info.fileSize;
    emit attachmentTransferFinished({transfer->info.peer.peerId, transfer->info.transferId, transfer->info.filePath, {}, Network::TransferDirection::Incoming, true});
    delete transfer;
    return true;
}

void TcpChatTransport::failIncomingFile(QTcpSocket *socket, const QString &reason, bool cancelled)
{
    IncomingFileTransfer *transfer = m_incomingFiles.take(socket);
    if (!transfer)
    {
        return;
    }

    if (cancelled)
    {
        QLOG_INFO() << QStringLiteral("[网络.文件] 文件接收已取消 好友标识=") << transfer->info.peer.peerId
                              << QStringLiteral("传输标识=") << transfer->info.transferId
                              << QStringLiteral("文件名=") << transfer->info.fileName;
    }
    else
    {
        QLOG_WARN() << QStringLiteral("[网络.文件] 文件接收失败 好友标识=") << transfer->info.peer.peerId
                              << QStringLiteral("传输标识=") << transfer->info.transferId
                              << QStringLiteral("文件名=") << transfer->info.fileName
                              << QStringLiteral("原因=") << reason;
    }
    transfer->file.cancelWriting();
    emit attachmentTransferFinished(
        {transfer->info.peer.peerId, transfer->info.transferId, transfer->info.filePath, reason, Network::TransferDirection::Incoming, false, cancelled});
    delete transfer;
    socket->abort();
    socket->deleteLater();
}

void TcpChatTransport::pumpOutgoingFile(QTcpSocket *socket)
{
    OutgoingFileTransfer *transfer = m_outgoingFiles.value(socket);
    if (!transfer || socket->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }

    while (transfer->file.pos() < transfer->info.fileSize && socket->bytesToWrite() < MaximumPendingBytes)
    {
        const qint64 availableBuffer = MaximumPendingBytes - socket->bytesToWrite();
        const qint64 bytesToRead = qMin(FileChunkSize, qMin(availableBuffer, transfer->info.fileSize - transfer->file.pos()));
        const QByteArray chunk = transfer->file.read(bytesToRead);
        if (chunk.size() != bytesToRead)
        {
            failOutgoingFile(socket, tr("读取发送文件失败：%1").arg(transfer->file.errorString()));
            return;
        }
        if (socket->write(chunk) != chunk.size())
        {
            failOutgoingFile(socket, socket->errorString());
            return;
        }
    }

    transfer = m_outgoingFiles.value(socket);
    if (transfer && transfer->file.pos() == transfer->info.fileSize && socket->bytesToWrite() == 0 &&
        transfer->socketBytesWritten >= transfer->headerSize + transfer->info.fileSize)
    {
        finishOutgoingFile(socket);
    }
}

void TcpChatTransport::finishOutgoingFile(QTcpSocket *socket)
{
    OutgoingFileTransfer *transfer = m_outgoingFiles.take(socket);
    if (!transfer)
    {
        return;
    }

    transfer->file.close();
    if (transfer->reportedProgress < 1.0)
    {
        emit attachmentTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Outgoing, 1.0});
    }
    QLOG_INFO() << QStringLiteral("[网络.文件] 文件发送已完成 好友标识=") << transfer->info.peer.peerId
                          << QStringLiteral("传输标识=") << transfer->info.transferId
                          << QStringLiteral("文件名=") << transfer->info.fileName
                          << QStringLiteral("大小=") << transfer->info.fileSize;
    emit attachmentTransferFinished({transfer->info.peer.peerId, transfer->info.transferId, transfer->info.filePath, {}, Network::TransferDirection::Outgoing, true});
    delete transfer;
    socket->disconnectFromHost();
}

void TcpChatTransport::failOutgoingFile(QTcpSocket *socket, const QString &reason, bool cancelled)
{
    OutgoingFileTransfer *transfer = m_outgoingFiles.take(socket);
    if (!transfer)
    {
        return;
    }

    if (cancelled)
    {
        QLOG_INFO() << QStringLiteral("[网络.文件] 文件发送已取消 好友标识=") << transfer->info.peer.peerId
                              << QStringLiteral("传输标识=") << transfer->info.transferId
                              << QStringLiteral("文件名=") << transfer->info.fileName;
    }
    else
    {
        QLOG_WARN() << QStringLiteral("[网络.文件] 文件发送失败 好友标识=") << transfer->info.peer.peerId
                              << QStringLiteral("传输标识=") << transfer->info.transferId
                              << QStringLiteral("文件名=") << transfer->info.fileName
                              << QStringLiteral("原因=") << reason;
    }
    transfer->file.close();
    emit attachmentTransferFinished(
        {transfer->info.peer.peerId, transfer->info.transferId, transfer->info.filePath, reason, Network::TransferDirection::Outgoing, false, cancelled});
    delete transfer;
    socket->abort();
    socket->deleteLater();
}

void TcpChatTransport::expireFileTransfers()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    for (QTcpSocket *socket : m_outgoingFiles.keys())
    {
        const OutgoingFileTransfer *transfer = m_outgoingFiles.value(socket);
        if (transfer && nowMs - transfer->lastActivityMs > FileTransferTimeoutMs)
        {
            failOutgoingFile(socket, tr("文件发送超时。"));
        }
    }
    for (QTcpSocket *socket : m_incomingFiles.keys())
    {
        const IncomingFileTransfer *transfer = m_incomingFiles.value(socket);
        if (transfer && nowMs - transfer->lastActivityMs > FileTransferTimeoutMs)
        {
            failIncomingFile(socket, tr("文件接收超时。"));
        }
    }
}

QString TcpChatTransport::uniqueReceivePath(const QString &fileName) const
{
    const QString receiveDirectory = QString::fromStdString(Config::value<Config::ApplicationConfig>().download_directory);
    if (!QDir().mkpath(receiveDirectory))
    {
        QLOG_ERROR() << QStringLiteral("[网络.文件] 创建文件接收目录失败");
        return {};
    }

    const QDir directory(receiveDirectory);
    const QFileInfo nameInfo(fileName);
    QString stem = nameInfo.completeBaseName();
    const QString suffix = nameInfo.completeSuffix();
    if (stem.isEmpty())
    {
        stem = fileName;
    }

    const auto isAvailable = [this](const QString &path) {
        if (QFileInfo::exists(path))
        {
            return false;
        }
        for (const IncomingFileTransfer *transfer : m_incomingFiles)
        {
            if (transfer && transfer->info.filePath == path)
            {
                return false;
            }
        }
        return true;
    };
    const auto candidateFor = [&directory, &stem, &suffix](int index) {
        const QString numberedStem = index == 0 ? stem : QStringLiteral("%1 (%2)").arg(stem).arg(index);
        const QString candidateName = suffix.isEmpty() ? numberedStem : QStringLiteral("%1.%2").arg(numberedStem, suffix);
        return directory.filePath(candidateName);
    };

    for (int index = 0; index < 10000; ++index)
    {
        const QString candidate = candidateFor(index);
        if (isAvailable(candidate))
        {
            if (index > 0)
            {
                QLOG_DEBUG() << QStringLiteral("[网络.文件] 为避免名称冲突已重命名接收文件 文件名=") << fileName
                                       << QStringLiteral("序号=") << index;
            }
            return candidate;
        }
    }
    QLOG_WARN() << QStringLiteral("[网络.文件] 接收文件名称冲突次数已达上限 文件名=") << fileName;
    return directory.filePath(QStringLiteral("%1-%2%3").arg(
        stem, QUuid::createUuid().toString(QUuid::WithoutBraces), suffix.isEmpty() ? QString() : QStringLiteral(".%1").arg(suffix)));
}
