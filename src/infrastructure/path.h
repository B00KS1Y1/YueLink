/**
 * @file path.h
 * @brief 声明应用数据与配置文件路径解析辅助函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef PATH_H
#define PATH_H

#include <QString>

namespace Utils::Path
{

/**
 * @brief 返回应用配置目录的绝对路径。
 * @return 应用配置目录的绝对路径。
 */
QString configDirectory();
/**
 * @brief 返回配置目录中指定文件的绝对路径。
 * @param fileName 配置文件名。
 * @return 规范化后的配置文件绝对路径。
 */
QString configFile(const QString &fileName);
/**
 * @brief 返回应用数据目录的绝对路径。
 * @return 应用数据目录的绝对路径。
 */
QString dataDirectory();
/**
 * @brief 返回数据目录中指定文件的绝对路径。
 * @param fileName 数据文件名。
 * @return 规范化后的数据文件绝对路径。
 */
QString dataFile(const QString &fileName);

} // namespace Utils::Path

#endif // PATH_H
