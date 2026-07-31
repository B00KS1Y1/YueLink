/**
 * @file ipeerdiscovery.h
 * @brief 声明局域网节点发现抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef IPEERDISCOVERY_H
#define IPEERDISCOVERY_H

#include "networktypes.h"

#include <QObject>

class IPeerDiscovery : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造节点发现服务。
     * @param parent 可选的 QObject 父对象。
     */
    explicit IPeerDiscovery(QObject *parent = nullptr)
    : QObject(parent)
    {
    }

    /** @brief 销毁节点发现服务。 */
    ~IPeerDiscovery() override = default;

    /**
     * @brief 启动局域网身份广播与节点发现。
     * @param identity 待广播的本地身份。
     * @param tcpPort 接收聊天连接的 TCP 端口。
     * @return 节点发现启动成功时返回 @c true。
     */
    [[nodiscard]] virtual bool start(const Network::LocalIdentity &identity, quint16 tcpPort) = 0;
    /** @brief 停止节点发现，并在条件允许时发送离线通知。 */
    virtual void stop() = 0;
    /**
     * @brief 更新后续广播中携带的本地身份。
     * @param identity 新的本地身份。
     */
    virtual void updateIdentity(const Network::LocalIdentity &identity) = 0;
    /** @brief 立即发送一次在线广播。 */
    virtual void announce() = 0;
    /** @brief 广播一次发现请求，并请求在线节点立即单播回应。*/
    virtual void probe() = 0;
    /**
     * @brief 刷新指定节点的最近活动时间。
     * @param peerId 待刷新的节点标识。
     */
    virtual void recordPeerActivity(const QString &peerId) = 0;
    /**
     * @brief 返回节点发现服务是否正在运行。
     * @return 节点发现服务正在运行时返回 @c true。
     */
    [[nodiscard]] virtual bool isRunning() const = 0;
    /**
     * @brief 返回最近一次节点发现错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] virtual QString lastError() const = 0;

signals:
    /**
     * @brief 发现节点或节点信息刷新时发出。
     * @param peer 已发现或刷新的节点信息。
     */
    void peerFound(const Network::PeerEndpoint &peer);
    /**
     * @brief 节点离线或超时时发出。
     * @param peerId 离线节点的标识。
     */
    void peerLost(const QString &peerId);
    /**
     * @brief 节点发现过程中发生运行错误时发出。
     * @param message 错误说明。
     */
    void errorOccurred(const QString &message);
};

#endif // IPEERDISCOVERY_H
