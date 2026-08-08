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

/**
 * @brief 配置操作的机器可读错误分类。
 */
enum class ErrorCode
{
    None,                 ///< 操作成功。
    OperationFailed,      ///< 未细分的通用操作失败。
    NotInitialized,       ///< 配置存储尚未初始化。
    InvalidPath,          ///< 配置路径无效或目录创建失败。
    OpenFailed,           ///< 配置文件无法打开。
    ReadFailed,           ///< 配置文件读取失败。
    ParseFailed,          ///< JSON 语法、根节点或字段类型无法解析。
    UnsupportedVersion,   ///< 文件 schema 版本无效或高于程序支持版本。
    MigrationFailed,      ///< 旧 schema 迁移到当前版本失败。
    ValidationFailed,     ///< 配置字段不满足业务约束。
    SerializationFailed,  ///< 配置与 JSON 之间的转换或比较失败。
    WriteFailed,          ///< 临时配置文件写入不完整。
    CommitFailed,         ///< 原子替换原配置文件失败。
    MutationFailed,       ///< @c update 修改器抛出异常。
    Conflict              ///< 修改期间配置已被其他线程发布，当前候选值未提交。
};

/**
 * @brief 描述单个配置字段的校验问题。
 */
struct Issue
{
    /// 使用 JSON 点分路径表示的问题字段，例如 @c sqlite.pool_size。
    QString fieldPath;
    /// 面向用户或日志输出的中文问题说明。
    QString message;
};

/**
 * @brief 汇总一次配置操作的状态、诊断信息和发布元数据。
 */
struct Result
{
    /// 机器可读错误码；@c None 表示成功。
    ErrorCode errorCode = ErrorCode::None;
    /// 面向用户或日志输出的错误摘要。
    QString errorMessage;
    /// 本次操作涉及的配置文件绝对路径。
    QString filePath;
    /// 校验失败时的全部字段问题。
    QList<Issue> issues;
    /// 是否成功发布了不同于原内存值的新配置。
    bool changed = false;
    /// 成功操作完成后的类型内修订号；失败结果通常保持为 0，只有发布新值时修订号才递增。
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
     * @return 包含全部字段问题，并以首个问题生成错误摘要的失败结果。
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
