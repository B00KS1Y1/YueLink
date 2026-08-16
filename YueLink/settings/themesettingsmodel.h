/**
 * @file themesettingsmodel.h
 * @brief 声明外观设置的 QML 状态与自动保存模型。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#ifndef THEMESETTINGSMODEL_H
#define THEMESETTINGSMODEL_H

#include "settingsmodelbase.h"

#include <QString>
#include <QtQml/qqmlregistration.h>

/**
 * @brief 管理主题模式、主题色和动画，并将修改自动保存到 theme.json。
 */
class ThemeSettingsModel : public SettingsModelBase
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QString primaryColor READ primaryColor NOTIFY primaryColorChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled NOTIFY animationsEnabledChanged)

public:
    /**
     * @brief 构造外观设置模型并载入当前主题配置。
     * @param[in] parent 可选的 QObject 父对象。
     */
    explicit ThemeSettingsModel(QObject *parent = nullptr);

    /**
     * @brief 返回当前主题模式。
     * @return light、dark 或 system。
     */
    [[nodiscard]] QString mode() const;

    /**
     * @brief 返回当前主题色。
     * @return 使用大写 #RRGGBB 表示的颜色文本。
     */
    [[nodiscard]] QString primaryColor() const;

    /**
     * @brief 返回是否启用界面动画。
     * @return 启用界面动画时返回 @c true。
     */
    [[nodiscard]] bool animationsEnabled() const;

    /**
     * @brief 更新并自动保存主题模式。
     * @param[in] mode 主题模式名称。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateMode(const QString &mode);

    /**
     * @brief 更新并自动保存主题色。
     * @param[in] color 颜色文本。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updatePrimaryColor(const QString &color);

    /**
     * @brief 更新并自动保存界面动画开关。
     * @param[in] enabled 是否启用界面动画。
     * @return 配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateAnimationsEnabled(bool enabled);

signals:
    /** @brief 主题模式发生变化时发出。 */
    void modeChanged();
    /** @brief 主题色发生变化时发出。 */
    void primaryColorChanged();
    /** @brief 界面动画开关发生变化时发出。 */
    void animationsEnabledChanged();
    /** @brief 任一外观设置发生变化时发出。 */
    void settingsChanged();

private:
    /** @brief 从配置管理器刷新规范化后的主题设置缓存。 */
    void refreshFromConfig();

    QString m_mode;                  ///< 当前主题模式。
    QString m_primaryColor;          ///< 当前规范化主题色。
    bool m_animationsEnabled = true; ///< 当前界面动画开关。
};

#endif // THEMESETTINGSMODEL_H
