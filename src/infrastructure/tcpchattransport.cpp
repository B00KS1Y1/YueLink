#include "tcpchattransport.h"

#include "wireprotocol.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QtEndian>

#include <spdlog/spdlog.h>

TcpChatTransport::TcpChatTransport(QObject *parent)
: IChatTransport(parent)
{
    connect(&m_server,
            &QTcpServer::newConnection,
            this,
            &TcpChatTransport::acceptConnections);
    m_transferTimer.setInterval(2000);
    m_transferTimer.setTimerType(Qt::CoarseTimer);
    connect(&m_transferTimer,
            &QTimer::timeout,
            this,
            &TcpChatTransport::expireFileTransfers);
}

TcpChatTransport::~TcpChatTransport()
{
    stop();
}

bool TcpChatTransport::start(const Network::LocalIdentity &identity)
{
    if (m_running)
    {
        spdlog::debug("[network.transport] start ignored because TCP transport is already running");
        return true;
    }
    if (identity.deviceId.isEmpty() || identity.displayName.isEmpty())
    {
        spdlog::error("[network.transport] invalid TCP transport identity");
        setLastError(tr("TCP 服务身份信息无效。"));
        return false;
    }
    if (!m_server.listen(QHostAddress::AnyIPv4))
    {
        spdlog::error("[network.transport] TCP listen failed reason={}",
                      m_server.errorString().toUtf8().toStdString());
        setLastError(tr("无法启动 TCP 服务：%1").arg(m_server.errorString()));
        return false;
    }

    m_identity = identity;
    m_running = true;
    setLastError({});
    m_transferTimer.start();
    spdlog::info("[network.transport] TCP transport started port={}",
                 m_server.serverPort());
    return true;
}

void TcpChatTransport::stop()
{
    if (!m_running)
    {
        return;
    }

    spdlog::info(
        "[network.transport] stopping TCP transport text_transfers={} outgoing_files={} incoming_files={}",
        m_outgoingTextSockets.size(),
        m_outgoingFiles.size(),
        m_incomingFiles.size());
    m_transferTimer.stop();
    m_server.close();
    for (QTcpSocket *socket : m_outgoingTextSockets.values())
    {
        failOutgoingText(socket, tr("消息发送已取消。"));
    }
    for (QTcpSocket *socket : m_outgoingFiles.keys())
    {
        failOutgoingFile(socket, tr("文件传输已取消。"), true);
    }
    for (QTcpSocket *socket : m_incomingBuffers.keys())
    {
        if (m_incomingFiles.contains(socket))
        {
            failIncomingFile(socket, tr("文件接收已取消。"), true);
        }
        socket->abort();
    }
    m_incomingBuffers.clear();
    m_identity = Network::LocalIdentity{};
    m_running = false;
    spdlog::info("[network.transport] TCP transport stopped");
}

void TcpChatTransport::updateIdentity(const Network::LocalIdentity &identity)
{
    m_identity = identity;
    spdlog::info("[network.transport] local identity updated");
}

bool TcpChatTransport::isRunning() const
{
    return m_running;
}

quint16 TcpChatTransport::listeningPort() const
{
    return m_server.serverPort();
}

QString TcpChatTransport::lastError() const
{
    return m_lastError;
}

