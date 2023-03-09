#pragma once

#ifdef __GNUC__
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ATTRIB_PRINTF(start, end) __attribute__((format(printf, start, end)))
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ATTRIB_PRINTF(start, end)
#endif

namespace zen::common {

class Logger
{
 public:
  enum Severity {
    kDebug = 0,  // logs for debugging during development.
    kInfo,       // logs that may be useful to some users.
    kWarn,       // logs for recoverable failures.
    kError,      // logs for unrecoverable failures.
  };

  static void Init(Severity severity);

  static void Print(Severity severity, const char *format, ...)
      ATTRIB_PRINTF(2, 3);
};

}  // namespace zen::common

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG(format, ...)                                \
  ::zen::common::Logger::Print(::zen::common::Logger::kDebug, \
      "[zen] [%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INFO(format, ...)                                \
  ::zen::common::Logger::Print(::zen::common::Logger::kInfo, \
      "[zen] [%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_WARN(format, ...)                                \
  ::zen::common::Logger::Print(::zen::common::Logger::kWarn, \
      "[zen] [%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ERROR(format, ...)                                \
  ::zen::common::Logger::Print(::zen::common::Logger::kError, \
      "[zen] [%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
