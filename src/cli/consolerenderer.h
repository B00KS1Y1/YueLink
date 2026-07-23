/**
 * @file consolerenderer.h
 * @brief 声明 CLI 文本与 JSON Lines 输出渲染器。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef CONSOLERENDERER_H
#define CONSOLERENDERER_H

#include <QJsonObject>
#include <QString>
#include <QTextStream>

class ConsoleRenderer final
{
public:
    /**
     * @brief 构造终端输出渲染器。
     * @param jsonLines 是否使用 JSON Lines 输出模式。
     */
    explicit ConsoleRenderer(bool jsonLines);

    /**
     * @brief 返回当前是否使用 JSON Lines 输出模式。
     * @return 使用 JSON Lines 输出模式时返回 @c true。
     */
    [[nodiscard]] bool jsonLines() const;
    /**
     * @brief 输出普通文本行。
     * @param text 待输出的文本。
     */
    void line(const QString &text = {});
    /**
     * @brief 输出结构化错误。
     * @param code 机器可读的错误码。
     * @param message 便于用户阅读的错误说明。
     */
    void error(const QString &code, const QString &message);
    /**
     * @brief 输出终端事件。
     * @param type 事件类型。
     * @param data 事件数据。
     */
    void event(const QString &type, QJsonObject data = {});
    /** @brief 在文本模式下输出交互提示符。 */
    void prompt();

private:
    QTextStream m_output;
    bool m_jsonLines = false;
};

#endif // CONSOLERENDERER_H
