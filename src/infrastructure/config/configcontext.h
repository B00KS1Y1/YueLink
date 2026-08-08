/**
 * @file configcontext.h
 * @brief 定义配置存储运行时所需的路径上下文。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-06
 */

#ifndef CONFIGCONTEXT_H
#define CONFIGCONTEXT_H

#include <QString>

namespace Config
{

struct ConfigContext
{
    QString configDirectory;
    QString defaultDownloadDirectory;
    QString defaultConfigDirectory;
};

} // namespace Config

#endif // CONFIGCONTEXT_H
