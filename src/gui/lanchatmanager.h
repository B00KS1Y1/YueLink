/**
 * @file lanchatmanager.h
 * @brief 声明聚合聊天视图模型与桌面服务的 QML 外观。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef LANCHATMANAGER_H
#define LANCHATMANAGER_H

#include <QAbstractItemModel>
#include <QList>
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class ChatCoordinator;
class ConversationViewModel;
class DesktopIntegration;
class IFileLauncher;
class INotificationService;
class PeerListViewModel;
class QJSEngine;
class QQmlEngine;

class LanChatManager final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LanChat)
    QML_SINGLETON
    Q_PROPERTY(QAbstractItemModel *peers READ peers CONSTANT)
    Q_PROPERTY(QAbstractItemModel *messages READ messages CONSTANT)
    Q_PROPERTY(QString peerSearchText READ peerSearchText WRITE setPeerSearchText NOTIFY peerSearchTextChanged)
    Q_PROPERTY(QString messageSearchText READ messageSearchText WRITE setMessageSearchText NOTIFY messageSearchTextChanged)
    Q_PROPERTY(QString localName READ localName NOTIFY localProfileChanged)
    Q_PROPERTY(QString localInitial READ localInitial NOTIFY localProfileChanged)
    Q_PROPERTY(QString currentPeerId READ currentPeerId NOTIFY currentPeerIdChanged)
    Q_PROPERTY(int onlineCount READ onlineCount NOTIFY onlineCountChanged)
    Q_PROPERTY(int totalUnreadCount READ totalUnreadCount NOTIFY totalUnreadCountChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    /**
     * @brief 设置 QML 单例工厂使用的聊天协调器。
     * @param coordinator 由应用程序生命周期管理的聊天协调器。
     */
    static void setCoordinator(ChatCoordinator *coordinator);
    /**
     * @brief 为 QML 引擎创建聊天管理器单例。
     * @param qmlEngine 请求单例的 QML 引擎。
     * @param jsEngine 请求单例的 JavaScript 引擎。
     * @return 使用已注入协调器创建的管理器实例。
     */
    static LanChatManager *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    /**
     * @brief 使用默认桌面适配器构造 QML 聊天管理器。
     * @param coordinator 共享聊天协调器。
     * @param parent 可选的 QObject 父对象。
     */
    explicit LanChatManager(ChatCoordinator *coordinator,
                            QObject *parent = nullptr);
    /**
     * @brief 使用指定桌面适配器构造 QML 聊天管理器。
     * @param coordinator 共享聊天协调器。
     * @param fileLauncher 由桌面集成服务接管所有权的文件启动器。
     * @param notificationService 由桌面集成服务接管所有权的通知服务。
     * @param parent 可选的 QObject 父对象。
     */
    LanChatManager(ChatCoordinator *coordinator,
                   std::unique_ptr<IFileLauncher> fileLauncher,
                   std::unique_ptr<INotificationService> notificationService,
                   QObject *parent = nullptr);
    /** @brief 销毁 QML 聊天管理器及其视图模型与桌面服务。 */
    ~LanChatManager() override;

    /**
     * @brief 返回供 QML 使用的好友列表模型。
     * @return 好友列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *peers();
    /**
     * @brief 返回供 QML 使用的当前会话消息模型。
     * @return 消息列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *messages();
    /**
     * @brief 返回好友列表当前使用的搜索文本。
     * @return 未经裁剪的好友搜索文本。
     */
    [[nodiscard]] QString peerSearchText() const;
    /**
     * @brief 更新好友列表搜索文本。
     * @param text 新的搜索文本。
     */
    void setPeerSearchText(const QString &text);
    /**
     * @brief 返回当前会话使用的消息搜索文本。
     * @return 未经裁剪的消息搜索文本。
     */
    [[nodiscard]] QString messageSearchText() const;
    /**
     * @brief 更新当前会话的消息搜索文本。
     * @param text 新的搜索文本。
     */
    void setMessageSearchText(const QString &text);
    /**
     * @brief 返回本地显示名称。
     * @return 当前本地显示名称。
     */
    [[nodiscard]] QString localName() const;
    /**
     * @brief 返回本地名称首字符。
     * @return 用于头像显示的本地名称首字符。
     */
    [[nodiscard]] QString localInitial() const;
    /**
     * @brief 返回当前选中会话的节点标识。
     * @return 当前节点标识；没有选择时返回空字符串。
     */
    [[nodiscard]] QString currentPeerId() const;
    /**
     * @brief 返回当前在线好友数量。
     * @return 在线好友数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有好友未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回聊天协调器是否正在运行。
     * @return 服务正在运行时返回 @c true。
     */
    [[nodiscard]] bool running() const;
    /**
     * @brief 返回最近一次聊天协调错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] QString lastError() const;

    /**
     * @brief 启动聊天服务。
     * @return 服务启动成功时返回 @c true。
     */
    Q_INVOKABLE bool start();
    /** @brief 停止聊天服务。 */
    Q_INVOKABLE void stop();
    /**
     * @brief 选择需要显示的会话。
     * @param peerId 会话对应的节点标识。
     * @return 节点存在且选择成功时返回 @c true。
     */
    Q_INVOKABLE bool selectPeer(const QString &peerId);
    /**
     * @brief 将指定会话标记为已读。
     * @param peerId 会话对应的节点标识。
     * @return 操作成功时返回 @c true。
     */
    Q_INVOKABLE bool markConversationRead(const QString &peerId);
    /**
     * @brief 返回指定节点的 QML 属性映射。
     * @param peerId 待查询的节点标识。
     * @return 节点属性；节点未知时返回空映射。
     */
    Q_INVOKABLE QVariantMap peerInfo(const QString &peerId) const;
    /**
     * @brief 更新并保存本地显示名称。
     * @param displayName 新的显示名称。
     * @return 更新成功时返回 @c true。
     */
    Q_INVOKABLE bool updateLocalProfile(const QString &displayName);
    /**
     * @brief 向指定节点发送文本消息。
     * @param peerId 目标节点标识。
     * @param text 消息内容。
     * @return 消息被接受发送时返回 @c true。
     */
    Q_INVOKABLE bool sendMessage(const QString &peerId, const QString &text);
    /**
     * @brief 向指定节点发送本地文件。
     * @param peerId 目标节点标识。
     * @param fileUrl 本地文件 URL。
     * @return 文件传输请求被接受时返回 @c true。
     */
    Q_INVOKABLE bool sendFile(const QString &peerId, const QUrl &fileUrl);
    /**
     * @brief 向指定节点发送多个本地文件。
     * @param peerId 目标节点标识。
     * @param fileUrls 本地文件 URL 列表。
     * @return 已接受发送的文件数量。
     */
    Q_INVOKABLE int sendFiles(const QString &peerId, const QList<QUrl> &fileUrls);
    /**
     * @brief 返回剪贴板中可发送的本地图片 URL。
     * @return 本地图片 URL 列表；没有图片或保存失败时返回空列表。
     */
    Q_INVOKABLE QList<QUrl> clipboardImageUrls();
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 成功取消匹配传输时返回 @c true。
     */
    Q_INVOKABLE bool cancelFileTransfer(const QString &peerId,
                                        const QString &transferId);
    /**
     * @brief 使用系统默认应用打开文件。
     * @param filePath 待打开的本地文件路径。
     * @return 已成功请求系统打开文件时返回 @c true。
     */
    Q_INVOKABLE bool openFile(const QString &filePath);
    /**
     * @brief 在系统文件管理器中定位文件。
     * @param filePath 待定位的本地文件路径。
     * @return 已成功请求系统定位文件时返回 @c true。
     */
    Q_INVOKABLE bool revealFile(const QString &filePath);
    /**
     * @brief 启用或禁用桌面通知。
     * @param enabled 是否允许显示通知。
     */
    Q_INVOKABLE void setNotificationsEnabled(bool enabled);

