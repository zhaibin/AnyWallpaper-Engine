#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_SAFETY_MACROS_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_SAFETY_MACROS_H_

#include "logger.h"
#include <windows.h>
#include <string>

// ========================================
// Null Pointer Safety Macros
// ========================================

// Return with custom value if pointer is null
#define RETURN_IF_NULL(ptr, msg, ret_val) \
  do { \
    if (!(ptr)) { \
      anywp_engine::Logger::Instance().Error("NullCheck", \
        std::string(__FUNCTION__) + ": " + (msg)); \
      return ret_val; \
    } \
  } while(0)

// Return false if pointer is null
#define RETURN_FALSE_IF_NULL(ptr, msg) \
  RETURN_IF_NULL(ptr, msg, false)

// Return empty string if pointer is null
#define RETURN_EMPTY_IF_NULL(ptr, msg) \
  RETURN_IF_NULL(ptr, msg, std::string())

// Return void if pointer is null
#define RETURN_VOID_IF_NULL(ptr, msg) \
  do { \
    if (!(ptr)) { \
      anywp_engine::Logger::Instance().Error("NullCheck", \
        std::string(__FUNCTION__) + ": " + (msg)); \
      return; \
    } \
  } while(0)

// ========================================
// HRESULT Safety Macros
// ========================================

