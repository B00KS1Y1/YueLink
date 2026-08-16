/**
 * @file appsettings.h
 * @brief 声明面向 QML 的分类设置组合入口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "settings/applicationsettingsmodel.h"
#include "settings/logsettingsmodel.h"
#include "settings/themesettingsmodel.h"

#include <QObject>
#include <QtQml/qqmlregistration.h>

/**
 * @brief 作为 QML 单例统一暴露应用、主题和日志三个独立设置模型。
 *
 * 分类模型分别拥有自己的自动保存状态，并且只写入与其对应的 JSON 配置文件。
 */
class AppSettings : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AppSettings)
    QML_SINGLETON
    Q_PROPERTY(ApplicationSettingsModel *application READ application CONSTANT)
    Q_PROPERTY(ThemeSettingsModel *theme READ theme CONSTANT)
    Q_PROPERTY(LogSettingsModel *log READ log CONSTANT)

public:
    /**
     * @brief 构造 QML 设置组合入口及其三个分类模型。
     * @param[in] parent 可选的 QObject 父对象。
     */
    explicit AppSettings(QObject *parent = nullptr);

    /**
     * @brief 返回通用设置模型。
     * @return 由当前单例拥有、生命周期与当前单例一致的模型指针。
     */
    [[nodiscard]] ApplicationSettingsModel *application() const;

    /**
     * @brief 返回外观设置模型。
     * @return 由当前单例拥有、生命周期与当前单例一致的模型指针。
     */
    [[nodiscard]] ThemeSettingsModel *theme() const;

    /**
     * @brief 返回高级日志设置模型。
     * @return 由当前单例拥有、生命周期与当前单例一致的模型指针。
     */
    [[nodiscard]] LogSettingsModel *log() const;

private:
    ApplicationSettingsModel *m_application = nullptr; ///< 通用设置模型。
    ThemeSettingsModel *m_theme = nullptr;             ///< 外观设置模型。
    LogSettingsModel *m_log = nullptr;                 ///< 高级日志设置模型。
};

#endif // APPSETTINGS_H
