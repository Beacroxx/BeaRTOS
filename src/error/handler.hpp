#pragma once

#include "codes.hpp"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

namespace ErrorHandler {
void handle(ErrorCode code = ErrorCode::UNKNOWN, const char *file = __FILE__, int line = __LINE__);
void hardFault(ErrorCode code = ErrorCode::HARD_FAULT, const char *file = __FILE__, int line = __LINE__);
void reportError(ErrorCode code, const char *file, int line);
#if ENABLE_ERROR_STRINGS
const char *getErrorString(ErrorCode code);
#endif

bool isCriticalError(ErrorCode code);
void printErrorOverUart(ErrorCode code, const char *file, int line);
} // namespace ErrorHandler