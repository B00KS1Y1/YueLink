/**
 * @file consoleinput.h
 * @brief 声明 CLI 异步标准输入处理器。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-24
 */

#ifndef CONSOLEINPUT_H
#define CONSOLEINPUT_H

#include <QObject>

class ConsoleInput final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造异步终端输入处理器。
     * @param parent 可选的 QObject 父对象。
     */
    explicit ConsoleInput(QObject *parent = nullptr);
    /** @brief 启动标准输入读取线程。 */
    void start();

signals:
    /**
     * @brief 读取到完整输入行时发出。
     * @param line 不包含换行符的输入文本。
     */
    void lineRead(const QString &line);
    /** @brief 标准输入关闭或到达文件末尾时发出。 */
    void inputClosed();

private:
    bool m_started = false;
};

#endif // CONSOLEINPUT_H
