#include "tcpchattransport.h"

#include "wireprotocol.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QtEndian>

#include <spdlog/spdlog.h>

#include <utility>

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
        spdlog::debug("[网络.传输] TCP 传输服务已在运行，忽略启动请求");
        return true;
    }
    if (identity.deviceId.isEmpty() || identity.displayName.isEmpty())
    {
        spdlog::error("[网络.传输] TCP 传输服务身份信息无效");
        setLastError(tr("TCP 服务身份信息无效。"));
        return false;
    }
    if (!m_server.listen(QHostAddress::AnyIPv4))
    {
        spdlog::error("[网络.传输] TCP 监听失败 原因={}",
                      m_server.errorString().toUtf8().toStdString());
        setLastError(tr("无法启动 TCP 服务：%1").arg(m_server.errorString()));
        return false;
    }

    m_identity = identity;
    m_running = true;
    setLastError({});
    m_transferTimer.start();
    spdlog::info("[网络.传输] TCP 传输服务已启动 端口={}",
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
        "[网络.传输] 正在停止 TCP 传输服务 文本传输数={} 发送文件数={} 接收文件数={}",
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
    spdlog::info("[网络.传输] TCP 传输服务已停止");
}

void TcpChatTransport::updateIdentity(const Network::LocalIdentity &identity)
{
    m_identity = identity;
    spdlog::info("[网络.传输] 本机身份信息已更新");
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
                                const QString &groupId,
                                const QString &text,
                                const QDateTime &timestamp)
{
    spdlog::debug(
        "[网络.传输] 正在连接以发送文本消息 好友标识={} 消息标识={} 地址={} 端口={} 长度={}",
        peer.peerId.toUtf8().toStdString(),
        messageId.toUtf8().toStdString(),
        peer.address.toString().toStdString(),
        peer.tcpPort,
        text.size());
    sendFramedEvent(peer,
                    messageId,
                    QStringLiteral("message"),
                    Network::WireProtocol::textFrame(m_identity,
                                                     listeningPort(),
                                                     peer,
                                                     messageId,
                                                     groupId,
                                                     text,
                                                     timestamp));
}

void TcpChatTransport::sendGroupSnapshot(
    const Network::PeerEndpoint &peer,
    const Network::GroupSnapshot &snapshot)
{
    sendFramedEvent(peer,
                    snapshot.groupId,
                    QStringLiteral("group.snapshot"),
                    Network::WireProtocol::groupSnapshotFrame(m_identity,
                                                              listeningPort(),
                                                              peer,
                                                              snapshot));
}

