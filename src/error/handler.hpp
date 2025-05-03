#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "codes.hpp"

class ErrorHandler {
public:
  static void handle(ErrorCode code = ErrorCode::UNKNOWN, const char* file = __FILE__, int line = __LINE__);
  static void hardFault(ErrorCode code = ErrorCode::HARD_FAULT, const char* file = __FILE__, int line = __LINE__);
  static void reportError(ErrorCode code, const char* file, int line);
#if ENABLE_ERROR_STRINGS
  static const char* getErrorString(ErrorCode code);
#endif
  
private:
  static bool isCriticalError(ErrorCode code);
  static void printErrorOverUart(ErrorCode code, const char* file, int line);
}; 