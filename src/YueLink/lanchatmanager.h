/**
 * @file lanchatmanager.h
 * @brief 声明统一单聊与群聊视图模型及桌面服务的 QML 外观。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef LANCHATMANAGER_H
#define LANCHATMANAGER_H

#include <QAbstractItemModel>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class ChatCoordinator;
class ConversationListViewModel;
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
    Q_PROPERTY(QAbstractItemModel *conversations READ conversations CONSTANT)
    Q_PROPERTY(QAbstractItemModel *peers READ peers CONSTANT)
    Q_PROPERTY(QAbstractItemModel *messages READ messages CONSTANT)
    Q_PROPERTY(QString conversationSearchText READ conversationSearchText WRITE setConversationSearchText NOTIFY conversationSearchTextChanged)
    Q_PROPERTY(QString peerSearchText READ peerSearchText WRITE setPeerSearchText NOTIFY peerSearchTextChanged)
    Q_PROPERTY(QString messageSearchText READ messageSearchText WRITE setMessageSearchText NOTIFY messageSearchTextChanged)
    Q_PROPERTY(QString localName READ localName NOTIFY localProfileChanged)
    Q_PROPERTY(QString localInitial READ localInitial NOTIFY localProfileChanged)
    Q_PROPERTY(QUrl localAvatarUrl READ localAvatarUrl NOTIFY localProfileChanged)
    Q_PROPERTY(QString localAvatarColor READ localAvatarColor NOTIFY localProfileChanged)
    Q_PROPERTY(QString currentConversationId READ currentConversationId NOTIFY currentConversationIdChanged)
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
    /** @brief 销毁聊天管理器及其视图模型与桌面服务。 */
    ~LanChatManager() override;

    /**
     * @brief 返回统一单聊与群聊会话模型。
     * @return 会话列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *conversations();
    /**
     * @brief 返回联系人模型。
     * @return 联系人列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *peers();
    /**
     * @brief 返回当前会话消息模型。
     * @return 消息列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *messages();
    /**
     * @brief 返回会话搜索文本。
     * @return 未经裁剪的会话搜索文本。
     */
    [[nodiscard]] QString conversationSearchText() const;
    /**
     * @brief 更新会话搜索文本。
     * @param text 新搜索文本。
     */
    void setConversationSearchText(const QString &text);
    /**
     * @brief 返回联系人搜索文本。
     * @return 未经裁剪的联系人搜索文本。
     */
    [[nodiscard]] QString peerSearchText() const;
    /**
     * @brief 更新联系人搜索文本。
     * @param text 新搜索文本。
     */
    void setPeerSearchText(const QString &text);
    /**
     * @brief 返回当前会话的消息搜索文本。
     * @return 未经裁剪的消息搜索文本。
     */
    [[nodiscard]] QString messageSearchText() const;
    /**
     * @brief 更新当前会话的消息搜索文本。
     * @param text 新搜索文本。
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
     * @brief 返回本机头像图片 URL。
     * @return 已配置且存在时返回本地文件 URL，否则返回空 URL。
     */
    [[nodiscard]] QUrl localAvatarUrl() const;
    /**
     * @brief 返回本机头像背景色。
     * @return 可供 QML 使用的十六进制颜色。
     */
    [[nodiscard]] QString localAvatarColor() const;
    /**
     * @brief 返回当前选中的统一会话标识。
     * @return 当前会话标识；没有选择时为空。
     */
    [[nodiscard]] QString currentConversationId() const;
    /**
     * @brief 返回当前在线联系人数。
     * @return 在线联系人数。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有会话未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回聊天服务是否正在运行。
     * @return 服务正在运行时返回 @c true。
     */
    [[nodiscard]] bool running() const;
    /**
     * @brief 返回最近一次聊天协调错误。
     * @return 最近错误文本；没有错误时为空。
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
     * @brief 立即发起一次局域网联系人发现。
     * @return 发现请求成功发出时返回 @c true。
     */
    Q_INVOKABLE bool refreshPeers();
    /**
     * @brief 选择并加载直接会话或群聊。
     * @param conversationId 会话标识。
     * @return 会话存在且选择成功时返回 @c true。
     */
    Q_INVOKABLE bool selectConversation(const QString &conversationId);
    /**
     * @brief 将指定会话标记为已读。
     * @param conversationId 会话标识。
     * @return 操作成功时返回 @c true。
     */
    Q_INVOKABLE bool markConversationRead(const QString &conversationId);
    /**
     * @brief 返回指定会话的 QML 属性映射。
     * @param conversationId 会话标识。
     * @return 会话属性；会话不存在时为空映射。
     */
    Q_INVOKABLE QVariantMap conversationInfo(const QString &conversationId) const;
    /**
     * @brief 返回群聊成员的 QML 属性列表。
     * @param groupId 群会话标识。
     * @return 成员属性列表；群聊不存在时为空。
     */
    Q_INVOKABLE QVariantList groupMembers(const QString &groupId) const;
    /**
     * @brief 创建群聊。
     * @param name 群名称，长度为 1 到 64 个字符。
     * @param memberIds 联系人标识列表，数量为 2 到 31。
     * @return 创建成功时返回新群会话标识，失败时返回空字符串并发出失败信号。
     */
    Q_INVOKABLE QString createGroup(const QString &name,
                                    const QStringList &memberIds);
    /**
     * @brief 更新并保存本地资料。
     * @param displayName 新显示名称。
     * @param avatarUrl 本地头像 URL；空 URL 表示清除头像。
     * @param avatarColor 头像背景色。
     * @return 更新成功时返回 @c true。
     */
    Q_INVOKABLE bool updateLocalProfile(const QString &displayName,
                                        const QUrl &avatarUrl,
                                        const QString &avatarColor);
    /**
     * @brief 向指定直接会话或群聊发送文本消息。
     * @param conversationId 目标会话标识。
     * @param text 消息内容。
     * @return 消息被接受发送时返回 @c true。
     */
    Q_INVOKABLE bool sendMessage(const QString &conversationId,
                                 const QString &text);
    /**
     * @brief 向指定直接会话发送本地文件。
     * @param conversationId 目标直接会话标识。
     * @param fileUrl 本地文件 URL。
     * @return 文件传输请求被接受时返回 @c true；群聊始终返回 @c false。
     */
    Q_INVOKABLE bool sendFile(const QString &conversationId,
                              const QUrl &fileUrl);
    /**
     * @brief 向指定直接会话发送本地图片。
     * @param conversationId 目标直接会话标识。
     * @param imageUrl 本地图片 URL。
     * @return 图片传输请求被接受时返回 @c true。
     */
    Q_INVOKABLE bool sendImage(const QString &conversationId,
                               const QUrl &imageUrl);
    /**
     * @brief 向指定直接会话发送多个本地图片。
     * @param conversationId 目标直接会话标识。
     * @param imageUrls 本地图片 URL 列表。
     * @return 已接受发送的图片数量。
     */
    Q_INVOKABLE int sendImages(const QString &conversationId,
                               const QList<QUrl> &imageUrls);
    /**
     * @brief 向指定会话发送表情。
     * @param conversationId 目标会话标识。
     * @param packageId 表情包标识。
     * @param emojiId 表情标识。
     * @param fallbackText 回退文本。
     * @return 消息被接受发送时返回 @c true。
     */
    Q_INVOKABLE bool sendEmoji(const QString &conversationId,
                               const QString &packageId,
                               const QString &emojiId,
                               const QString &fallbackText);
    /**
     * @brief 向指定直接会话发送多个本地文件。
     * @param conversationId 目标直接会话标识。
     * @param fileUrls 本地文件 URL 列表。
     * @return 已接受发送的文件数量；群聊返回零。
     */
    Q_INVOKABLE int sendFiles(const QString &conversationId,
                              const QList<QUrl> &fileUrls);
    /**
     * @brief 返回剪贴板中可发送的本地图片 URL。
     * @return 本地图片 URL 列表；没有图片或保存失败时为空。
     */
    Q_INVOKABLE QList<QUrl> clipboardImageUrls();
    /**
     * @brief 取消直接会话中正在进行的文件传输。
     * @param conversationId 目标直接会话标识。
     * @param transferId 文件传输标识。
     * @return 成功取消匹配传输时返回 @c true。
     */
    Q_INVOKABLE bool cancelFileTransfer(const QString &conversationId,
                                        const QString &transferId);
    /**
     * @brief 接受直接会话中等待确认的文件传输请求。
     * @param conversationId 目标直接会话标识。
     * @param transferId 文件传输标识。
     * @return 已开始接收匹配文件时返回 @c true。
     */
    Q_INVOKABLE bool acceptFileTransfer(const QString &conversationId,
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
     * @brief 将本机设备标识复制到系统剪贴板。
     * @return 设备标识已复制时返回 @c true。
     */
    Q_INVOKABLE bool copyLocalDeviceId();
    /**
     * @brief 启用或禁用桌面通知。
     * @param enabled 是否允许显示通知。
     */
    Q_INVOKABLE void setNotificationsEnabled(bool enabled);

signals:
    /** @brief 会话搜索文本发生变化时发出。 */
    void conversationSearchTextChanged();
    /** @brief 联系人搜索文本发生变化时发出。 */
    void peerSearchTextChanged();
    /** @brief 消息搜索文本发生变化时发出。 */
    void messageSearchTextChanged();
    /** @brief 本地身份信息发生变化时发出。 */
    void localProfileChanged();
    /** @brief 当前统一会话标识发生变化时发出。 */
    void currentConversationIdChanged();
    /** @brief 在线联系人数发生变化时发出。 */
    void onlineCountChanged();
    /** @brief 会话未读消息总数发生变化时发出。 */
    void totalUnreadCountChanged();
    /** @brief 服务运行状态发生变化时发出。 */
    void runningChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();
    /**
     * @brief 发现此前未知的联系人时发出。
     * @param peerId 新联系人标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 已知联系人状态发生变化时发出。
     * @param peerId 已更新联系人标识。
     */
    void peerUpdated(const QString &peerId);
    /**
     * @brief 收到文本消息时发出。
     * @param conversationId 消息所属会话标识。
     * @param text 收到的消息文本。
     */
    void messageReceived(const QString &conversationId, const QString &text);
    /**
     * @brief 文本消息发送失败时发出。
     * @param conversationId 目标会话标识。
     * @param reason 失败原因。
     */
    void sendFailed(const QString &conversationId, const QString &reason);
    /**
     * @brief 接收文件成功保存后发出。
     * @param conversationId 文件所属直接会话标识。
     * @param filePath 接收文件的本地路径。
     */
    void fileReceived(const QString &conversationId, const QString &filePath);
    /**
     * @brief 文件传输失败时发出。
     * @param conversationId 文件所属直接会话标识。
     * @param reason 失败原因。
     */
    void fileTransferFailed(const QString &conversationId,
                            const QString &reason);
    /**
     * @brief 一般性服务操作失败时发出。
     * @param reason 失败原因。
     */
    void operationFailed(const QString &reason);
    /**
     * @brief 用户激活聊天通知时发出。
     * @param conversationId 通知关联的会话标识。
     */
    void notificationActivated(const QString &conversationId);

private:
    /** @brief 连接协调器、视图模型与桌面服务事件。 */
    void connectComponents();

    ChatCoordinator *m_coordinator = nullptr;
    std::unique_ptr<ConversationListViewModel> m_conversations;
    std::unique_ptr<PeerListViewModel> m_peers;
    std::unique_ptr<ConversationViewModel> m_conversation;
    std::unique_ptr<DesktopIntegration> m_desktop;
    static ChatCoordinator *s_coordinator;
};

#endif // LANCHATMANAGER_H
