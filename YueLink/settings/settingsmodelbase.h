/**
 * @file settingsmodelbase.h
 * @brief 声明设置分类模型共享的自动保存状态基类。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#ifndef SETTINGSMODELBASE_H
#define SETTINGSMODELBASE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

/**
 * @brief 为面向 QML 的设置分类模型提供统一的保存状态和错误文本。
 *
 * 成功状态会短暂保留后自动恢复为空闲；错误状态会持续到下一次成功保存。
 */
class SettingsModelBase : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SettingsModel)
    QML_UNCREATABLE("SettingsModel 仅用于暴露设置保存状态。")
    Q_PROPERTY(SaveState saveState READ saveState NOTIFY saveStateChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    /** @brief 描述最近一次自动保存操作的界面状态。 */
    enum SaveState
    {
        Idle,  ///< 当前没有需要展示的保存反馈。
        Saved, ///< 最近一次设置已成功保存。
        Error  ///< 最近一次设置保存失败。
    };
    Q_ENUM(SaveState)

    /**
     * @brief 构造设置分类模型状态基类。
     * @param[in] parent 可选的 QObject 父对象。
     */
    explicit SettingsModelBase(QObject *parent = nullptr);

    /**
     * @brief 返回当前自动保存状态。
     * @return 当前保存状态枚举值。
     */
    [[nodiscard]] SaveState saveState() const;

    /**
     * @brief 返回最近一次保存错误。
     * @return 错误文本；当前没有错误时返回空字符串。
     */
    [[nodiscard]] QString errorMessage() const;

signals:
    /** @brief 自动保存状态发生变化时发出。 */
    void saveStateChanged();
    /** @brief 最近一次保存错误文本发生变化时发出。 */
    void errorMessageChanged();

protected:
    /** @brief 将状态标记为保存成功，并启动成功提示自动清除计时。 */
    void markSaved();

    /**
     * @brief 将状态标记为保存失败。
     * @param[in] error 错误文本；应提供可直接展示给用户的说明。
     */
    void markError(const QString &error);

private:
    /**
     * @brief 更新保存状态并按需发送变更信号。
     * @param[in] state 新的保存状态。
     */
    void setSaveState(SaveState state);

    /**
     * @brief 更新保存错误文本并按需发送变更信号。
     * @param[in] error 新的错误文本。
     */
    void setErrorMessage(const QString &error);

    SaveState m_saveState = Idle; ///< 当前自动保存反馈状态。
    QString m_errorMessage;       ///< 最近一次自动保存错误文本。
    QTimer m_savedStateTimer;     ///< 成功提示恢复为空闲状态的单次计时器。
};

#endif // SETTINGSMODELBASE_H
