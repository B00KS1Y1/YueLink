/**
 * @file desktopintegration.h
 * @brief 声明剪贴板、文件启动与通知的桌面集成服务。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef DESKTOPINTEGRATION_H
#define DESKTOPINTEGRATION_H

#include <QList>
#include <QObject>
#include <QUrl>

#include <memory>

class ChatCoordinator;
class ConversationViewModel;
class IFileLauncher;
class INotificationService;

class DesktopIntegration final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造桌面集成服务并接管平台适配器所有权。
     * @param coordinator 非拥有的聊天协调器；必须覆盖本对象生命周期。
     * @param conversation 非拥有的当前会话视图模型；必须覆盖本对象生命周期。
     * @param fileLauncher 文件启动适配器。
     * @param notificationService 桌面通知适配器。
     * @param parent 可选的 QObject 父对象。
     */
    DesktopIntegration(ChatCoordinator *coordinator,
                       ConversationViewModel *conversation,
                       std::unique_ptr<IFileLauncher> fileLauncher,
                       std::unique_ptr<INotificationService> notificationService,
                       QObject *parent = nullptr);
    /** @brief 销毁桌面集成服务及其拥有的平台适配器。 */
    ~DesktopIntegration() override;

    /**
     * @brief 返回剪贴板中可发送的本地图片 URL。
     * @param[out] errorMessage 保存剪贴板位图失败时接收错误说明。
     * @return 本地图片 URL 列表；没有图片或失败时返回空列表。
     */
    [[nodiscard]] QList<QUrl> clipboardImageUrls(QString *errorMessage);
    /**
     * @brief 使用系统默认应用打开文件。
     * @param filePath 待打开的本地文件路径。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 已成功请求系统打开文件时返回 @c true。
     */
    [[nodiscard]] bool openFile(const QString &filePath, QString *errorMessage);
    /**
     * @brief 在系统文件管理器中定位文件。
     * @param filePath 待定位的本地文件路径。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 已成功请求系统定位文件时返回 @c true。
     */
    [[nodiscard]] bool revealFile(const QString &filePath, QString *errorMessage);
    /**
     * @brief 启用或禁用桌面通知。
     * @param enabled 是否允许显示通知。
     */
    void setNotificationsEnabled(bool enabled);

signals:
    /**
     * @brief 用户激活聊天通知时发出。
     * @param peerId 通知关联的节点标识。
     */
    void notificationActivated(const QString &peerId);

private:
    /** @brief 连接聊天事件与桌面通知策略。 */
    void connectServices();
    /**
     * @brief 处理收到的文本消息及前台已读策略。
     * @param peerId 发送方节点标识。
     * @param text 收到的消息文本。
     */
    void handleIncomingMessage(const QString &peerId, const QString &text);
    /**
     * @brief 处理已接收文件的通知。
     * @param peerId 发送方节点标识。
     * @param filePath 接收文件的本地路径。
     */
    void handleFileReceived(const QString &peerId, const QString &filePath);
    /**
     * @brief 处理文件传输失败通知。
     * @param peerId 远端节点标识。
     * @param reason 失败原因。
     * @param incoming 是否为接收方向。
     */
    void handleFileTransferFailed(const QString &peerId,
                                  const QString &reason,
                                  bool incoming);
    /**
     * @brief 在应用处于后台时显示聊天通知。
     * @param peerId 消息来源节点标识。
     * @param message 通知预览文本。
     */
    void showIncomingNotification(const QString &peerId,
                                  const QString &message);

    ChatCoordinator *m_coordinator = nullptr;
    ConversationViewModel *m_conversation = nullptr;
    std::unique_ptr<IFileLauncher> m_fileLauncher;
    std::unique_ptr<INotificationService> m_notificationService;
};

#endif // DESKTOPINTEGRATION_H
