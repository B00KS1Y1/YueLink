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
    explicit ConfigStore(QString fileName, T defaults = {});

    [[nodiscard]] T get() const;
    void set(T config);
    [[nodiscard]] Result load();
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
