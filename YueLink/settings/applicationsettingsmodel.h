/**
 * @file applicationsettingsmodel.h
 * @brief 声明通用设置的 QML 状态与自动保存模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#ifndef APPLICATIONSETTINGSMODEL_H
#define APPLICATIONSETTINGSMODEL_H

#include "settingsmodelbase.h"

#include <QString>
#include <QtQml/qqmlregistration.h>

/**
 * @brief 管理开机自启动、通知和下载目录，并持久化对应设置。
 */
class ApplicationSettingsModel : public SettingsModelBase
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(bool autoStartEnabled READ autoStartEnabled NOTIFY autoStartEnabledChanged)
    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled NOTIFY notificationsEnabledChanged)
    Q_PROPERTY(QString downloadDirectory READ downloadDirectory NOTIFY downloadDirectoryChanged)

public:
    /**
     * @brief 构造通用设置模型并载入当前应用配置。
     * @param[in] parent 可选的 QObject 父对象。
     */
    explicit ApplicationSettingsModel(QObject *parent = nullptr);

    /**
     * @brief 返回应用是否已注册为登录系统后自动启动。
     * @return 系统登录项已启用时返回 @c true。
     */
    [[nodiscard]] bool autoStartEnabled() const;

    /**
     * @brief 返回是否启用桌面通知。
     * @return 启用桌面通知时返回 @c true。
     */
    [[nodiscard]] bool notificationsEnabled() const;

    /**
     * @brief 返回接收文件的下载目录。
     * @return 规范化后的绝对目录路径。
     */
    [[nodiscard]] QString downloadDirectory() const;

    /**
     * @brief 更新系统开机自启动状态。
     * @param[in] enabled 是否在登录系统后自动启动应用。
     * @return 系统状态更新成功或已经处于目标状态时返回 @c true。
     */
    Q_INVOKABLE bool updateAutoStartEnabled(bool enabled);

    /**
     * @brief 更新并自动保存桌面通知设置。
     * @param[in] enabled 是否启用桌面通知。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateNotificationsEnabled(bool enabled);

    /**
     * @brief 更新并自动保存接收文件目录。
     * @param[in] directory 接收文件目录；配置层负责规范化路径。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateDownloadDirectory(const QString &directory);

signals:
    /** @brief 系统开机自启动状态发生变化时发出。 */
    void autoStartEnabledChanged();
    /** @brief 桌面通知设置发生变化时发出。 */
    void notificationsEnabledChanged();
    /** @brief 下载目录发生变化时发出。 */
    void downloadDirectoryChanged();
    /** @brief 任一通用设置发生变化时发出。 */
    void settingsChanged();

private:
    /** @brief 从配置管理器刷新规范化后的通用设置缓存。 */
    void refreshFromConfig();

    /**
     * @brief 更新缓存的系统开机自启动状态并发送变更信号。
     * @param[in] enabled 系统当前的开机自启动状态。
     */
    void setAutoStartEnabled(bool enabled);

    bool m_autoStartEnabled = false;    ///< 系统当前的开机自启动状态。
    bool m_notificationsEnabled = true; ///< 当前桌面通知开关。
    QString m_downloadDirectory;        ///< 当前接收文件目录。
};

#endif // APPLICATIONSETTINGSMODEL_H
