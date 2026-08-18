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
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

/**
 * @brief 管理主题模式、主题色、窗口背景和动画，并将修改自动保存到 theme.json。
 */
class ThemeSettingsModel : public SettingsModelBase
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QString primaryColor READ primaryColor NOTIFY primaryColorChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled NOTIFY animationsEnabledChanged)
    Q_PROPERTY(QUrl backgroundImage READ backgroundImage NOTIFY backgroundImageChanged)
    Q_PROPERTY(qreal backgroundOpacity READ backgroundOpacity NOTIFY backgroundOpacityChanged)
    Q_PROPERTY(QVariantList backgroundImages READ backgroundImages NOTIFY backgroundImagesChanged)

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
     * @brief 返回当前窗口背景图片 URL。
     * @return 当前已启用背景的内置 qrc URL 或本地 file URL。
     */
    [[nodiscard]] QUrl backgroundImage() const;

    /**
     * @brief 返回覆盖背景图片的主题表面不透明度。
     * @return 0.0 到 1.0 之间的不透明度。
     */
    [[nodiscard]] qreal backgroundOpacity() const;

    /**
     * @brief 返回可供用户切换的内置与自定义背景图片列表。
     * @return 由 label 和 value 字段组成的 QML 下拉选项列表。
     */
    [[nodiscard]] QVariantList backgroundImages() const;

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

    /**
     * @brief 更新并自动保存窗口背景图片。
     * @param[in] backgroundImage 已启用背景的内置 qrc URL 或本地 file URL。
     * @return 图片有效且配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateBackgroundImage(const QUrl &backgroundImage);

    /**
     * @brief 更新并自动保存背景表面不透明度。
     * @param[in] opacity 0.0 到 1.0 之间的不透明度。
     * @return 数值有效且配置保存成功或值未变化时返回 @c true。
     */
    Q_INVOKABLE bool updateBackgroundOpacity(qreal opacity);

    /**
     * @brief 将本地图片复制到应用背景库，使用指定名称保存并立即切换。
     * @param[in] name 用户输入的背景名称，长度为 1 到 64 个字符且不能重复。
     * @param[in] sourceImage 待导入的本地图片 URL。
     * @return 图片复制和配置保存均成功时返回 @c true。
     */
    Q_INVOKABLE bool importBackgroundImage(const QString &name, const QUrl &sourceImage);

    /**
     * @brief 在背景库中停用指定图片，并保留配置记录与本地文件。
     * @param[in] backgroundImage 待停用的内置或用户背景图片 URL。
     * @return 背景成功标记为停用时返回 @c true。
     *
     * 背景库至少保留一项启用背景；如果停用当前背景，将自动切换到其余背景。
     */
    Q_INVOKABLE bool removeBackgroundImage(const QUrl &backgroundImage);

signals:
    /** @brief 主题模式发生变化时发出。 */
    void modeChanged();
    /** @brief 主题色发生变化时发出。 */
    void primaryColorChanged();
    /** @brief 界面动画开关发生变化时发出。 */
    void animationsEnabledChanged();
    /** @brief 窗口背景图片发生变化时发出。 */
    void backgroundImageChanged();
    /** @brief 背景表面不透明度发生变化时发出。 */
    void backgroundOpacityChanged();
    /** @brief 可选背景图片列表发生变化时发出。 */
    void backgroundImagesChanged();
    /** @brief 任一外观设置发生变化时发出。 */
    void settingsChanged();

private:
    /** @brief 从配置管理器刷新规范化后的主题设置缓存。 */
    void refreshFromConfig();

    QString m_mode;                  ///< 当前主题模式。
    QString m_primaryColor;          ///< 当前规范化主题色。
    bool m_animationsEnabled = true; ///< 当前界面动画开关。
    QUrl m_backgroundImage;          ///< 当前窗口背景图片 URL。
    qreal m_backgroundOpacity = 0.6; ///< 当前背景表面不透明度。
    QVariantList m_backgroundImages; ///< 提供给 QML 下拉框的背景图片选项。
};

#endif // THEMESETTINGSMODEL_H
