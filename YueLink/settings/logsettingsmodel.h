/**
 * @file logsettingsmodel.h
 * @brief 声明高级日志设置的 QML 状态与自动保存模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#ifndef LOGSETTINGSMODEL_H
#define LOGSETTINGSMODEL_H

#include "settingsmodelbase.h"

#include <QString>
#include <QtQml/qqmlregistration.h>

/**
 * @brief 管理诊断日志设置，并将修改自动保存到 log.json。
 *
 * 日志级别会立即应用；文件路径、源码位置和独立线程设置在下次启动时完整应用。
 */
class LogSettingsModel : public SettingsModelBase
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString level READ level NOTIFY levelChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(bool sourceLocationEnabled READ sourceLocationEnabled NOTIFY sourceLocationEnabledChanged)
    Q_PROPERTY(bool separateThreadEnabled READ separateThreadEnabled NOTIFY separateThreadEnabledChanged)
    Q_PROPERTY(bool restartRequired READ restartRequired NOTIFY restartRequiredChanged)

public:
    /**
     * @brief 构造日志设置模型并记录当前运行时已应用的基线配置。
     * @param[in] parent 可选的 QObject 父对象。
     */
    explicit LogSettingsModel(QObject *parent = nullptr);

    /**
     * @brief 返回当前日志级别。
     * @return 规范化后的日志级别名称。
     */
    [[nodiscard]] QString level() const;

    /**
     * @brief 返回当前日志文件路径。
     * @return 规范化后的绝对文件路径。
     */
    [[nodiscard]] QString filePath() const;

    /**
     * @brief 返回是否记录源码位置。
     * @return 启用源码位置记录时返回 @c true。
     */
    [[nodiscard]] bool sourceLocationEnabled() const;

    /**
     * @brief 返回是否使用独立日志线程。
     * @return 使用独立线程时返回 @c true。
     */
    [[nodiscard]] bool separateThreadEnabled() const;

    /**
     * @brief 返回是否存在需要重启后完整应用的日志设置。
     * @return 当前配置与启动时日志配置不同时返回 @c true。
     */
    [[nodiscard]] bool restartRequired() const;

    /**
     * @brief 更新并自动保存日志级别，同时立即应用运行时过滤级别。
     * @param[in] level 日志级别名称。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateLevel(const QString &level);

    /**
     * @brief 更新并自动保存日志文件路径。
     * @param[in] filePath 日志文件路径；相对路径由配置层解析。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateFilePath(const QString &filePath);

    /**
     * @brief 更新并自动保存源码位置记录开关。
     * @param[in] enabled 是否记录源码文件路径和行号。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateSourceLocationEnabled(bool enabled);

    /**
     * @brief 更新并自动保存独立日志线程开关。
     * @param[in] enabled 是否通过独立线程写入日志。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateSeparateThreadEnabled(bool enabled);

signals:
    /** @brief 日志级别发生变化时发出。 */
    void levelChanged();
    /** @brief 日志文件路径发生变化时发出。 */
    void filePathChanged();
    /** @brief 源码位置记录开关发生变化时发出。 */
    void sourceLocationEnabledChanged();
    /** @brief 独立日志线程开关发生变化时发出。 */
    void separateThreadEnabledChanged();
    /** @brief 重启生效状态发生变化时发出。 */
    void restartRequiredChanged();
    /** @brief 任一日志设置发生变化时发出。 */
    void settingsChanged();

private:
    /** @brief 从配置管理器刷新规范化后的日志设置缓存。 */
    void refreshFromConfig();

    /** @brief 根据启动基线重新计算是否需要重启。 */
    void updateRestartRequired();

    QString m_level;                      ///< 当前日志级别。
    QString m_filePath;                   ///< 当前日志文件路径。
    bool m_sourceLocationEnabled = false; ///< 当前源码位置记录开关。
    bool m_separateThreadEnabled = true;  ///< 当前独立线程写入开关。
    QString m_appliedFilePath;            ///< 启动时已应用的日志文件路径。
    bool m_appliedSourceLocation = false; ///< 启动时已应用的源码位置记录开关。
    bool m_appliedSeparateThread = true;  ///< 启动时已应用的独立线程写入开关。
    bool m_baselineInitialized = false;   ///< 是否已经记录启动时运行配置。
    bool m_restartRequired = false;       ///< 是否存在尚未应用的启动级设置。
};

#endif // LOGSETTINGSMODEL_H
