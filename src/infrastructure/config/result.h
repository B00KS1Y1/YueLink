/**
 * @file result.h
 * @brief 定义配置操作返回的结果类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CONFIG_RESULT_H
#define CONFIG_RESULT_H

#include <QString>

#include <utility>

namespace Config
{

struct Result
{
    QString errorMessage;

    /**
     * @brief 判断配置操作是否成功。
     * @return 未记录错误时返回 @c true。
     */
    [[nodiscard]] bool ok() const noexcept
    {
        return errorMessage.isEmpty();
    }

    /**
     * @brief 将结果转换为布尔值。
     * @return 操作成功时返回 @c true。
     */
    explicit operator bool() const noexcept
    {
        return ok();
    }

    /**
     * @brief 创建失败的配置操作结果。
     * @param errorMessage 错误说明。
     * @return 包含指定错误说明的失败结果。
     */
    static Result failure(QString errorMessage)
    {
        Result result;
        result.errorMessage = std::move(errorMessage);
        return result;
    }
};

} // namespace Config

#endif // CONFIG_RESULT_H
