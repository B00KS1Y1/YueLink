/**
 * @file configbase.h
 * @brief 定义配置值类型共享的 CRTP 基类。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-08
 */

#ifndef CONFIGBASE_H
#define CONFIGBASE_H

#include "result.h"

#include <QList>

#include <nlohmann/json.hpp>

namespace Config
{

/**
 * @brief 为配置值类型提供默认策略和 schema 版本。
 * @tparam Derived 继承该基类的具体配置值类型。
 *
 * 各策略均为静态扩展点而非虚函数。派生类型只需声明同名静态函数即可覆盖默认行为，
 * 从而让 @c ConfigStore 在编译期选择相应策略。
 */
template <typename Derived> struct ConfigBase
{
    /// 当前持久化 JSON 的 schema 版本；派生类型可用同名常量覆盖。
    static constexpr int SchemaVersion = 1;

    /**
     * @brief 使用派生类型的成员初始值创建默认配置。
     * @return 使用派生类型成员默认值构造的配置。
     */
    [[nodiscard]] static Derived defaults()
    {
        return {};
    }

    /**
     * @brief 将派生配置规范化为持久化和运行时使用的标准形式。
     * @param[in,out] config 待规范化的配置。
     * @note 默认实现不修改配置。
     */
    static void normalize(Derived &config)
    {
        Q_UNUSED(config)
    }

    /**
     * @brief 校验派生配置是否满足约束。
     * @param[in] config 待校验的配置。
     * @return 校验问题列表；空列表表示校验通过。
     * @note 默认实现始终通过校验。
     */
    [[nodiscard]] static QList<Issue> validate(const Derived &config)
    {
        Q_UNUSED(config)
        return {};
    }

    /**
     * @brief 将旧版本 JSON 文档迁移到派生配置的当前版本。
     * @param[in,out] json 待迁移的 JSON 文档。
     * @param[in] sourceVersion 文档原始版本。
     * @param[in] targetVersion 目标版本。
     * @return 迁移结果。
     * @note 默认实现不修改 JSON 并报告成功；提高 @c SchemaVersion 的派生类型应提供同名迁移函数。
     */
    [[nodiscard]] static Result migrate(nlohmann::json &json, int sourceVersion, int targetVersion)
    {
        Q_UNUSED(json)
        Q_UNUSED(sourceVersion)
        Q_UNUSED(targetVersion)
        return {};
    }
};

} // namespace Config

#endif // CONFIGBASE_H