void TcpChatTransport::sendText(const Network::PeerEndpoint &peer,
                                const QString &messageId,
                                const QString &text,
                                const QDateTime &timestamp)
{
    spdlog::debug(
        "[network.transport] connecting for text message peer_id={} message_id={} address={} port={} length={}",
        peer.peerId.toUtf8().toStdString(),
        messageId.toUtf8().toStdString(),
        peer.address.toString().toStdString(),
        peer.tcpPort,
        text.size());
    auto *socket = new QTcpSocket(this);
    socket->setProperty("peerId", peer.peerId);
    socket->setProperty("messageId", messageId);
    socket->setProperty("frame",
                        Network::WireProtocol::textFrame(m_identity,
                                                         listeningPort(),
                                                         peer,
                                                         messageId,
                                                         text,
                                                         timestamp));
    m_outgoingTextSockets.insert(socket);

    connect(socket, &QTcpSocket::connected, this, [this, socket]() {
        spdlog::trace("[network.transport] text socket connected peer_id={} message_id={}",
                      socket->property("peerId").toString().toUtf8().toStdString(),
                      socket->property("messageId").toString().toUtf8().toStdString());
        const QByteArray frame = socket->property("frame").toByteArray();
        if (socket->write(frame) != frame.size())
        {
            failOutgoingText(socket, socket->errorString());
        }
        else if (socket->bytesToWrite() == 0)
        {
            finishOutgoingText(socket);
        }
    });
    connect(socket, &QTcpSocket::bytesWritten, this, [this, socket](qint64) {
        if (socket->bytesToWrite() == 0)
        {
            finishOutgoingText(socket);
        }
    });
    connect(socket,
            &QTcpSocket::errorOccurred,
            this,
            [this, socket](QAbstractSocket::SocketError) {
                failOutgoingText(socket, socket->errorString());
            });
    connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
        if (m_outgoingTextSockets.contains(socket))
        {
            failOutgoingText(socket, tr("连接已中断。"));
        }
        socket->deleteLater();
    });
    QTimer::singleShot(5000, socket, [this, socket]() {
        if (m_outgoingTextSockets.contains(socket))
        {
            failOutgoingText(socket, tr("连接超时。"));
        }
    });
    socket->connectToHost(peer.address, peer.tcpPort);
}

void TcpChatTransport::acceptConnections()
{
    while (m_server.hasPendingConnections())
    {
        QTcpSocket *socket = m_server.nextPendingConnection();
        if (!socket)
        {
            continue;
        }
        spdlog::debug("[network.transport] accepted TCP connection address={} port={}",
                      socket->peerAddress().toString().toStdString(),
                      socket->peerPort());
        socket->setReadBufferSize(Network::WireProtocol::MaximumFrameSize + 4);
        socket->setProperty("receivedData", false);
        m_incomingBuffers.insert(socket, QByteArray());
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            socket->setProperty("receivedData", true);
            readIncomingData(socket);
        });
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            if (m_incomingFiles.contains(socket))
            {
                failIncomingFile(socket, tr("文件接收连接已中断。"));
            }
            m_incomingBuffers.remove(socket);
            socket->deleteLater();
        });
        QTimer::singleShot(10000, socket, [socket]() {
            if (!socket->property("receivedData").toBool()
                && socket->state() != QAbstractSocket::UnconnectedState)
            {
                socket->disconnectFromHost();
            }
        });
    }
}

void TcpChatTransport::readIncomingData(QTcpSocket *socket)
{
    QByteArray &buffer = m_incomingBuffers[socket];
    buffer.append(socket->readAll());
    while (true)
    {
        if (m_incomingFiles.contains(socket))
        {
            if (!consumeIncomingFile(socket, buffer))
            {
                if (!m_incomingFiles.contains(socket))
                {
                    socket->disconnectFromHost();
                }
                return;
            }
            continue;
        }
        if (buffer.size() < 4)
        {
            return;
        }

        const quint32 frameSize = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar *>(buffer.constData()));
        if (frameSize == 0 || frameSize > Network::WireProtocol::MaximumFrameSize)
        {
            spdlog::warn("[network.transport] rejected invalid frame size={} address={}",
                         frameSize,
                         socket->peerAddress().toString().toStdString());
            socket->disconnectFromHost();
            return;
        }
        if (buffer.size() < static_cast<qsizetype>(4 + frameSize))
        {
            return;
        }

        const QByteArray payload = buffer.mid(4, frameSize);
        buffer.remove(0, 4 + frameSize);
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            spdlog::debug("[network.transport] ignored malformed JSON frame address={}",
                          socket->peerAddress().toString().toStdString());
            continue;
        }

        const QJsonObject object = document.object();
        const QString type = object.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("message"))
        {
            handleIncomingText(object, socket);
        }
        else if (type == QLatin1String("file")
                 && !handleIncomingFileHeader(object, socket))
        {
            spdlog::warn("[network.transport] rejected incoming file header address={}",
                         socket->peerAddress().toString().toStdString());
            socket->disconnectFromHost();
            return;
        }
        else if (type != QLatin1String("file"))
        {
            spdlog::debug("[network.transport] ignored unknown frame type={} address={}",
                          type.toUtf8().toStdString(),
                          socket->peerAddress().toString().toStdString());
        }
    }
}

