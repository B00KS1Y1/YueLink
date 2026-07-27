/**
 * @file qsettingsidentitystore.h
 * @brief 声明基于 system/configs/identity.ini 的身份信息存储实现。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef QSETTINGSIDENTITYSTORE_H
#define QSETTINGSIDENTITYSTORE_H

#include "core/iidentitystore.h"

class QSettingsIdentityStore final : public IIdentityStore
{
public:
    /**
     * @brief 从 system/configs/identity.ini 加载本地身份信息。
     * @param[out] identity 接收加载后的身份信息。
     * @param[out] errorMessage 加载失败时接收错误说明。
     * @return 身份信息加载成功时返回 @c true。
     */
    [[nodiscard]] bool load(Network::LocalIdentity *identity,
                            QString *errorMessage) override;
    /**
     * @brief 将本地身份信息保存到 system/configs/identity.ini。
     * @param identity 待保存的身份信息。
     * @param[out] errorMessage 保存失败时接收错误说明。
     * @return 身份信息保存成功时返回 @c true。
     */
    [[nodiscard]] bool save(const Network::LocalIdentity &identity,
                            QString *errorMessage) override;
};

#endif // QSETTINGSIDENTITYSTORE_H
