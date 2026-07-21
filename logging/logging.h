#ifndef LOGGING_H
#define LOGGING_H

#include "config/config.h"
#include "config/result.h"

namespace Logging
{

[[nodiscard]] Config::Result initialize(const Config::LogConfig &config);
void shutdown();

} // namespace Logging

#endif // LOGGING_H