void TcpChatTransport::sendFramedEvent(const Network::PeerEndpoint &peer,
                                       const QString &eventId,
                                       const QString &frameType,
                                       const QByteArray &frame)
{
    auto *socket = new QTcpSocket(this);
    socket->setProperty("peerId", peer.peerId);
    socket->setProperty("messageId", eventId);
    socket->setProperty("frameType", frameType);
    socket->setProperty("frame", frame);
    m_outgoingTextSockets.insert(socket);

    connect(socket, &QTcpSocket::connected, this, [this, socket]() {
        spdlog::trace("[网络.传输] 文本消息套接字已连接 好友标识={} 消息标识={}",
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
        spdlog::debug("[网络.传输] 已接受 TCP 连接 地址={} 端口={}",
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
            spdlog::warn("[网络.传输] 已拒绝无效长度的数据帧 长度={} 地址={}",
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
            spdlog::debug("[网络.传输] 已忽略格式错误的 JSON 数据帧 地址={}",
                          socket->peerAddress().toString().toStdString());
            continue;
        }

        const QJsonObject object = document.object();
        const QString type = object.value(QStringLiteral("type")).toString();
        if (type == QLatin1String("message"))
        {
            handleIncomingText(object, socket);
        }
        else if (type == QLatin1String("group.snapshot"))
        {
            handleIncomingGroupSnapshot(object, socket);
        }
        else if (type == QLatin1String("file")
                 && !handleIncomingFileHeader(object, socket))
        {
            spdlog::warn("[网络.传输] 已拒绝接收文件头 地址={}",
                         socket->peerAddress().toString().toStdString());
            socket->disconnectFromHost();
            return;
        }
        else if (type != QLatin1String("file"))
        {
            spdlog::debug("[网络.传输] 已忽略未知类型的数据帧 类型={} 地址={}",
                          type.toUtf8().toStdString(),
                          socket->peerAddress().toString().toStdString());
        }
    }
}

void TcpChatTransport::handleIncomingText(const QJsonObject &object, QTcpSocket *socket)
{
    if (!Network::WireProtocol::isEnvelopeFor(object, m_identity))
    {
        spdlog::debug("[网络.传输] 已忽略发给其他接收方的文本消息信封 地址={}",
                      socket->peerAddress().toString().toStdString());
        return;
    }
    const Network::PeerEndpoint peer = incomingPeer(object, socket);
    const QString messageId = object.value(QStringLiteral("messageId")).toString();
    const QString groupId = object.value(QStringLiteral("groupId")).toString();
    const QString text = object.value(QStringLiteral("text")).toString();
    if (!peer.isValid() || peer.peerId == m_identity.deviceId || messageId.isEmpty()
        || groupId.size() > 128
        || (!groupId.isEmpty() && !groupId.startsWith(QLatin1String("group:")))
        || text.isEmpty() || text.size() > 2000
        || !rememberEventId(messageId))
    {
        spdlog::debug("[网络.传输] 已拒绝无效或重复的文本消息 地址={}",
                      socket->peerAddress().toString().toStdString());
        return;
    }

    spdlog::debug(
        "[网络.传输] 已接收文本消息 好友标识={} 消息标识={} 长度={}",
        peer.peerId.toUtf8().toStdString(),
        messageId.toUtf8().toStdString(),
        text.size());
    emit peerObserved(peer);
    emit textReceived({messageId, groupId, peer, text, timestampFrom(object)});
}

void TcpChatTransport::handleIncomingGroupSnapshot(const QJsonObject &object,
                                                   QTcpSocket *socket)
{
    if (!Network::WireProtocol::isEnvelopeFor(object, m_identity))
    {
        return;
    }

    Network::GroupSnapshot snapshot;
    snapshot.sender = incomingPeer(object, socket);
    snapshot.groupId = object.value(QStringLiteral("groupId")).toString();
    snapshot.name = object.value(QStringLiteral("name")).toString().trimmed().left(64);
    snapshot.ownerId = object.value(QStringLiteral("ownerId")).toString();
    snapshot.revision = object.value(QStringLiteral("revision")).toVariant().toULongLong();
    snapshot.createdAt = QDateTime::fromString(
        object.value(QStringLiteral("createdAt")).toString(),
        Qt::ISODateWithMs);

    const QJsonArray members = object.value(QStringLiteral("members")).toArray();
    if (!snapshot.sender.isValid()
        || !snapshot.groupId.startsWith(QLatin1String("group:"))
        || snapshot.groupId.size() > 128 || snapshot.name.isEmpty()
        || snapshot.ownerId != snapshot.sender.peerId || snapshot.revision == 0
        || members.size() < 2 || members.size() > 32)
    {
        return;
    }

    bool includesLocalUser = false;
    bool includesOwner = false;
    QSet<QString> memberIds;
    for (const QJsonValue &value : members)
    {
        const QJsonObject memberObject = value.toObject();
        Network::GroupMemberInfo member;
        member.peerId = memberObject.value(QStringLiteral("peerId")).toString();
        member.displayName = memberObject.value(QStringLiteral("displayName"))
                                 .toString()
                                 .trimmed()
                                 .left(64);
        member.owner = memberObject.value(QStringLiteral("owner")).toBool();
        if (member.peerId.isEmpty() || member.displayName.isEmpty()
            || memberIds.contains(member.peerId)
            || (member.owner && member.peerId != snapshot.ownerId))
        {
            return;
        }
        memberIds.insert(member.peerId);
        includesLocalUser = includesLocalUser || member.peerId == m_identity.deviceId;
        includesOwner = includesOwner
                        || (member.peerId == snapshot.ownerId && member.owner);
        snapshot.members.append(std::move(member));
    }
    if (!includesLocalUser || !includesOwner
        || !rememberEventId(QStringLiteral("%1:%2")
                                .arg(snapshot.groupId)
                                .arg(snapshot.revision)))
    {
        return;
    }

    if (!snapshot.createdAt.isValid())
    {
        snapshot.createdAt = QDateTime::currentDateTimeUtc();
    }
    emit peerObserved(snapshot.sender);
    emit groupSnapshotReceived(snapshot);
}

void TcpChatTransport::finishOutgoingText(QTcpSocket *socket)
{
    if (!m_outgoingTextSockets.remove(socket))
    {
        return;
    }
    spdlog::debug("[网络.传输] 文本消息已发送 好友标识={} 消息标识={}",
                  socket->property("peerId").toString().toUtf8().toStdString(),
                  socket->property("messageId").toString().toUtf8().toStdString());
    if (socket->property("frameType").toString() == QLatin1String("message"))
    {
        emit textSent(socket->property("peerId").toString(),
                      socket->property("messageId").toString());
    }
    socket->disconnectFromHost();
}

void TcpChatTransport::failOutgoingText(QTcpSocket *socket, const QString &reason)
{
    if (!m_outgoingTextSockets.remove(socket))
    {
        return;
    }
    spdlog::warn("[网络.传输] 文本消息发送失败 好友标识={} 消息标识={} 原因={}",
                 socket->property("peerId").toString().toUtf8().toStdString(),
                 socket->property("messageId").toString().toUtf8().toStdString(),
                 reason.toUtf8().toStdString());
    if (socket->property("frameType").toString() == QLatin1String("group.snapshot"))
    {
        emit groupSnapshotSendFailed(socket->property("peerId").toString(),
                                     socket->property("messageId").toString(),
                                     reason);
    }
    else
    {
        emit textSendFailed(socket->property("peerId").toString(),
                            socket->property("messageId").toString(),
                            reason);
    }
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
        spdlog::trace("[网络.传输] 已忽略重复事件 事件标识={}",
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
        spdlog::error("[网络.传输] 服务错误 原因={}",
                      error.toUtf8().toStdString());
        emit errorOccurred(error);
    }
}
