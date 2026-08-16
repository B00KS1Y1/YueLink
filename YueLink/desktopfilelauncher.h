/**
 * @file desktopfilelauncher.h
 * @brief 声明桌面文件打开适配器。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef DESKTOPFILELAUNCHER_H
#define DESKTOPFILELAUNCHER_H

#include "ifilelauncher.h"

class DesktopFileLauncher final : public IFileLauncher
{
public:
    /**
     * @brief 使用系统默认应用打开文件。
     * @param filePath 待打开的本地文件路径。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 已成功请求系统打开文件时返回 @c true。
     */
    [[nodiscard]] bool openFile(const QString &filePath, QString *errorMessage) override;
    /**
     * @brief 在系统文件管理器中定位文件。
     * @param filePath 待定位的本地文件路径。
     * @param[out] errorMessage 操作失败时接收错误说明。
     * @return 已成功请求系统定位文件时返回 @c true。
     */
    [[nodiscard]] bool revealInFolder(const QString &filePath, QString *errorMessage) override;
};

#endif // DESKTOPFILELAUNCHER_H
