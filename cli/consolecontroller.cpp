#include "consolecontroller.h"

#include "application/chatservice.h"
#include "cli/commandparser.h"
#include "cli/consolerenderer.h"
#include "config/configstore.h"
#include "domain/chattypes.h"
#include "utils/path.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

namespace
{
QJsonObject peerObject(const Domain::Peer &peer)
{
    return {{QStringLiteral("peer_id"), peer.endpoint.peerId},
            {QStringLiteral("name"), peer.endpoint.displayName},
            {QStringLiteral("address"), peer.endpoint.address.toString()},
            {QStringLiteral("port"), peer.endpoint.tcpPort},
            {QStringLiteral("online"), peer.online},
            {QStringLiteral("unread"), peer.unreadCount},
            {QStringLiteral("last_message"), peer.lastMessage},
            {QStringLiteral("last_activity"),
             peer.lastActivity.toUTC().toString(Qt::ISODateWithMs)}};
}

QJsonObject messageObject(const Domain::Message &message)
{
    return {{QStringLiteral("message_id"), message.messageId},
            {QStringLiteral("peer_id"), message.peerId},
            {QStringLiteral("from_me"), message.fromMe},
            {QStringLiteral("kind"), Domain::messageKindName(message.kind)},
            {QStringLiteral("text"), message.text},
            {QStringLiteral("timestamp"),
             message.timestamp.toUTC().toString(Qt::ISODateWithMs)},
            {QStringLiteral("status"),
             Domain::deliveryStateName(message.deliveryState)},
            {QStringLiteral("file_name"), message.fileName},
            {QStringLiteral("file_path"), message.filePath},
            {QStringLiteral("file_size"), static_cast<double>(message.fileSize)},
            {QStringLiteral("progress"), message.fileProgress}};
}
} // namespace

ConsoleController::ConsoleController(ChatService *service,
                                     ConsoleRenderer *renderer,
                                     QObject *parent)
: QObject(parent)
, m_service(service)
, m_renderer(renderer)
{
    Q_ASSERT(m_service);
    Q_ASSERT(m_renderer);
    connectService();
}

void ConsoleController::start()
{
    if (m_renderer->jsonLines())
    {
        m_renderer->event(QStringLiteral("ready"),
                          {{QStringLiteral("running"), m_service->running()},
                           {QStringLiteral("identity"),
                            m_service->localIdentity().displayName}});
    }
    else
    {
        m_renderer->line(QStringLiteral("YueLink CLI 已启动。输入 help 查看命令。"));
        showStatus();
    }
    m_renderer->prompt();
}

void ConsoleController::handleLine(const QString &line)
{
    if (m_quitting)
    {
        return;
    }
    const Cli::ParsedCommand command = Cli::parseCommand(line);
    if (!command.error.isEmpty())
    {
        m_renderer->error(QStringLiteral("command.syntax"), command.error);
        m_renderer->prompt();
        return;
    }
    if (command.name.isEmpty())
    {
        m_renderer->prompt();
        return;
    }
    execute(command);
    if (!m_quitting)
    {
        m_renderer->prompt();
    }
}

