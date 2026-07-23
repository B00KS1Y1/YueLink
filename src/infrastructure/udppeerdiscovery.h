/**
 * @file udppeerdiscovery.h
 * @brief 声明基于 UDP 的局域网节点发现服务。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef UDPPEERDISCOVERY_H
#define UDPPEERDISCOVERY_H

#include "core/ipeerdiscovery.h"

#include <QHash>
#include <QTimer>
#include <QUdpSocket>

class UdpPeerDiscovery final : public IPeerDiscovery
{
    Q_OBJECT

public:
    /**
     * @brief 构造 UDP 节点发现服务。
     * @param parent 可选的 QObject 父对象。
     */
    explicit UdpPeerDiscovery(QObject *parent = nullptr);

    /**
     * @brief 启动局域网身份广播与节点发现。
     * @param identity 待广播的本地身份。
     * @param tcpPort 接收聊天连接的 TCP 端口。
     * @return 节点发现启动成功时返回 @c true。
     */
    [[nodiscard]] bool start(const Network::LocalIdentity &identity, quint16 tcpPort) override;
    /** @brief 停止节点发现并发送离线通知。 */
    void stop() override;
    /**
     * @brief 更新后续广播中携带的本地身份。
     * @param identity 新的本地身份。
     */
    void updateIdentity(const Network::LocalIdentity &identity) override;
    /** @brief 立即发送一次在线广播。 */
    void announce() override;
    /**
     * @brief 刷新指定节点的最近活动时间。
     * @param peerId 待刷新的节点标识。
     */
    void recordPeerActivity(const QString &peerId) override;
    /**
     * @brief 返回节点发现服务是否正在运行。
     * @return 节点发现服务正在运行时返回 @c true。
     */
    [[nodiscard]] bool isRunning() const override;
    /**
     * @brief 返回最近一次节点发现错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] QString lastError() const override;

private:
    static constexpr quint16 DiscoveryPort = 45454;
    static constexpr qint64 PeerTimeoutMs = 7000;

    /** @brief 读取并处理所有等待中的 UDP 数据报。 */
    void readPendingDatagrams();
    /**
     * @brief 发送指定类型的节点状态广播。
     * @param type 广播类型，例如在线或离线。
     */
    void sendPresence(const QString &type);
    /** @brief 将超过活动超时时间的节点标记为离线。 */
    void expirePeers();
    /**
     * @brief 更新最近一次节点发现错误。
     * @param error 新的错误文本；传入空字符串表示清除错误。
     */
    void setLastError(const QString &error);

    QUdpSocket m_socket;
    QTimer m_heartbeatTimer;
    QHash<QString, qint64> m_lastSeenByPeer;
    Network::LocalIdentity m_identity;
    QString m_lastError;
    quint16 m_tcpPort = 0;
    bool m_running = false;
};

#endif // UDPPEERDISCOVERY_H
