#ifndef CONFIG_RESULT_H
#define CONFIG_RESULT_H

#include <QString>

#include <utility>

namespace Config
{

struct Result
{
    QString errorMessage;

    [[nodiscard]] bool ok() const noexcept
    {
        return errorMessage.isEmpty();
    }

    explicit operator bool() const noexcept
    {
        return ok();
    }

    static Result failure(QString errorMessage)
    {
        Result result;
        result.errorMessage = std::move(errorMessage);
        return result;
    }
};

} // namespace Config

#endif // CONFIG_RESULT_H