signals:
    /** @brief 好友搜索文本发生变化时发出。 */
    void peerSearchTextChanged();
    /** @brief 消息搜索文本发生变化时发出。 */
    void messageSearchTextChanged();
    /** @brief 本地身份信息发生变化时发出。 */
    void localProfileChanged();
    /** @brief 当前会话节点标识发生变化时发出。 */
    void currentPeerIdChanged();
    /** @brief 在线好友数量发生变化时发出。 */
    void onlineCountChanged();
    /** @brief 未读消息总数发生变化时发出。 */
    void totalUnreadCountChanged();
    /** @brief 服务运行状态发生变化时发出。 */
    void runningChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();
    /**
     * @brief 发现此前未知的节点时发出。
     * @param peerId 新节点标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 已知节点状态发生变化时发出。
     * @param peerId 已更新节点标识。
     */
    void peerUpdated(const QString &peerId);
    /**
     * @brief 收到文本消息时发出。
     * @param peerId 发送方节点标识。
     * @param text 收到的消息文本。
     */
    void messageReceived(const QString &peerId, const QString &text);
    /**
     * @brief 文本消息发送失败时发出。
     * @param peerId 目标节点标识。
     * @param reason 失败原因。
     */
    void sendFailed(const QString &peerId, const QString &reason);
    /**
     * @brief 接收文件成功保存后发出。
     * @param peerId 发送方节点标识。
     * @param filePath 接收文件的本地路径。
     */
    void fileReceived(const QString &peerId, const QString &filePath);
    /**
     * @brief 文件传输失败时发出。
     * @param peerId 远端节点标识。
     * @param reason 失败原因。
     */
    void fileTransferFailed(const QString &peerId, const QString &reason);
    /**
     * @brief 一般性服务操作失败时发出。
     * @param reason 失败原因。
     */
    void operationFailed(const QString &reason);
    /**
     * @brief 用户激活聊天通知时发出。
     * @param peerId 通知关联的节点标识。
     */
    void notificationActivated(const QString &peerId);

private:
    /** @brief 连接协调器、视图模型与桌面服务事件。 */
    void connectComponents();

    ChatCoordinator *m_coordinator = nullptr;
    std::unique_ptr<PeerListViewModel> m_peers;
    std::unique_ptr<ConversationViewModel> m_conversation;
    std::unique_ptr<DesktopIntegration> m_desktop;
    static ChatCoordinator *s_coordinator;
};

#endif // LANCHATMANAGER_H
