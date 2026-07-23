#include "application/chatservice.h"
#include "application/iidentitystore.h"
#include "cli/commandparser.h"
#include "discovery/ipeerdiscovery.h"
#include "storage/ichatrepository.h"
#include "transport/ichattransport.h"

#include <QHostAddress>
#include <QHash>
#include <QSet>
#include <QtTest>

#include <memory>

namespace
{
class FakeDiscovery final : public IPeerDiscovery
{
public:
    using IPeerDiscovery::IPeerDiscovery;

    bool start(const Network::LocalIdentity &, quint16) override
    {
        running = startAllowed;
        return startAllowed;
    }

    void stop() override
    {
        running = false;
        stopCalled = true;
    }

    void updateIdentity(const Network::LocalIdentity &) override {}
    void announce() override {}
    void recordPeerActivity(const QString &) override {}
    [[nodiscard]] bool isRunning() const override { return running; }
    [[nodiscard]] QString lastError() const override
    {
        return startAllowed ? QString() : QStringLiteral("discovery failed");
    }

    bool startAllowed = true;
    bool running = false;
    bool stopCalled = false;
};

class FakeTransport final : public IChatTransport
{
public:
    using IChatTransport::IChatTransport;

    bool start(const Network::LocalIdentity &) override
    {
        running = startAllowed;
        return startAllowed;
    }

    void stop() override
    {
        running = false;
        stopCalled = true;
    }

    void updateIdentity(const Network::LocalIdentity &) override {}
    [[nodiscard]] bool isRunning() const override { return running; }
    [[nodiscard]] quint16 listeningPort() const override { return 48101; }
    [[nodiscard]] QString lastError() const override
    {
        return startAllowed ? QString() : QStringLiteral("transport failed");
    }

    void sendText(const Network::PeerEndpoint &peer,
                  const QString &messageId,
                  const QString &text,
                  const QDateTime &) override
    {
        lastPeer = peer;
        lastMessageId = messageId;
        lastText = text;
    }

    bool sendFile(const Network::PeerEndpoint &,
                  const QUrl &,
                  QString *errorMessage) override
    {
        if (errorMessage)
        {
            errorMessage->clear();
        }
        return true;
    }

    bool cancelFileTransfer(const QString &, const QString &) override
    {
        return true;
    }

    bool startAllowed = true;
    bool running = false;
    bool stopCalled = false;
    Network::PeerEndpoint lastPeer;
    QString lastMessageId;
    QString lastText;
};

class FakeRepository final : public IChatRepository
{
public:
    bool initialize(QString *errorMessage) override
    {
        clearError(errorMessage);
        return true;
    }

    bool loadPeers(QList<Storage::PeerRecord> *output,
                   QString *errorMessage) override
    {
        *output = peers;
        clearError(errorMessage);
        return true;
    }

    bool loadMessages(const QString &peerId,
                      int,
                      QList<Storage::MessageRecord> *output,
                      QString *errorMessage) override
    {
        *output = messages.value(peerId);
        clearError(errorMessage);
        return true;
    }

    bool upsertPeer(const Network::PeerEndpoint &peer,
                    QString *errorMessage) override
    {
        if (!peerIds.contains(peer.peerId))
        {
            Storage::PeerRecord record;
            record.endpoint = peer;
            peers.append(record);
            peerIds.insert(peer.peerId);
        }
        clearError(errorMessage);
        return true;
    }

    bool updateConversation(const QString &peerId,
                            const QString &lastMessage,
                            const QDateTime &timestamp,
                            bool incrementUnread,
                            QString *errorMessage) override
    {
        for (Storage::PeerRecord &peer : peers)
        {
            if (peer.endpoint.peerId == peerId)
            {
                peer.lastMessage = lastMessage;
                peer.lastActivity = timestamp;
                peer.unreadCount += incrementUnread ? 1 : 0;
            }
        }
        clearError(errorMessage);
        return true;
    }

    bool clearUnread(const QString &peerId, QString *errorMessage) override
    {
        for (Storage::PeerRecord &peer : peers)
        {
            if (peer.endpoint.peerId == peerId)
            {
                peer.unreadCount = 0;
            }
        }
        clearError(errorMessage);
        return true;
    }

    bool saveMessage(const Storage::MessageRecord &message,
                     QString *errorMessage) override
    {
        messages[message.peerId].append(message);
        clearError(errorMessage);
        return true;
    }

    bool updateDeliveryStatus(const QString &,
                              const QString &,
                              const QString &,
                              QString *errorMessage) override
    {
        clearError(errorMessage);
        return true;
    }

