#include <cstdlib>

#include "zen-common/cpp/logger.h"

auto
main() -> int
{
  zen::common::Logger::Init(zen::common::Logger::kDebug);
  LOG_DEBUG("Hello World!");
  return EXIT_SUCCESS;
}
