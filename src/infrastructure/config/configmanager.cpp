#include "configmanager.h"

#include <type_traits>

namespace
{
void mergeResult(Config::Result &aggregate, const Config::Result &current)
{
    aggregate.changed = aggregate.changed || current.changed;
    aggregate.issues.append(current.issues);
    if (current)
    {
        return;
    }
    if (aggregate)
    {
        aggregate.errorCode = current.errorCode;
        aggregate.filePath = current.filePath;
        aggregate.errorMessage = current.errorMessage;
        return;
    }
    if (!aggregate.errorMessage.contains(current.errorMessage))
    {
        aggregate.errorMessage += QStringLiteral("；%1").arg(current.errorMessage);
    }
}
} // namespace

namespace Config
{

ConfigManager::ConfigManager()
: QObject(nullptr)
{
}

ConfigManager &ConfigManager::instance()
{
    static ConfigManager manager;
    return manager;
}

Result ConfigManager::initialize(const QString &configDirectory)
{
    Result aggregate;
    std::apply(
        [&aggregate, &configDirectory](auto &...stores) {
            const auto initializeOne = [&aggregate, &configDirectory](auto &store) {
                const Result result = store.initialize(configDirectory);
                mergeResult(aggregate, result);
            };
            (initializeOne(stores), ...);
        },
        m_stores);
    return aggregate;
}

Result ConfigManager::reloadAll()
{
    Result aggregate;
    std::apply(
        [this, &aggregate](auto &...stores) {
            const auto reloadOne = [this, &aggregate](auto &store) {
                using Store = std::decay_t<decltype(store)>;
                using Value = typename Store::ValueType;
                const Result result = store.reload();
                mergeResult(aggregate, result);
                notifyChanged<Value>(result);
            };
            (reloadOne(stores), ...);
        },
        m_stores);
    return aggregate;
}

} // namespace Config
