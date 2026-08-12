/**
 * @file configpolicyutils_p.h
 * @brief 声明配置策略实现共享的私有规范化与校验工具。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-09
 */

#ifndef CONFIGPOLICYUTILS_P_H
#define CONFIGPOLICYUTILS_P_H

#include "result.h"

#include <QString>

#include <string>

namespace Config::Detail
{

/**
 * @brief 创建一个配置字段校验问题。
 * @param[in] fieldPath 使用 JSON 点分路径表示的字段位置。
 * @param[in] message 面向用户或日志的中文问题说明。
 * @return 包含指定字段路径和说明的校验问题。
 */
[[nodiscard]] Issue makeIssue(QString fieldPath, QString message);

/**
 * @brief 将配置名称去除首尾空白并转换为小写。
 * @param[in] value 待规范化的 UTF-8 文本。
 * @return 规范化后的 Qt 字符串。
 */
[[nodiscard]] QString normalizedName(const std::string &value);

/**
 * @brief 判断文本是否为规范的十六进制 RGB 颜色。
 * @param[in] value 待校验的颜色文本。
 * @param[in] allowEmpty 是否允许空文本。
 * @return 文本为空且允许空值，或符合 @c #RRGGBB 格式时返回 @c true。
 */
[[nodiscard]] bool isColor(const std::string &value, bool allowEmpty = false);

/**
 * @brief 将颜色文本去除首尾空白并转换为大写。
 * @param[in,out] value 待规范化的颜色文本。
 */
void normalizeColor(std::string &value);

} // namespace Config::Detail

#endif // CONFIGPOLICYUTILS_P_H
