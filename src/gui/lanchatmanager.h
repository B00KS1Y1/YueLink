/**
 * @file lanchatmanager.h
 * @brief 声明共享聊天服务的 QML 适配器。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef LANCHATMANAGER_H
#define LANCHATMANAGER_H

#include "chatmessagemodel.h"
#include "peerlistmodel.h"

#include <QAbstractItemModel>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class ChatService;
class IFileLauncher;
class INotificationService;
class QJSEngine;
class QQmlEngine;

namespace Domain
{
struct Message;
}

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
     * @brief 设置 QML 单例工厂使用的共享聊天服务。
     * @param service 由应用程序生命周期管理的聊天服务。
     */
    static void setService(ChatService *service);
    /**
     * @brief 为 QML 引擎创建聊天管理器单例。
     * @param qmlEngine 请求单例的 QML 引擎。
     * @param jsEngine 请求单例的 JavaScript 引擎。
     * @return 使用已注入聊天服务创建的管理器实例。
     */
    static LanChatManager *create(QQmlEngine *qmlEngine,
                                  QJSEngine *jsEngine);

    /**
     * @brief 使用默认桌面适配器构造 QML 聊天管理器。
     * @param service 共享聊天服务。
     * @param parent 可选的 QObject 父对象。
     */
    explicit LanChatManager(ChatService *service, QObject *parent = nullptr);
    /**
     * @brief 使用指定桌面适配器构造 QML 聊天管理器。
     * @param service 共享聊天服务。
     * @param fileLauncher 由管理器接管所有权的文件启动器。
     * @param notificationService 由管理器接管所有权的通知服务。
     * @param parent 可选的 QObject 父对象。
     */
    LanChatManager(ChatService *service,
                   std::unique_ptr<IFileLauncher> fileLauncher,
                   std::unique_ptr<INotificationService> notificationService,
                   QObject *parent = nullptr);
    /** @brief 销毁 QML 聊天管理器及其桌面适配器。 */
    ~LanChatManager() override;

    /**
     * @brief 返回供 QML 使用的节点列表模型。
     * @return 节点列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *peers();
    /**
     * @brief 返回供 QML 使用的消息列表模型。
     * @return 当前会话消息列表模型指针。
     */
    [[nodiscard]] QAbstractItemModel *messages();
    /**
     * @brief 返回好友列表当前使用的搜索文本。
     * @return 未经裁剪的好友搜索文本。
     */
    [[nodiscard]] QString peerSearchText() const;
    /**
     * @brief 更新好友列表搜索文本。
     * @param text 新的搜索文本；匹配时忽略大小写和首尾空白。
     */
    void setPeerSearchText(const QString &text);
    /**
     * @brief 返回当前会话使用的消息搜索文本。
     * @return 未经裁剪的消息搜索文本。
     */
    [[nodiscard]] QString messageSearchText() const;
    /**
     * @brief 更新当前会话的消息搜索文本。
     * @param text 新的搜索文本；匹配时忽略大小写和首尾空白。
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
     * @return 当前节点标识；未选择会话时返回空字符串。
     */
    [[nodiscard]] QString currentPeerId() const;
    /**
     * @brief 返回当前在线的节点数量。
     * @return 在线节点数量。
     */
    [[nodiscard]] int onlineCount() const;
    /**
     * @brief 返回所有节点的未读消息总数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;
    /**
     * @brief 返回共享聊天服务是否正在运行。
     * @return 服务正在运行时返回 @c true。
     */
    [[nodiscard]] bool running() const;
    /**
     * @brief 返回最近一次聊天服务错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] QString lastError() const;

    /**
     * @brief 启动共享聊天服务。
     * @return 服务启动成功时返回 @c true。
     */
    Q_INVOKABLE bool start();
    /** @brief 停止共享聊天服务。 */
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
     * @return 已接受发送的文件传输数量。
     */
    Q_INVOKABLE int sendFiles(const QString &peerId,
                              const QList<QUrl> &fileUrls);
    /**
     * @brief 取消正在进行的文件传输。
     * @param peerId 远端节点标识。
     * @param transferId 文件传输标识。
     * @return 成功取消匹配的传输时返回 @c true。
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
    /** @brief 在线节点数量发生变化时发出。 */
    void onlineCountChanged();
    /** @brief 未读消息总数发生变化时发出。 */
    void totalUnreadCountChanged();
    /** @brief 服务运行状态发生变化时发出。 */
    void runningChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();
    /**
     * @brief 发现此前未知的节点时发出。
     * @param peerId 已发现节点的标识。
     */
    void peerDiscovered(const QString &peerId);
    /**
     * @brief 已知节点的信息发生变化时发出。
     * @param peerId 已更新节点的标识。
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
    /** @brief 连接共享聊天服务与桌面适配器事件。 */
    void connectService();
    /** @brief 将服务中的节点状态同步到 QML 模型。 */
    void synchronizePeers();
    /**
     * @brief 将指定会话同步到 QML 消息模型。
     * @param peerId 会话对应的节点标识。
     */
    void synchronizeConversation(const QString &peerId);
    /**
     * @brief 处理收到的文本消息及已读、通知逻辑。
     * @param peerId 发送方节点标识。
     * @param text 收到的消息文本。
     */
    void handleIncomingMessage(const QString &peerId, const QString &text);
    /**
     * @brief 在允许时显示收到消息的桌面通知。
     * @param peerId 发送方节点标识。
     * @param message 通知预览文本。
     */
    void showIncomingNotification(const QString &peerId,
                                  const QString &message);
    /**
     * @brief 将领域消息转换为 QML 视图消息。
     * @param message 待转换的领域消息。
     * @return 可供消息模型使用的视图消息。
     */
    [[nodiscard]] ChatMessageModel::Message toViewMessage(
        const Domain::Message &message) const;
    /**
     * @brief 将文件字节数格式化为可读文本。
     * @param bytes 文件字节数。
     * @param fallback 字节数不可用时使用的旧文本。
     * @return 格式化后的文件大小文本。
     */
    [[nodiscard]] static QString displayFileSize(qint64 bytes,
                                                 const QString &fallback);
    /**
     * @brief 将时间戳格式化为会话显示文本。
     * @param timestamp 待格式化的时间戳。
     * @return 本地化的简短时间文本。
     */
    [[nodiscard]] static QString displayTime(const QDateTime &timestamp);
    /**
     * @brief 返回名称首字符的大写形式。
     * @param name 待处理的显示名称。
     * @return 名称首字符；名称为空时返回占位符。
     */
    [[nodiscard]] static QString initialForName(const QString &name);
    /**
     * @brief 为节点标识生成稳定的头像颜色。
     * @param peerId 节点标识。
     * @return 颜色表中的十六进制颜色字符串。
     */
    [[nodiscard]] static QString colorForId(const QString &peerId);

    ChatService *m_service = nullptr;
    std::unique_ptr<IFileLauncher> m_fileLauncher;
    std::unique_ptr<INotificationService> m_notificationService;
    PeerListModel m_peerModel;
    QSortFilterProxyModel m_peerFilterModel;
    ChatMessageModel m_messageModel;
    QSortFilterProxyModel m_messageFilterModel;
    QString m_peerSearchText;
    QString m_messageSearchText;
    QString m_currentPeerId;
    static ChatService *s_service;
};

#endif // LANCHATMANAGER_H
