/**
 * @file configstore.h
 * @brief 声明类型化配置存储及全局配置对象。
 * @author xili <1424858143@qq.com>
 * @date 2026-07-21
 */

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "config.h"
#include "result.h"

#include <QString>

namespace Config
{

template <typename T> class ConfigStore
{
public:
    /**
     * @brief 构造指定配置文件对应的类型化存储。
     * @param fileName 配置文件名。
     * @param defaults 文件不存在或字段缺失时使用的默认配置。
     */
    explicit ConfigStore(QString fileName, T defaults = {});

    /**
     * @brief 返回当前内存配置的副本。
     * @return 当前配置值。
     */
    [[nodiscard]] T get() const;
    /**
     * @brief 更新当前内存配置。
     * @param config 新的配置值。
     */
    void set(T config);
    /**
     * @brief 从配置文件加载数据。
     * @return 加载操作结果。
     */
    [[nodiscard]] Result load();
    /**
     * @brief 将当前配置原子保存到文件。
     * @return 保存操作结果。
     */
    [[nodiscard]] Result save() const;

private:
    QString m_fileName;
    T m_defaults;
    T m_config;
};

extern ConfigStore<LogConfig> log;
extern ConfigStore<ThemeConfig> theme;
extern ConfigStore<DatabaseConfig> database;
extern ConfigStore<ApplicationConfig> application;

} // namespace Config

#endif // CONFIGSTORE_H
