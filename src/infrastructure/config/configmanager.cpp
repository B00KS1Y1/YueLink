#include "configmanager.h"

#include "infrastructure/path.h"

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

Config::ConfigContext currentContext()
{
    return {Utils::Path::configDirectory(), Utils::Path::defaultDownloadDirectory()};
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

Result ConfigManager::initialize()
{
    const ConfigContext context = currentContext();
    Result aggregate;
    std::apply(
        [&aggregate, &context](auto &...stores) {
            const auto initializeOne = [&aggregate, &context](auto &store) {
                const Result result = store.initialize(context);
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
