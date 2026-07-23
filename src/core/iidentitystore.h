/**
 * @file iidentitystore.h
 * @brief 声明本地身份信息的持久化抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef IIDENTITYSTORE_H
#define IIDENTITYSTORE_H

#include "networktypes.h"

#include <QString>

class IIdentityStore
{
public:
    /** @brief 销毁身份信息存储对象。 */
    virtual ~IIdentityStore() = default;

    /**
     * @brief 加载已持久化的本地身份信息。
     * @param[out] identity 接收加载后的身份信息。
     * @param[out] errorMessage 加载失败时接收错误说明。
     * @return 身份信息加载成功时返回 @c true。
     */
    [[nodiscard]] virtual bool load(Network::LocalIdentity *identity,
                                    QString *errorMessage) = 0;
    /**
     * @brief 持久化本地身份信息。
     * @param identity 待保存的身份信息。
     * @param[out] errorMessage 保存失败时接收错误说明。
     * @return 身份信息保存成功时返回 @c true。
     */
    [[nodiscard]] virtual bool save(const Network::LocalIdentity &identity,
                                    QString *errorMessage) = 0;
};

#endif // IIDENTITYSTORE_H
