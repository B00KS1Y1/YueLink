#include "tcpchattransport.h"

#include "config/configapi.h"
#include "infrastructure/path.h"
#include "wireprotocol.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QSaveFile>
#include <QTcpSocket>
#include <QUuid>

#include <spdlog/spdlog.h>

namespace
{
constexpr qint64 FileChunkSize = 64 * 1024;
constexpr qint64 MaximumPendingBytes = 256 * 1024;
constexpr qreal ProgressStep = 0.01;
} // namespace

struct TcpChatTransport::OutgoingFileTransfer
{
    Network::FileTransferInfo info;
    QFile file;
    qint64 headerSize = 0;
    qint64 socketBytesWritten = 0;
    qint64 lastActivityMs = 0;
    qreal reportedProgress = 0.0;
};

struct TcpChatTransport::IncomingFileTransfer
{
    Network::FileTransferInfo info;
    QSaveFile file;
    qint64 receivedBytes = 0;
    qint64 lastActivityMs = 0;
    qreal reportedProgress = 0.0;
};

bool TcpChatTransport::sendFile(const Network::PeerEndpoint &peer, const QUrl &fileUrl, QString *errorMessage)
{
    const auto fail = [errorMessage](const QString &message) {
        spdlog::warn("[网络.文件] 已拒绝文件发送请求 原因={}", message.toUtf8().toStdString());
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
    if (!fileUrl.isLocalFile())
    {
        return fail(tr("只能发送本地文件。"));
    }

    const QFileInfo fileInfo(fileUrl.toLocalFile());
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        return fail(tr("要发送的文件不存在。"));
    }
    if (fileInfo.size() < 0 || fileInfo.size() > Network::WireProtocol::MaximumFileSize)
    {
        return fail(tr("文件大小不能超过 2 GB。"));
    }

    auto *transfer = new OutgoingFileTransfer;
    transfer->info.transferId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    transfer->info.peer = peer;
    transfer->info.fileName = fileInfo.fileName();
    transfer->info.filePath = fileInfo.absoluteFilePath();
    transfer->info.fileSize = fileInfo.size();
    transfer->info.timestamp = QDateTime::currentDateTimeUtc();
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
    m_outgoingFiles.insert(socket, transfer);
    spdlog::info("[网络.文件] 文件发送已开始 好友标识={} 传输标识={} 文件名={} 大小={}",
                 peer.peerId.toUtf8().toStdString(),
                 transfer->info.transferId.toUtf8().toStdString(),
                 transfer->info.fileName.toUtf8().toStdString(),
                 transfer->info.fileSize);

    connect(socket, &QTcpSocket::connected, this, [this, socket]() {
        OutgoingFileTransfer *transfer = m_outgoingFiles.value(socket);
        if (!transfer)
        {
            return;
        }
        spdlog::debug("[网络.文件] 文件发送连接已建立 好友标识={} 传输标识={}",
                      transfer->info.peer.peerId.toUtf8().toStdString(),
                      transfer->info.transferId.toUtf8().toStdString());

        const QByteArray header = Network::WireProtocol::fileHeaderFrame(m_identity,
                                                                         listeningPort(),
                                                                         transfer->info.peer,
                                                                         transfer->info.transferId,
                                                                         transfer->info.fileName,
                                                                         transfer->info.fileSize,
                                                                         transfer->info.timestamp);
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
            spdlog::trace("[网络.文件] 文件发送进度 好友标识={} 传输标识={} 进度={:.0f}%",
                          transfer->info.peer.peerId.toUtf8().toStdString(),
                          transfer->info.transferId.toUtf8().toStdString(),
                          progress * 100.0);
            emit fileTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Outgoing, progress});
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

    emit fileTransferStarted(transfer->info);
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

bool TcpChatTransport::handleIncomingFileHeader(const QJsonObject &object, QTcpSocket *socket)
{
    if (!Network::WireProtocol::isEnvelopeFor(object, m_identity))
    {
        spdlog::debug("[网络.文件] 已忽略发给其他接收方的文件信封 地址={}", socket->peerAddress().toString().toStdString());
        return false;
    }

    const Network::PeerEndpoint peer = incomingPeer(object, socket);
    const QString transferId = object.value(QStringLiteral("transferId")).toString().trimmed();
    QString fileName = object.value(QStringLiteral("fileName")).toString().trimmed();
    fileName.replace(QLatin1Char('\\'), QLatin1Char('/'));
    fileName = fileName.section(QLatin1Char('/'), -1);
    const qint64 fileSize = object.value(QStringLiteral("fileSize")).toInteger(-1);
    if (!peer.isValid() || peer.peerId == m_identity.deviceId || transferId.isEmpty() || transferId.size() > 128 || fileName.isEmpty() ||
        fileName == QLatin1String(".") || fileName == QLatin1String("..") || fileName.size() > 255 || fileName.contains(QChar::Null) || fileSize < 0 ||
        fileSize > Network::WireProtocol::MaximumFileSize || !rememberEventId(transferId))
    {
        spdlog::warn("[网络.文件] 接收文件头无效或重复 地址={} 传输标识={} 文件大小={}",
                     socket->peerAddress().toString().toStdString(),
                     transferId.toUtf8().toStdString(),
                     fileSize);
        return false;
    }

    const QString receivePath = uniqueReceivePath(fileName);
    if (receivePath.isEmpty())
    {
        spdlog::error("[网络.文件] 创建文件接收目录失败 传输标识={}", transferId.toUtf8().toStdString());
        setLastError(tr("无法创建文件接收目录。"));
        return false;
    }

    auto *transfer = new IncomingFileTransfer;
    transfer->info.transferId = transferId;
    transfer->info.peer = peer;
    transfer->info.fileName = fileName;
    transfer->info.filePath = receivePath;
    transfer->info.fileSize = fileSize;
    transfer->info.timestamp = timestampFrom(object);
    transfer->info.direction = Network::TransferDirection::Incoming;
    transfer->file.setFileName(receivePath);
    if (!transfer->file.open(QIODevice::WriteOnly))
    {
        const QString reason = transfer->file.errorString();
        spdlog::error("[网络.文件] 打开接收文件失败 传输标识={} 原因={}", transferId.toUtf8().toStdString(), reason.toUtf8().toStdString());
        delete transfer;
        setLastError(tr("无法接收文件：%1").arg(reason));
        return false;
    }
    transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();
    m_incomingFiles.insert(socket, transfer);

    spdlog::info("[网络.文件] 文件接收已开始 好友标识={} 传输标识={} 文件名={} 大小={}",
                 peer.peerId.toUtf8().toStdString(),
                 transferId.toUtf8().toStdString(),
                 fileName.toUtf8().toStdString(),
                 fileSize);
    emit peerObserved(peer);
    emit fileTransferStarted(transfer->info);
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
        const qint64 writtenBytes = transfer->file.write(buffer.constData(), consumedBytes);
        if (writtenBytes != consumedBytes)
        {
            const QString reason = transfer->file.errorString();
            failIncomingFile(socket, tr("写入接收文件失败：%1").arg(reason));
            return false;
        }
        buffer.remove(0, consumedBytes);
        transfer->receivedBytes += writtenBytes;
        transfer->lastActivityMs = QDateTime::currentMSecsSinceEpoch();

        const qreal progress = transfer->info.fileSize == 0 ? 1.0 : static_cast<qreal>(transfer->receivedBytes) / static_cast<qreal>(transfer->info.fileSize);
        if (progress >= 1.0 || progress - transfer->reportedProgress >= ProgressStep)
        {
            transfer->reportedProgress = progress;
            spdlog::trace("[网络.文件] 文件接收进度 好友标识={} 传输标识={} 进度={:.0f}%",
                          transfer->info.peer.peerId.toUtf8().toStdString(),
                          transfer->info.transferId.toUtf8().toStdString(),
                          progress * 100.0);
            emit fileTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Incoming, progress});
        }
    }

    if (transfer->receivedBytes < transfer->info.fileSize)
    {
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
        emit fileTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Incoming, 1.0});
    }
    spdlog::info("[网络.文件] 文件接收已完成 好友标识={} 传输标识={} 文件名={} 大小={}",
                 transfer->info.peer.peerId.toUtf8().toStdString(),
                 transfer->info.transferId.toUtf8().toStdString(),
                 transfer->info.fileName.toUtf8().toStdString(),
                 transfer->info.fileSize);
    emit fileTransferFinished({transfer->info.peer.peerId, transfer->info.transferId, transfer->info.filePath, {}, Network::TransferDirection::Incoming, true});
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
        spdlog::info("[网络.文件] 文件接收已取消 好友标识={} 传输标识={} 文件名={}",
                     transfer->info.peer.peerId.toUtf8().toStdString(),
                     transfer->info.transferId.toUtf8().toStdString(),
                     transfer->info.fileName.toUtf8().toStdString());
    }
    else
    {
        spdlog::warn("[网络.文件] 文件接收失败 好友标识={} 传输标识={} 文件名={} 原因={}",
                     transfer->info.peer.peerId.toUtf8().toStdString(),
                     transfer->info.transferId.toUtf8().toStdString(),
                     transfer->info.fileName.toUtf8().toStdString(),
                     reason.toUtf8().toStdString());
    }
    transfer->file.cancelWriting();
    emit fileTransferFinished(
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
        emit fileTransferProgressed({transfer->info.peer.peerId, transfer->info.transferId, Network::TransferDirection::Outgoing, 1.0});
    }
    spdlog::info("[网络.文件] 文件发送已完成 好友标识={} 传输标识={} 文件名={} 大小={}",
                 transfer->info.peer.peerId.toUtf8().toStdString(),
                 transfer->info.transferId.toUtf8().toStdString(),
                 transfer->info.fileName.toUtf8().toStdString(),
                 transfer->info.fileSize);
    emit fileTransferFinished({transfer->info.peer.peerId, transfer->info.transferId, transfer->info.filePath, {}, Network::TransferDirection::Outgoing, true});
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
        spdlog::info("[网络.文件] 文件发送已取消 好友标识={} 传输标识={} 文件名={}",
                     transfer->info.peer.peerId.toUtf8().toStdString(),
                     transfer->info.transferId.toUtf8().toStdString(),
                     transfer->info.fileName.toUtf8().toStdString());
    }
    else
    {
        spdlog::warn("[网络.文件] 文件发送失败 好友标识={} 传输标识={} 文件名={} 原因={}",
                     transfer->info.peer.peerId.toUtf8().toStdString(),
                     transfer->info.transferId.toUtf8().toStdString(),
                     transfer->info.fileName.toUtf8().toStdString(),
                     reason.toUtf8().toStdString());
    }
    transfer->file.close();
    emit fileTransferFinished(
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
        spdlog::error("[网络.文件] 创建文件接收目录失败");
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
                spdlog::debug("[网络.文件] 为避免名称冲突已重命名接收文件 文件名={} 序号={}", fileName.toUtf8().toStdString(), index);
            }
            return candidate;
        }
    }
    spdlog::warn("[网络.文件] 接收文件名称冲突次数已达上限 文件名={}", fileName.toUtf8().toStdString());
    return directory.filePath(QStringLiteral("%1-%2%3").arg(
        stem, QUuid::createUuid().toString(QUuid::WithoutBraces), suffix.isEmpty() ? QString() : QStringLiteral(".%1").arg(suffix)));
}
