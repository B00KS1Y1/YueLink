/**
 * @file consolecontroller.h
 * @brief 声明交互式 CLI 应用控制器。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef CONSOLECONTROLLER_H
#define CONSOLECONTROLLER_H

#include <QObject>
#include <QString>

class ChatService;
class ConsoleRenderer;

namespace Cli
{
struct ParsedCommand;
}

class ConsoleController final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造终端应用控制器。
     * @param service 共享聊天服务。
     * @param renderer 终端输出渲染器。
     * @param parent 可选的 QObject 父对象。
     */
    ConsoleController(ChatService *service,
                      ConsoleRenderer *renderer,
                      QObject *parent = nullptr);
    /** @brief 连接服务事件并显示初始终端提示。 */
    void start();

public slots:
    /**
     * @brief 解析并执行一行终端输入。
     * @param line 用户输入的原始文本。
     */
    void handleLine(const QString &line);

signals:
    /** @brief 用户请求退出终端应用时发出。 */
    void quitRequested();

private:
    /** @brief 连接聊天服务事件与终端输出处理器。 */
    void connectService();
    /**
     * @brief 执行已解析的终端命令。
     * @param command 待执行的命令。
     */
    void execute(const Cli::ParsedCommand &command);
    /** @brief 输出命令帮助信息。 */
    void showHelp();
    /** @brief 输出当前服务状态。 */
    void showStatus();
    /**
     * @brief 输出已知节点列表。
     * @param onlineOnly 是否仅显示在线节点。
     */
    void showPeers(bool onlineOnly);
    /**
     * @brief 输出当前会话的最近消息。
     * @param limit 最多输出的消息数量。
     */
    void showMessages(int limit);
    /** @brief 输出当前文件传输列表。 */
    void showTransfers();
    /** @brief 输出本地身份信息。 */
    void showProfile();
    /** @brief 输出当前应用配置。 */
    void showConfig();
    /**
     * @brief 将序号或节点标识解析为完整节点标识。
     * @param selector 用户输入的节点选择器。
     * @return 匹配的节点标识；未找到时返回空字符串。
     */
    [[nodiscard]] QString resolvePeer(const QString &selector) const;
    /**
     * @brief 将操作结果输出为文本或 JSON Lines 事件。
     * @param operation 操作名称。
     * @param succeeded 操作是否成功。
     * @param code 机器可读的结果码。
     * @param message 便于用户阅读的结果说明。
     */
    void reportResult(const QString &operation,
                      bool succeeded,
                      const QString &code,
                      const QString &message);

    ChatService *m_service = nullptr;
    ConsoleRenderer *m_renderer = nullptr;
    QString m_currentPeerId;
    bool m_quitting = false;
};

#endif // CONSOLECONTROLLER_H