    bool updateFileTransfer(const QString &,
                            const QString &,
                            qreal,
                            const QString &,
                            const QString &,
                            QString *errorMessage) override
    {
        clearError(errorMessage);
        return true;
    }

private:
    static void clearError(QString *errorMessage)
    {
        if (errorMessage)
        {
            errorMessage->clear();
        }
    }

    QList<Storage::PeerRecord> peers;
    QHash<QString, QList<Storage::MessageRecord>> messages;
    QSet<QString> peerIds;
};

class FakeIdentityStore final : public IIdentityStore
{
public:
    bool load(Network::LocalIdentity *identity,
              QString *errorMessage) override
    {
        *identity = {QStringLiteral("local-id"), QStringLiteral("Local")};
        if (errorMessage)
        {
            errorMessage->clear();
        }
        return true;
    }

    bool save(const Network::LocalIdentity &identity,
              QString *errorMessage) override
    {
        saved = identity;
        if (errorMessage)
        {
            errorMessage->clear();
        }
        return true;
    }

    Network::LocalIdentity saved;
};

Network::PeerEndpoint testPeer()
{
    return {QStringLiteral("peer-id"),
            QStringLiteral("Peer"),
            QHostAddress(QHostAddress::LocalHost),
            48102};
}
} // namespace

class ChatServiceTest final : public QObject
{
    Q_OBJECT

private slots:
    void parsesQuotedTextAndWindowsPath()
    {
        const Cli::ParsedCommand command = Cli::parseCommand(
            QStringLiteral("send-file \"C:\\My Files\\photo.png\""));

        QCOMPARE(command.name, QStringLiteral("send-file"));
        QCOMPARE(command.arguments,
                 QStringList{QStringLiteral("C:\\My Files\\photo.png")});
        QVERIFY(command.error.isEmpty());
    }

    void discoveryFailureRollsBackTransport()
    {
        auto discovery = std::make_unique<FakeDiscovery>();
        discovery->startAllowed = false;
        auto transport = std::make_unique<FakeTransport>();
        FakeTransport *transportHandle = transport.get();
        ChatService service(std::move(discovery),
                            std::move(transport),
                            std::make_unique<FakeRepository>(),
                            std::make_unique<FakeIdentityStore>());

        const Domain::OperationResult result = service.start();

        QVERIFY(!static_cast<bool>(result));
        QVERIFY(transportHandle->stopCalled);
        QVERIFY(!service.running());
    }

    void incomingMessageIsUnreadUntilMarkedRead()
    {
        auto discovery = std::make_unique<FakeDiscovery>();
        FakeDiscovery *discoveryHandle = discovery.get();
        auto transport = std::make_unique<FakeTransport>();
        FakeTransport *transportHandle = transport.get();
        ChatService service(std::move(discovery),
                            std::move(transport),
                            std::make_unique<FakeRepository>(),
                            std::make_unique<FakeIdentityStore>());
        QVERIFY(static_cast<bool>(service.start()));
        emit discoveryHandle->peerFound(testPeer());

        emit transportHandle->textReceived(
            {QStringLiteral("message-id"),
             testPeer(),
             QStringLiteral("hello"),
             QDateTime::currentDateTimeUtc()});

        QCOMPARE(service.totalUnreadCount(), 1);
        QCOMPARE(service.messages(testPeer().peerId).size(), 1);
        QVERIFY(static_cast<bool>(
            service.markConversationRead(testPeer().peerId)));
        QCOMPARE(service.totalUnreadCount(), 0);
    }

    void sendsTextThroughOnlinePeer()
    {
        auto discovery = std::make_unique<FakeDiscovery>();
        FakeDiscovery *discoveryHandle = discovery.get();
        auto transport = std::make_unique<FakeTransport>();
        FakeTransport *transportHandle = transport.get();
        ChatService service(std::move(discovery),
                            std::move(transport),
                            std::make_unique<FakeRepository>(),
                            std::make_unique<FakeIdentityStore>());
        QVERIFY(static_cast<bool>(service.start()));
        emit discoveryHandle->peerFound(testPeer());

        const Domain::OperationResult result = service.sendText(
            testPeer().peerId,
            QStringLiteral("hello"));

        QVERIFY(static_cast<bool>(result));
        QCOMPARE(transportHandle->lastPeer.peerId, testPeer().peerId);
        QCOMPARE(transportHandle->lastText, QStringLiteral("hello"));
        QVERIFY(!transportHandle->lastMessageId.isEmpty());
    }
};

QTEST_MAIN(ChatServiceTest)

#include "chatservicetest.moc"
