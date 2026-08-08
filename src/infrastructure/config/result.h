/**
 * @file result.h
 * @brief 定义配置操作返回的结果类型。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CONFIG_RESULT_H
#define CONFIG_RESULT_H

#include <QList>
#include <QString>
#include <QtGlobal>

#include <utility>

namespace Config
{

enum class ErrorCode
{
    None,
    OperationFailed,
    NotInitialized,
    InvalidPath,
    OpenFailed,
    ReadFailed,
    ParseFailed,
    UnsupportedVersion,
    MigrationFailed,
    ValidationFailed,
    SerializationFailed,
    WriteFailed,
    CommitFailed,
    MutationFailed,
    Conflict
};

struct Issue
{
    QString fieldPath;
    QString message;
};

struct Result
{
    ErrorCode errorCode = ErrorCode::None;
    QString errorMessage;
    QString filePath;
    QList<Issue> issues;
    bool changed = false;
    quint64 revision = 0;

    /**
     * @brief 判断配置操作是否成功。
     * @return 错误码为 @c ErrorCode::None 时返回 @c true。
     */
    [[nodiscard]] bool ok() const noexcept
    {
        return errorCode == ErrorCode::None;
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
     * @param[in] errorMessage 错误说明。
     * @return 包含指定错误说明的失败结果。
     */
    [[nodiscard]] static Result failure(QString errorMessage)
    {
        return failure(ErrorCode::OperationFailed, std::move(errorMessage));
    }

    /**
     * @brief 创建带错误码和文件路径的失败结果。
     * @param[in] errorCode 错误分类。
     * @param[in] errorMessage 错误说明。
     * @param[in] filePath 发生错误的配置文件路径。
     * @return 包含指定错误信息的失败结果。
     */
    [[nodiscard]] static Result failure(ErrorCode errorCode, QString errorMessage, QString filePath = {})
    {
        Result result;
        result.errorCode = errorCode;
        result.errorMessage = std::move(errorMessage);
        result.filePath = std::move(filePath);
        return result;
    }

    /**
     * @brief 创建配置校验失败结果。
     * @param[in] issues 配置字段问题列表。
     * @param[in] filePath 对应的配置文件路径。
     * @return 包含字段问题的失败结果。
     */
    [[nodiscard]] static Result validationFailure(QList<Issue> issues, QString filePath = {})
    {
        Result result;
        result.errorCode = ErrorCode::ValidationFailed;
        result.filePath = std::move(filePath);
        result.issues = std::move(issues);
        result.errorMessage = result.issues.isEmpty()
                                  ? QStringLiteral("配置校验失败。")
                                  : QStringLiteral("配置字段“%1”无效：%2").arg(result.issues.constFirst().fieldPath, result.issues.constFirst().message);
        return result;
    }
};

} // namespace Config

#endif // CONFIG_RESULT_H
