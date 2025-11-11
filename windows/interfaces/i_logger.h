#ifndef ANYWP_ENGINE_I_LOGGER_H_
#define ANYWP_ENGINE_I_LOGGER_H_

#include <string>

namespace anywp_engine {

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
  DEBUG = 0,
  INFO = 1,
  WARNING = 2,
  ERROR = 3,
  FATAL = 4
};

/**
 * @brief ILogger - Abstract interface for logging
 * 
 * Purpose: Decouples logging operations from concrete implementation,
 *          enabling different logging backends (console, file, network, etc.)
 * 
 * Thread-safe: Implementation-dependent
 */
class ILogger {
public:
  virtual ~ILogger() = default;
  
  /**
   * Log debug message
   * 
   * @param component Component name
   * @param message Log message
   */
  virtual void Debug(const std::string& component, const std::string& message) = 0;
  
  /**
   * Log info message
   * 
   * @param component Component name
   * @param message Log message
   */
  virtual void Info(const std::string& component, const std::string& message) = 0;
  
  /**
   * Log warning message
   * 
   * @param component Component name
   * @param message Log message
   */
  virtual void Warning(const std::string& component, const std::string& message) = 0;
  
  /**
   * Log error message
   * 
   * @param component Component name
   * @param message Log message
   */
  virtual void Error(const std::string& component, const std::string& message) = 0;
  
  /**
   * Set minimum log level
   * 
   * @param level Minimum level to log
   */
  virtual void SetMinLevel(LogLevel level) = 0;
  
  /**
   * Enable/disable file logging
   * 
   * @param file_path Log file path
   */
  virtual void EnableFileLogging(const std::string& file_path) = 0;
  
  /**
   * Disable file logging
   */
  virtual void DisableFileLogging() = 0;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_I_LOGGER_H_