void TcpChatTransport::handleIncomingText(const QJsonObject &object, QTcpSocket *socket)
{
    if (!Network::WireProtocol::isEnvelopeFor(object, m_identity))
    {
        spdlog::debug("[network.transport] ignored text envelope for another recipient address={}",
                      socket->peerAddress().toString().toStdString());
        return;
    }
    const Network::PeerEndpoint peer = incomingPeer(object, socket);
    const QString messageId = object.value(QStringLiteral("messageId")).toString();
    const QString text = object.value(QStringLiteral("text")).toString();
    if (!peer.isValid() || peer.peerId == m_identity.deviceId || messageId.isEmpty()
        || text.isEmpty() || text.size() > 2000 || !rememberEventId(messageId))
    {
        spdlog::debug("[network.transport] rejected invalid or duplicate text message address={}",
                      socket->peerAddress().toString().toStdString());
        return;
    }

    spdlog::debug(
        "[network.transport] accepted incoming text message peer_id={} message_id={} length={}",
        peer.peerId.toUtf8().toStdString(),
        messageId.toUtf8().toStdString(),
        text.size());
    emit peerObserved(peer);
    emit textReceived({messageId, peer, text, timestampFrom(object)});
}

void TcpChatTransport::finishOutgoingText(QTcpSocket *socket)
{
    if (!m_outgoingTextSockets.remove(socket))
    {
        return;
    }
    spdlog::debug("[network.transport] text message sent peer_id={} message_id={}",
                  socket->property("peerId").toString().toUtf8().toStdString(),
                  socket->property("messageId").toString().toUtf8().toStdString());
    emit textSent(socket->property("peerId").toString(),
                  socket->property("messageId").toString());
    socket->disconnectFromHost();
}

void TcpChatTransport::failOutgoingText(QTcpSocket *socket, const QString &reason)
{
    if (!m_outgoingTextSockets.remove(socket))
    {
        return;
    }
    spdlog::warn("[network.transport] text message send failed peer_id={} message_id={} reason={}",
                 socket->property("peerId").toString().toUtf8().toStdString(),
                 socket->property("messageId").toString().toUtf8().toStdString(),
                 reason.toUtf8().toStdString());
    emit textSendFailed(socket->property("peerId").toString(),
                        socket->property("messageId").toString(),
                        reason);
    socket->abort();
    socket->deleteLater();
}

Network::PeerEndpoint TcpChatTransport::incomingPeer(const QJsonObject &object,
                                                     QTcpSocket *socket) const
{
    return Network::WireProtocol::senderFromEnvelope(object, socket->peerAddress());
}

bool TcpChatTransport::rememberEventId(const QString &eventId)
{
    if (m_receivedEventIds.contains(eventId))
    {
        spdlog::trace("[network.transport] duplicate event ignored event_id={}",
                      eventId.toUtf8().toStdString());
        return false;
    }
    m_receivedEventIds.insert(eventId);
    m_receivedEventOrder.enqueue(eventId);
    while (m_receivedEventOrder.size() > 1024)
    {
        m_receivedEventIds.remove(m_receivedEventOrder.dequeue());
    }
    return true;
}

QDateTime TcpChatTransport::timestampFrom(const QJsonObject &object)
{
    QDateTime timestamp = QDateTime::fromString(
        object.value(QStringLiteral("timestamp")).toString(),
        Qt::ISODateWithMs);
    return timestamp.isValid() ? timestamp : QDateTime::currentDateTimeUtc();
}

void TcpChatTransport::setLastError(const QString &error)
{
    if (m_lastError == error)
    {
        return;
    }
    m_lastError = error;
    if (!error.isEmpty())
    {
        spdlog::error("[network.transport] service error: {}",
                      error.toUtf8().toStdString());
        emit errorOccurred(error);
    }
}
