#include "zen-common/cpp/logger.h"

#include "zen-common/log.h"

namespace zen::common {

namespace {

auto
ToLogImportance(Logger::Severity severity) -> zn_log_importance_t
{
  switch (severity) {
    case Logger::kDebug:
      return ZEN_DEBUG;
    case Logger::kInfo:
      return ZEN_INFO;
    case Logger::kWarn:
      return ZEN_WARN;
    case Logger::kError:
      return ZEN_ERROR;
  }
}

};  // namespace

void
Logger::Init(Logger::Severity severity)
{
  zn_log_importance_t importance = ToLogImportance(severity);

  zn_log_init(importance, nullptr);
}

void
// NOLINTNEXTLINE(cert-dcl50-cpp)
Logger::Print(Severity severity, const char *format, ...)
{
  va_list args;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  va_start(args, format);

  zn_log_importance_t verbosity = ToLogImportance(severity);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  zn_vlog_(verbosity, format, args);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  va_end(args);
}

}  // namespace zen::common
