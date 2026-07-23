/**
 * @file ifilelauncher.h
 * @brief 声明 GUI 文件启动抽象接口。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef IFILELAUNCHER_H
#define IFILELAUNCHER_H

#include <QString>

class IFileLauncher
{
public:
    /** @brief 销毁文件启动器。 */
    virtual ~IFileLauncher() = default;

    /**
     * @brief 使用系统默认应用打开文件。
     * @param filePath 待打开的本地文件路径。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 已成功请求系统打开文件时返回 @c true。
     */
    [[nodiscard]] virtual bool openFile(const QString &filePath,
                                        QString *errorMessage) = 0;
    /**
     * @brief 在系统文件管理器中定位文件。
     * @param filePath 待定位的本地文件路径。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 已成功请求系统定位文件时返回 @c true。
     */
    [[nodiscard]] virtual bool revealInFolder(const QString &filePath,
                                              QString *errorMessage) = 0;
};

#endif // IFILELAUNCHER_H
