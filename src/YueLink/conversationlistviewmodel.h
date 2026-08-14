/**
 * @file conversationlistviewmodel.h
 * @brief 声明统一单聊与群聊会话侧栏视图模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef CONVERSATIONLISTVIEWMODEL_H
#define CONVERSATIONLISTVIEWMODEL_H

#include "YueLink/sidebaritemmodel.h"

#include <QDateTime>
#include <QObject>
#include <QSortFilterProxyModel>

class ChatCoordinator;

class ConversationListViewModel final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造统一会话列表视图模型。
     * @param coordinator 非拥有的聊天协调器。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ConversationListViewModel(ChatCoordinator *coordinator, QObject *parent = nullptr);
    /** @brief 销毁会话列表视图模型。 */
    ~ConversationListViewModel() override;
    /**
     * @brief 返回供 QML 使用的筛选和排序模型。
     * @return 代理模型指针。
     */
    [[nodiscard]] QAbstractItemModel *model();
    /**
     * @brief 返回会话搜索文本。
     * @return 当前搜索文本。
     */
    [[nodiscard]] QString searchText() const;
    /**
     * @brief 更新会话搜索文本。
     * @param text 新搜索文本。
     */
    void setSearchText(const QString &text);
    /**
     * @brief 返回指定会话的 QML 属性。
     * @param conversationId 会话标识。
     * @return 会话属性；不存在时为空。
     */
    [[nodiscard]] QVariantMap conversationInfo(const QString &conversationId) const;
    /**
     * @brief 返回全部会话未读数。
     * @return 未读消息总数。
     */
    [[nodiscard]] int totalUnreadCount() const;

signals:
    /** @brief 搜索文本发生变化时发出。 */
    void searchTextChanged();
    /** @brief 未读总数发生变化时发出。 */
    void totalUnreadCountChanged();
    /** @brief 会话列表内容发生变化时发出。 */
    void conversationsChanged();

private:
    /** @brief 从协调器同步全部会话及在线成员统计。 */
    void synchronize();
    /**
     * @brief 格式化侧栏时间。
     * @param timestamp UTC 或本地时间戳。
     * @return 本地化短时间文本。
     */
    [[nodiscard]] static QString displayTime(const QDateTime &timestamp);
    /**
     * @brief 返回显示名称首字符。
     * @param name 显示名称。
     * @return 大写首字符或问号。
     */
    [[nodiscard]] static QString initialForName(const QString &name);
    /**
     * @brief 为标识生成稳定头像颜色。
     * @param id 稳定标识。
     * @return 十六进制颜色。
     */
    [[nodiscard]] static QString colorForId(const QString &id);

    ChatCoordinator *m_coordinator = nullptr;
    SidebarItemModel m_model;
    QSortFilterProxyModel m_filterModel;
    QString m_searchText;
};

#endif // CONVERSATIONLISTVIEWMODEL_H
