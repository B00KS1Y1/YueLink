/**
 * @file path.h
 * @brief 声明应用系统目录、配置、日志、数据库与数据文件路径解析辅助函数。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef PATH_H
#define PATH_H

#include <QString>

namespace Utils::Path
{

/**
 * @brief 返回应用系统目录的绝对路径。
 * @return 默认返回可执行文件同级 system 目录；可由 YUELINK_SYSTEM_DIR 覆盖。
 */
QString systemDirectory();
/**
 * @brief 返回应用配置目录的绝对路径。
 * @return 默认返回 system/configs；可由 YUELINK_CONFIG_DIR 覆盖。
 */
QString configDirectory();
/**
 * @brief 返回配置目录中指定文件的绝对路径。
 * @param fileName 配置文件名。
 * @return 规范化后的配置文件绝对路径。
 */
QString configFile(const QString &fileName);
/**
 * @brief 返回应用日志目录的绝对路径。
 * @return 默认返回 system/logs；可由 YUELINK_LOG_DIR 覆盖。
 */
QString logDirectory();
/**
 * @brief 返回日志目录中指定文件的绝对路径。
 * @param fileName 日志文件名或相对路径；兼容并移除旧版 logs/ 前缀。
 * @return 规范化后的日志文件绝对路径。
 */
QString logFile(const QString &fileName);
/**
 * @brief 返回应用数据库目录的绝对路径。
 * @return 默认返回 system/database；可由 YUELINK_DATABASE_DIR 覆盖。
 */
QString databaseDirectory();
/**
 * @brief 返回数据库目录中指定文件的绝对路径。
 * @param fileName 数据库文件名或相对路径；兼容并移除旧版 data/ 或 database/ 前缀。
 * @return 规范化后的数据库文件绝对路径。
 */
QString databaseFile(const QString &fileName);
/**
 * @brief 返回用于接收文件的默认下载目录。
 * @return 系统下载目录下的 YueLink 子目录；系统下载目录不可用时使用用户主目录下的 Downloads/YueLink。
 */
QString defaultDownloadDirectory();
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