void ConsoleController::connectService()
{
    connect(m_service,
            &ChatService::runningChanged,
            this,
            [this]() {
                m_renderer->event(
                    QStringLiteral("service_state"),
                    {{QStringLiteral("running"), m_service->running()},
                     {QStringLiteral("message"),
                      m_service->running() ? QStringLiteral("网络服务已启动。")
                                           : QStringLiteral("网络服务已停止。")}});
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::peerDiscovered,
            this,
            [this](const QString &peerId) {
                Domain::Peer peer;
                if (!m_service->peer(peerId, &peer))
                {
                    return;
                }
                QJsonObject data = peerObject(peer);
                data.insert(QStringLiteral("message"),
                            QStringLiteral("好友上线：%1 (%2)")
                                .arg(peer.endpoint.displayName, peerId));
                m_renderer->event(QStringLiteral("peer_discovered"), data);
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::peerUpdated,
            this,
            [this](const QString &peerId) {
                Domain::Peer peer;
                if (!m_service->peer(peerId, &peer))
                {
                    return;
                }
                QJsonObject data = peerObject(peer);
                data.insert(QStringLiteral("message"),
                            QStringLiteral("好友状态更新：%1 · %2")
                                .arg(peer.endpoint.displayName,
                                     peer.online ? QStringLiteral("在线")
                                                 : QStringLiteral("离线")));
                m_renderer->event(QStringLiteral("peer_changed"), data);
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::conversationChanged,
            this,
            [this](const QString &peerId) {
                if (peerId == m_currentPeerId)
                {
                    static_cast<void>(
                        m_service->markConversationRead(peerId));
                }
                if (!m_renderer->jsonLines())
                {
                    return;
                }
                const QList<Domain::Message> messages = m_service->messages(peerId, 1);
                if (!messages.isEmpty())
                {
                    m_renderer->event(QStringLiteral("message_changed"),
                                      messageObject(messages.constLast()));
                }
            });
    connect(m_service,
            &ChatService::messageReceived,
            this,
            [this](const QString &peerId, const QString &text) {
                Domain::Peer peer;
                static_cast<void>(m_service->peer(peerId, &peer));
                QJsonObject data{{QStringLiteral("peer_id"), peerId},
                                 {QStringLiteral("name"), peer.endpoint.displayName},
                                 {QStringLiteral("text"), text},
                                 {QStringLiteral("message"),
                                  QStringLiteral("%1：%2")
                                      .arg(peer.endpoint.displayName, text)}};
                m_renderer->event(QStringLiteral("message_received"), data);
                if (peerId == m_currentPeerId)
                {
                    static_cast<void>(
                        m_service->markConversationRead(peerId));
                }
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::sendFailed,
            this,
            [this](const QString &peerId, const QString &reason) {
                m_renderer->error(QStringLiteral("message.send"),
                                  QStringLiteral("%1：%2").arg(peerId, reason));
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::fileReceived,
            this,
            [this](const QString &peerId, const QString &filePath) {
                m_renderer->event(
                    QStringLiteral("file_received"),
                    {{QStringLiteral("peer_id"), peerId},
                     {QStringLiteral("path"), filePath},
                     {QStringLiteral("message"),
                      QStringLiteral("文件已接收：%1").arg(filePath)}});
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::fileTransferFailed,
            this,
            [this](const QString &peerId,
                   const QString &reason,
                   bool) {
                m_renderer->error(QStringLiteral("file.transfer"),
                                  QStringLiteral("%1：%2").arg(peerId, reason));
                m_renderer->prompt();
            });
    connect(m_service,
            &ChatService::operationFailed,
            this,
            [this](const QString &reason) {
                m_renderer->error(QStringLiteral("operation.failed"), reason);
                m_renderer->prompt();
            });
}

void ConsoleController::execute(const Cli::ParsedCommand &command)
{
    if (command.name == QLatin1String("help"))
    {
        showHelp();
        return;
    }
    if (command.name == QLatin1String("status"))
    {
        showStatus();
        return;
    }
    if (command.name == QLatin1String("peers"))
    {
        showPeers(command.arguments.value(0).compare(QStringLiteral("online"),
                                                     Qt::CaseInsensitive)
                  == 0);
        return;
    }
    if (command.name == QLatin1String("use"))
    {
        if (command.arguments.size() != 1)
        {
            m_renderer->error(QStringLiteral("command.arguments"),
                              QStringLiteral("用法：use <peer-id|序号>"));
            return;
        }
        const QString peerId = resolvePeer(command.arguments.constFirst());
        Domain::Peer peer;
        if (peerId.isEmpty() || !m_service->peer(peerId, &peer))
        {
            m_renderer->error(QStringLiteral("peer.not_found"),
                              QStringLiteral("找不到指定好友。"));
            return;
        }
        m_currentPeerId = peerId;
        static_cast<void>(m_service->messages(peerId));
        static_cast<void>(m_service->markConversationRead(peerId));
        m_renderer->event(
            QStringLiteral("peer_selected"),
            {{QStringLiteral("peer_id"), peerId},
             {QStringLiteral("name"), peer.endpoint.displayName},
             {QStringLiteral("message"),
              QStringLiteral("当前会话：%1 (%2)")
                  .arg(peer.endpoint.displayName, peerId)}});
        return;
    }
    if (command.name == QLatin1String("messages"))
    {
        bool valid = true;
        const int limit = command.arguments.isEmpty()
                              ? 50
                              : command.arguments.constFirst().toInt(&valid);
        if (!valid || limit <= 0)
        {
            m_renderer->error(QStringLiteral("command.arguments"),
                              QStringLiteral("消息数量必须是正整数。"));
            return;
        }
        showMessages(limit);
        return;
    }
    if (command.name == QLatin1String("send"))
    {
        if (m_currentPeerId.isEmpty() || command.arguments.isEmpty())
        {
            m_renderer->error(QStringLiteral("command.arguments"),
                              QStringLiteral("请先 use 好友，再执行 send <文本>。"));
            return;
        }
        const Domain::OperationResult result = m_service->sendText(
            m_currentPeerId,
            command.arguments.join(QLatin1Char(' ')));
        reportResult(QStringLiteral("message_queued"),
                     static_cast<bool>(result),
                     result.code,
                     result.message);
        return;
    }
    if (command.name == QLatin1String("send-file"))
    {
        if (m_currentPeerId.isEmpty() || command.arguments.isEmpty())
        {
            m_renderer->error(QStringLiteral("command.arguments"),
                              QStringLiteral("请先 use 好友，再执行 send-file <路径>。"));
            return;
        }
        const int accepted = m_service->sendFiles(m_currentPeerId,
                                                   command.arguments);
        m_renderer->event(
            QStringLiteral("files_queued"),
            {{QStringLiteral("requested"), command.arguments.size()},
             {QStringLiteral("accepted"), accepted},
             {QStringLiteral("message"),
              QStringLiteral("已接受 %1/%2 个文件发送任务。")
                  .arg(accepted)
                  .arg(command.arguments.size())}});
        return;
    }
    if (command.name == QLatin1String("transfers"))
    {
        showTransfers();
        return;
    }
    if (command.name == QLatin1String("cancel"))
    {
        if (m_currentPeerId.isEmpty() || command.arguments.size() != 1)
        {
            m_renderer->error(QStringLiteral("command.arguments"),
                              QStringLiteral("用法：cancel <transfer-id>"));
            return;
        }
        const Domain::OperationResult result = m_service->cancelFileTransfer(
            m_currentPeerId,
            command.arguments.constFirst());
        reportResult(QStringLiteral("transfer_cancelled"),
                     static_cast<bool>(result),
                     result.code,
                     result.message);
        return;
    }
    if (command.name == QLatin1String("read"))
    {
        if (m_currentPeerId.isEmpty())
        {
            m_renderer->error(QStringLiteral("peer.not_selected"),
                              QStringLiteral("请先选择好友。"));
            return;
        }
        const Domain::OperationResult result = m_service->markConversationRead(
            m_currentPeerId);
        reportResult(QStringLiteral("conversation_read"),
                     static_cast<bool>(result),
                     result.code,
                     result.message);
        return;
    }
    if (command.name == QLatin1String("profile"))
    {
        if (command.arguments.isEmpty())
        {
            showProfile();
            return;
        }
        if (command.arguments.constFirst() == QLatin1String("set")
            && command.arguments.size() > 1)
        {
            const Domain::OperationResult result = m_service->updateLocalProfile(
                command.arguments.mid(1).join(QLatin1Char(' ')));
            reportResult(QStringLiteral("profile_updated"),
                         static_cast<bool>(result),
                         result.code,
                         result.message);
            return;
        }
        m_renderer->error(QStringLiteral("command.arguments"),
                          QStringLiteral("用法：profile 或 profile set <昵称>"));
        return;
    }
    if (command.name == QLatin1String("config")
        && (command.arguments.isEmpty()
            || command.arguments.constFirst() == QLatin1String("show")))
    {
        showConfig();
        return;
    }
    if (command.name == QLatin1String("quit")
        || command.name == QLatin1String("exit"))
    {
        m_quitting = true;
        emit quitRequested();
        return;
    }

    m_renderer->error(QStringLiteral("command.unknown"),
                      QStringLiteral("未知命令：%1").arg(command.name));
}

void ConsoleController::showHelp()
{
    m_renderer->line(QStringLiteral(
        "命令：\n"
        "  status                     查看服务状态\n"
        "  peers [online|all]         查看好友\n"
        "  use <peer-id|序号>         选择会话\n"
        "  messages [数量]            查看历史消息\n"
        "  send <文本>                发送消息\n"
        "  send-file <路径> [...]     发送文件\n"
        "  transfers                  查看文件传输\n"
        "  cancel <transfer-id>       取消文件传输\n"
        "  read                       标记当前会话已读\n"
        "  profile                    查看本机身份\n"
        "  profile set <昵称>         修改昵称\n"
        "  config show                查看配置路径\n"
        "  quit                       退出"));
}

void ConsoleController::showStatus()
{
    const Network::LocalIdentity identity = m_service->localIdentity();
    m_renderer->event(
        QStringLiteral("status"),
        {{QStringLiteral("running"), m_service->running()},
         {QStringLiteral("device_id"), identity.deviceId},
         {QStringLiteral("display_name"), identity.displayName},
         {QStringLiteral("online_count"), m_service->onlineCount()},
         {QStringLiteral("unread_count"), m_service->totalUnreadCount()},
         {QStringLiteral("selected_peer"), m_currentPeerId},
         {QStringLiteral("message"),
          QStringLiteral("状态：%1；本机：%2；在线：%3；未读：%4")
              .arg(m_service->running() ? QStringLiteral("运行中")
                                        : QStringLiteral("已停止"),
                   identity.displayName)
              .arg(m_service->onlineCount())
              .arg(m_service->totalUnreadCount())}});
}

void ConsoleController::showPeers(bool onlineOnly)
{
    QJsonArray values;
    int visibleCount = 0;
    const QList<Domain::Peer> peers = m_service->peers();
    for (int index = 0; index < peers.size(); ++index)
    {
        const Domain::Peer &peer = peers.at(index);
        if (onlineOnly && !peer.online)
        {
            continue;
        }
        ++visibleCount;
        const int displayIndex = index + 1;
        QJsonObject value = peerObject(peer);
        value.insert(QStringLiteral("index"), displayIndex);
        values.append(value);
        if (!m_renderer->jsonLines())
        {
            m_renderer->line(
                QStringLiteral("%1. [%2] %3 · 未读 %4\n   %5")
                    .arg(displayIndex)
                    .arg(peer.online ? QStringLiteral("在线")
                                     : QStringLiteral("离线"),
                         peer.endpoint.displayName)
                    .arg(peer.unreadCount)
                    .arg(peer.endpoint.peerId));
        }
    }
    if (m_renderer->jsonLines())
    {
        m_renderer->event(QStringLiteral("peers"),
                          {{QStringLiteral("items"), values}});
    }
    else if (visibleCount == 0)
    {
        m_renderer->line(QStringLiteral("暂无好友。"));
    }
}

void ConsoleController::showMessages(int limit)
{
    if (m_currentPeerId.isEmpty())
    {
        m_renderer->error(QStringLiteral("peer.not_selected"),
                          QStringLiteral("请先使用 use 选择好友。"));
        return;
    }
    const QList<Domain::Message> messages = m_service->messages(m_currentPeerId,
                                                                 limit);
    QJsonArray values;
    for (const Domain::Message &message : messages)
    {
        values.append(messageObject(message));
        if (!m_renderer->jsonLines())
        {
            const QString content = message.kind == Domain::MessageKind::File
                                        ? QStringLiteral("[文件] %1 %2")
                                              .arg(message.fileName,
                                                   message.filePath)
                                        : message.text;
            m_renderer->line(
                QStringLiteral("[%1] %2：%3 (%4)")
                    .arg(message.timestamp.toLocalTime().toString(
                             QStringLiteral("yyyy-MM-dd HH:mm:ss")),
                         message.fromMe ? QStringLiteral("我")
                                        : QStringLiteral("对方"),
                         content,
                         Domain::deliveryStateName(message.deliveryState)));
        }
    }
    if (m_renderer->jsonLines())
    {
        m_renderer->event(QStringLiteral("messages"),
                          {{QStringLiteral("peer_id"), m_currentPeerId},
                           {QStringLiteral("items"), values}});
    }
    else if (messages.isEmpty())
    {
        m_renderer->line(QStringLiteral("当前会话没有消息。"));
    }
    static_cast<void>(m_service->markConversationRead(m_currentPeerId));
}

void ConsoleController::showTransfers()
{
    if (m_currentPeerId.isEmpty())
    {
        m_renderer->error(QStringLiteral("peer.not_selected"),
                          QStringLiteral("请先使用 use 选择好友。"));
        return;
    }
    QJsonArray values;
    for (const Domain::Message &message : m_service->messages(m_currentPeerId, 500))
    {
        if (message.kind != Domain::MessageKind::File)
        {
            continue;
        }
        values.append(messageObject(message));
        if (!m_renderer->jsonLines())
        {
            m_renderer->line(
                QStringLiteral("%1 · %2 · %3% · %4")
                    .arg(message.messageId,
                         message.fileName)
                    .arg(qRound(message.fileProgress * 100.0))
                    .arg(Domain::deliveryStateName(message.deliveryState)));
        }
    }
    if (m_renderer->jsonLines())
    {
        m_renderer->event(QStringLiteral("transfers"),
                          {{QStringLiteral("peer_id"), m_currentPeerId},
                           {QStringLiteral("items"), values}});
    }
    else if (values.isEmpty())
    {
        m_renderer->line(QStringLiteral("当前会话没有文件传输。"));
    }
}

void ConsoleController::showProfile()
{
    const Network::LocalIdentity identity = m_service->localIdentity();
    m_renderer->event(
        QStringLiteral("profile"),
        {{QStringLiteral("device_id"), identity.deviceId},
         {QStringLiteral("display_name"), identity.displayName},
         {QStringLiteral("message"),
          QStringLiteral("昵称：%1\n设备 ID：%2")
              .arg(identity.displayName, identity.deviceId)}});
}

void ConsoleController::showConfig()
{
    const Config::DatabaseConfig database = Config::database.get();
    const Config::LogConfig log = Config::log.get();
    m_renderer->event(
        QStringLiteral("config"),
        {{QStringLiteral("config_directory"), Utils::Path::configDirectory()},
         {QStringLiteral("data_directory"), Utils::Path::dataDirectory()},
         {QStringLiteral("database_driver"),
          QString::fromStdString(database.driver)},
         {QStringLiteral("log_level"), QString::fromStdString(log.level)},
         {QStringLiteral("message"),
          QStringLiteral("配置目录：%1\n数据目录：%2\n数据库：%3\n日志级别：%4")
              .arg(Utils::Path::configDirectory(),
                   Utils::Path::dataDirectory(),
                   QString::fromStdString(database.driver),
                   QString::fromStdString(log.level))}});
}

QString ConsoleController::resolvePeer(const QString &selector) const
{
    bool numeric = false;
    const int selectedIndex = selector.toInt(&numeric);
    const QList<Domain::Peer> peers = m_service->peers();
    if (numeric && selectedIndex > 0 && selectedIndex <= peers.size())
    {
        return peers.at(selectedIndex - 1).endpoint.peerId;
    }

    QString match;
    for (const Domain::Peer &peer : peers)
    {
        if (peer.endpoint.peerId == selector)
        {
            return selector;
        }
        if (peer.endpoint.peerId.startsWith(selector, Qt::CaseInsensitive))
        {
            if (!match.isEmpty())
            {
                return {};
            }
            match = peer.endpoint.peerId;
        }
    }
    return match;
}

void ConsoleController::reportResult(const QString &operation,
                                     bool succeeded,
                                     const QString &code,
                                     const QString &message)
{
    if (!succeeded)
    {
        m_renderer->error(code.isEmpty() ? QStringLiteral("operation.failed")
                                         : code,
                          message);
        return;
    }
    m_renderer->event(operation,
                      {{QStringLiteral("message"), QStringLiteral("操作已接受。")}});
}