// Return false if HRESULT indicates failure
#define RETURN_IF_FAILED(hr, msg) \
  do { \
    if (FAILED(hr)) { \
      anywp_engine::Logger::Instance().Error("HRESULTCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + " HRESULT=0x" + std::to_string(hr)); \
      return false; \
    } \
  } while(0)

// Return with custom value if HRESULT indicates failure
#define RETURN_VAL_IF_FAILED(hr, msg, ret_val) \
  do { \
    if (FAILED(hr)) { \
      anywp_engine::Logger::Instance().Error("HRESULTCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + " HRESULT=0x" + std::to_string(hr)); \
      return ret_val; \
    } \
  } while(0)

// Return void if HRESULT indicates failure
#define RETURN_VOID_IF_FAILED(hr, msg) \
  do { \
    if (FAILED(hr)) { \
      anywp_engine::Logger::Instance().Error("HRESULTCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + " HRESULT=0x" + std::to_string(hr)); \
      return; \
    } \
  } while(0)

// Log warning and continue if HRESULT indicates failure
#define WARN_IF_FAILED(hr, msg) \
  do { \
    if (FAILED(hr)) { \
      anywp_engine::Logger::Instance().Warn("HRESULTCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + " HRESULT=0x" + std::to_string(hr)); \
    } \
  } while(0)

// ========================================
// Boolean Condition Safety Macros
// ========================================

// Return false if condition is false
#define RETURN_FALSE_IF(condition, msg) \
  do { \
    if (condition) { \
      anywp_engine::Logger::Instance().Error("ConditionCheck", \
        std::string(__FUNCTION__) + ": " + (msg)); \
      return false; \
    } \
  } while(0)

// Return void if condition is false
#define RETURN_VOID_IF(condition, msg) \
  do { \
    if (condition) { \
      anywp_engine::Logger::Instance().Error("ConditionCheck", \
        std::string(__FUNCTION__) + ": " + (msg)); \
      return; \
    } \
  } while(0)

// Return with custom value if condition is true
#define RETURN_VAL_IF(condition, msg, ret_val) \
  do { \
    if (condition) { \
      anywp_engine::Logger::Instance().Error("ConditionCheck", \
        std::string(__FUNCTION__) + ": " + (msg)); \
      return ret_val; \
    } \
  } while(0)

// ========================================
// Window Handle Safety Macros
// ========================================

// Return false if window handle is invalid
#define RETURN_FALSE_IF_INVALID_HWND(hwnd, msg) \
  do { \
    if (!(hwnd) || !IsWindow(hwnd)) { \
      anywp_engine::Logger::Instance().Error("WindowCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + " (invalid HWND)"); \
      return false; \
    } \
  } while(0)

// Return void if window handle is invalid
#define RETURN_VOID_IF_INVALID_HWND(hwnd, msg) \
  do { \
    if (!(hwnd) || !IsWindow(hwnd)) { \
      anywp_engine::Logger::Instance().Error("WindowCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + " (invalid HWND)"); \
      return; \
    } \
  } while(0)

// ========================================
// Array/Vector Bounds Safety Macros
// ========================================

// Return false if index is out of bounds
#define RETURN_FALSE_IF_OUT_OF_BOUNDS(index, container, msg) \
  do { \
    if ((index) < 0 || (size_t)(index) >= (container).size()) { \
      anywp_engine::Logger::Instance().Error("BoundsCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + \
        " (index=" + std::to_string(index) + ", size=" + std::to_string((container).size()) + ")"); \
      return false; \
    } \
  } while(0)

// Return with custom value if index is out of bounds
#define RETURN_VAL_IF_OUT_OF_BOUNDS(index, container, msg, ret_val) \
  do { \
    if ((index) < 0 || (size_t)(index) >= (container).size()) { \
      anywp_engine::Logger::Instance().Error("BoundsCheck", \
        std::string(__FUNCTION__) + ": " + (msg) + \
        " (index=" + std::to_string(index) + ", size=" + std::to_string((container).size()) + ")"); \
      return ret_val; \
    } \
  } while(0)

// ========================================
// Try-Catch Wrapper Macros
// ========================================

// Wrap code block with try-catch, return false on exception
#define TRY_CATCH_RETURN_FALSE(code_block) \
  do { \
    try { \
      code_block \
    } catch (const std::exception& e) { \
      anywp_engine::Logger::Instance().Error("Exception", \
        std::string(__FUNCTION__) + ": " + e.what()); \
      return false; \
    } catch (...) { \
      anywp_engine::Logger::Instance().Error("Exception", \
        std::string(__FUNCTION__) + ": Unknown exception"); \
      return false; \
    } \
  } while(0)

// Wrap code block with try-catch, return void on exception
#define TRY_CATCH_RETURN_VOID(code_block) \
  do { \
    try { \
      code_block \
    } catch (const std::exception& e) { \
      anywp_engine::Logger::Instance().Error("Exception", \
        std::string(__FUNCTION__) + ": " + e.what()); \
      return; \
    } catch (...) { \
      anywp_engine::Logger::Instance().Error("Exception", \
        std::string(__FUNCTION__) + ": Unknown exception"); \
      return; \
    } \
  } while(0)

// Wrap code block with try-catch, log and continue on exception
#define TRY_CATCH_LOG(code_block) \
  do { \
    try { \
      code_block \
    } catch (const std::exception& e) { \
      anywp_engine::Logger::Instance().Error("Exception", \
        std::string(__FUNCTION__) + ": " + e.what()); \
    } catch (...) { \
      anywp_engine::Logger::Instance().Error("Exception", \
        std::string(__FUNCTION__) + ": Unknown exception"); \
    } \
  } while(0)

// v2.0+ Phase 5.3: Specialized macro for module initialization
#define TRY_CATCH_INIT_MODULE(module_name, init_code) \
  do { \
    try { \
      init_code \
      anywp_engine::Logger::Instance().Info("Refactor", \
        module_name " module initialized successfully"); \
    } catch (const std::exception& e) { \
      anywp_engine::Logger::Instance().Error("Refactor", \
        std::string("Failed to initialize " module_name ": ") + e.what()); \
    } catch (...) { \
      anywp_engine::Logger::Instance().Error("Refactor", \
        "Unknown exception initializing " module_name); \
    } \
  } while(0)

// ========================================
// Performance/Debug Macros
// ========================================

// Log entry to function (useful for debugging)
#define LOG_FUNCTION_ENTRY() \
  anywp_engine::Logger::Instance().Debug(__FUNCTION__, "Entering function")

// Log exit from function (useful for debugging)
#define LOG_FUNCTION_EXIT() \
  anywp_engine::Logger::Instance().Debug(__FUNCTION__, "Exiting function")

// Log function entry and exit with scope guard
#define LOG_FUNCTION_SCOPE() \
  struct FunctionScopeLogger { \
    std::string func_name; \
    FunctionScopeLogger(const char* name) : func_name(name) { \
      anywp_engine::Logger::Instance().Debug(func_name, "Entering"); \
    } \
    ~FunctionScopeLogger() { \
      anywp_engine::Logger::Instance().Debug(func_name, "Exiting"); \
    } \
  } __scope_logger(__FUNCTION__)

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_SAFETY_MACROS_H_

