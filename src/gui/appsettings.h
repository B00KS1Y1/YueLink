/**
 * @file appsettings.h
 * @brief 声明面向 QML 的应用设置单例。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class AppSettings : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AppSettings)
    QML_SINGLETON
    Q_PROPERTY(QString themeMode READ themeMode NOTIFY settingsChanged)
    Q_PROPERTY(QString primaryColor READ primaryColor NOTIFY settingsChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(QString logLevel READ logLevel NOTIFY settingsChanged)
    Q_PROPERTY(QString logFilePath READ logFilePath NOTIFY settingsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    /**
     * @brief 构造 QML 应用设置单例。
     * @param parent 可选的 QObject 父对象。
     */
    explicit AppSettings(QObject *parent = nullptr);

    /**
     * @brief 返回当前主题模式。
     * @return 规范化后的主题模式名称。
     */
    [[nodiscard]] QString themeMode() const;
    /**
     * @brief 返回当前主色。
     * @return 十六进制主色字符串。
     */
    [[nodiscard]] QString primaryColor() const;
    /**
     * @brief 返回是否启用界面动画。
     * @return 启用界面动画时返回 @c true。
     */
    [[nodiscard]] bool animationsEnabled() const;
    /**
     * @brief 返回是否启用桌面通知。
     * @return 启用桌面通知时返回 @c true。
     */
    [[nodiscard]] bool notificationsEnabled() const;
    /**
     * @brief 返回当前日志级别。
     * @return 规范化后的日志级别名称。
     */
    [[nodiscard]] QString logLevel() const;
    /**
     * @brief 返回当前配置的日志文件路径。
     * @return 日志文件的绝对路径。
     */
    [[nodiscard]] QString logFilePath() const;
    /**
     * @brief 返回最近一次设置保存错误。
     * @return 最近错误文本；没有错误时返回空字符串。
     */
    [[nodiscard]] QString lastError() const;

    /**
     * @brief 校验并保存应用设置。
     * @param themeMode 主题模式。
     * @param primaryColor 主色值。
     * @param animationsEnabled 是否启用界面动画。
     * @param notificationsEnabled 是否启用桌面通知。
     * @param logLevel 日志级别。
     * @param logFilePath 日志文件的绝对路径。
     * @return 设置保存成功时返回 @c true。
     */
    Q_INVOKABLE bool save(const QString &themeMode,
                          const QString &primaryColor,
                          bool animationsEnabled,
                          bool notificationsEnabled,
                          const QString &logLevel,
                          const QString &logFilePath);

signals:
    /** @brief 任一已保存设置发生变化时发出。 */
    void settingsChanged();
    /** @brief 最近错误发生变化时发出。 */
    void lastErrorChanged();

private:
    /**
     * @brief 更新最近一次设置错误。
     * @param error 新的错误文本；传入空字符串表示清除错误。
     */
    void setLastError(const QString &error);
    /**
     * @brief 规范化主题模式名称。
     * @param mode 待规范化的主题模式。
     * @return 支持的主题模式名称。
     */
    [[nodiscard]] static QString normalizedThemeMode(const QString &mode);
    /**
     * @brief 规范化日志级别名称。
     * @param level 待规范化的日志级别。
     * @return 支持的日志级别名称。
     */
    [[nodiscard]] static QString normalizedLogLevel(const QString &level);

    QString m_themeMode;
    QString m_primaryColor;
    QString m_logLevel;
    QString m_logFilePath;
    QString m_lastError;
    bool m_animationsEnabled = true;
    bool m_notificationsEnabled = true;
};

#endif // APPSETTINGS_H
