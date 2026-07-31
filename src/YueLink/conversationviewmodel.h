/**
 * @file conversationviewmodel.h
 * @brief 声明当前会话与消息列表的 QML 视图模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-01
 */

#ifndef CONVERSATIONVIEWMODEL_H
#define CONVERSATIONVIEWMODEL_H

#include "domain/chattypes.h"
#include "YueLink/chatmessagemodel.h"

#include <QDateTime>
#include <QObject>
#include <QSortFilterProxyModel>

class ChatCoordinator;

class ConversationViewModel final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造当前会话视图模型。
     * @param coordinator 非拥有的聊天协调器；必须覆盖本对象生命周期。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ConversationViewModel(ChatCoordinator *coordinator,
                                   QObject *parent = nullptr);
    /** @brief 销毁当前会话视图模型。 */
    ~ConversationViewModel() override;

    /**
     * @brief 返回供 QML 使用的消息筛选模型。
     * @return 当前会话消息筛选模型指针。
     */
    [[nodiscard]] QAbstractItemModel *model();
    /**
     * @brief 返回消息搜索文本。
     * @return 未经裁剪的搜索文本。
     */
    [[nodiscard]] QString searchText() const;
    /**
     * @brief 更新消息搜索文本。
     * @param text 新搜索文本；匹配时忽略大小写和首尾空白。
     */
    void setSearchText(const QString &text);
    /**
     * @brief 返回当前选中会话的节点标识。
     * @return 当前节点标识；没有选择时返回空字符串。
     */
    [[nodiscard]] QString currentPeerId() const;
    /**
     * @brief 返回本地显示名称首字符。
     * @return 用于头像显示的首字符。
     */
    [[nodiscard]] QString localInitial() const;
    /**
     * @brief 选择并加载指定节点的会话。
     * @param peerId 会话对应的节点标识。
     * @return 节点存在且选择成功时返回 @c true。
     */
    [[nodiscard]] bool selectPeer(const QString &peerId);

signals:
    /** @brief 消息搜索文本发生变化时发出。 */
    void searchTextChanged();
    /** @brief 当前会话节点标识发生变化时发出。 */
    void currentPeerIdChanged();

private:
    /**
     * @brief 将指定会话完整同步到消息模型。
     * @param peerId 会话对应的节点标识。
     */
    void synchronize(const QString &peerId);
    /**
     * @brief 处理新增领域消息并增量更新模型。
     * @param message 新增的领域消息。
     */
    void handleMessageAdded(const Domain::Message &message);
    /**
     * @brief 处理消息投递状态更新。
     * @param peerId 会话对应的节点标识。
     * @param messageId 消息标识。
     * @param state 新的投递状态。
     */
    void handleMessageStateChanged(const QString &peerId,
                                   const QString &messageId,
                                   Domain::DeliveryState state);
    /**
     * @brief 处理文件传输消息更新。
     * @param peerId 会话对应的节点标识。
     * @param messageId 文件传输消息标识。
     * @param progress 传输进度。
     * @param state 新的投递状态。
     * @param filePath 可用时为本地文件路径。
     */
    void handleFileTransferChanged(const QString &peerId,
                                   const QString &messageId,
                                   qreal progress,
                                   Domain::DeliveryState state,
                                   const QString &filePath);
    /**
     * @brief 将领域消息转换为视图消息。
     * @param message 待转换的领域消息。
     * @return 可供消息模型使用的视图消息。
     */
    [[nodiscard]] ChatMessageModel::Message toViewMessage(const Domain::Message &message) const;
    /**
     * @brief 将文件字节数格式化为可读文本。
     * @param bytes 文件字节数。
     * @param fallback 字节数不可用时使用的旧文本。
     * @return 格式化后的文件大小文本。
     */
    [[nodiscard]] static QString displayFileSize(qint64 bytes,
                                                 const QString &fallback);
    /**
     * @brief 将时间戳格式化为消息显示文本。
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
     * @brief 为节点标识生成稳定头像颜色。
     * @param peerId 节点标识。
     * @return 颜色表中的十六进制颜色字符串。
     */
    [[nodiscard]] static QString colorForId(const QString &peerId);

    ChatCoordinator *m_coordinator = nullptr;
    ChatMessageModel m_model;
    QSortFilterProxyModel m_filterModel;
    QString m_searchText;
    QString m_currentPeerId;
};

#endif // CONVERSATIONVIEWMODEL_H
