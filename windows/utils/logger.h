#ifndef ANYWP_ENGINE_LOGGER_H_
#define ANYWP_ENGINE_LOGGER_H_

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <vector>
#include <map>

// Undef Windows macros that conflict with our enum
#ifdef ERROR
#undef ERROR
#endif

namespace anywp_engine {

/**
 * Logger - Centralized logging system
 * 
 * Features:
 * - Thread-safe logging
 * - Multiple log levels (DEBUG, INFO, WARNING, ERROR)
 * - Console and file output
 * - Automatic timestamping
 * - Log file rotation (optional)
 * - Configurable output modes
 * 
 * Log Format Specification:
 * File:    [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [COMPONENT] message
 * Console: [AnyWP] [COMPONENT] message
 * 
 * Examples:
 * File:    [2025-01-15 10:30:45.123] [INFO] [Plugin] Plugin initialized
 * Console: [AnyWP] [Plugin] Plugin initialized
 * 
 * Usage Guidelines:
 * - Use Logger::Instance() for ALL logging (replace std::cout)
 * - Component name should be PascalCase (e.g., "WebViewManager", "FlutterBridge")
 * - Messages should be in English, no emoji or special symbols
 * - Use appropriate log levels: 
 *   * DEBUG: Detailed diagnostic info (only in Debug builds)
 *   * INFO: Normal operations, state changes, initialization
 *   * WARNING: Recoverable issues, unexpected but handled situations
 *   * ERROR: Failures, exceptions, critical issues
 * 
 * Output Modes:
 * - FILE_ONLY: Log to file only (Release default)
 * - CONSOLE_ONLY: Log to console only (for testing)
 * - BOTH: Log to both file and console (Debug default)
 * 
 * v2.3.2+: Enhanced for unified logging (replaces std::cout)
 */
class Logger {
public:
  enum class Level {
    DEBUG,
    INFO,
    WARNING,
    ERROR
  };
  
  // v2.3.2+: Output mode control
  enum class OutputMode {
    FILE_ONLY,      // Log to file only (Release default)
    CONSOLE_ONLY,   // Log to console only (for testing)
    BOTH            // Log to both (Debug default)
  };

  static Logger& Instance() {
    static Logger instance;
    return instance;
  }

  // Logging methods
  void Debug(const std::string& component, const std::string& message);
  void Info(const std::string& component, const std::string& message);
  void Warning(const std::string& component, const std::string& message);
  void Warn(const std::string& component, const std::string& message) { Warning(component, message); }  // Alias
  void Error(const std::string& component, const std::string& message);
  
  // Generic log method
  void Log(Level level, const std::string& component, const std::string& message);
  
  // v2.3.2+: Special formatting methods
  void Banner(const std::string& component, const std::string& title);  // Print banner with separators
  void Section(const std::string& component, const std::string& title);  // Print section header

  // Configuration
  void SetMinLevel(Level level);
  void EnableFileLogging(const std::string& file_path);
  void DisableFileLogging();
  void EnableConsoleLogging(bool enable);
  
  // v2.3.2+: Output mode control
  void SetOutputMode(OutputMode mode);
  OutputMode GetOutputMode() const;

  // v2.1.0+ Enhanced features
  void Flush();  // Manually flush buffered logs
  void SetBuffering(bool enabled, size_t buffer_size = 100);
  void EnableRotation(size_t max_file_size = 10 * 1024 * 1024);  // 10MB default
  std::map<std::string, size_t> GetStatistics() const;

private:
  Logger();
  ~Logger();
  
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  std::string GetTimestamp();
  std::string LevelToString(Level level);
  std::string FormatLogMessage(Level level, const std::string& component, const std::string& message);
  
  void WriteToConsole(const std::string& message);
  void WriteToFile(const std::string& message);
  void FlushInternal();  // Internal flush (no mutex lock)
  void RotateLogFile();
  bool ShouldRotate() const;

  Level min_level_;
  bool console_enabled_;
  bool file_enabled_;
  std::string log_file_path_;
  std::ofstream log_file_;
  mutable std::mutex mutex_;
  
  // v2.3.2+: Output mode
  OutputMode output_mode_;
  
  // v2.1.0+ Enhanced features
  bool buffering_enabled_;
  size_t buffer_size_;
  std::vector<std::string> buffer_;
  
  bool rotation_enabled_;
  size_t max_file_size_;
  size_t current_file_size_;
  
  std::map<Level, size_t> log_counts_;
};

// Convenience macros
#define ANYWP_LOG_DEBUG(component, message) \
  anywp_engine::Logger::Instance().Debug(component, message)

#define ANYWP_LOG_INFO(component, message) \
  anywp_engine::Logger::Instance().Info(component, message)

#define ANYWP_LOG_WARNING(component, message) \
  anywp_engine::Logger::Instance().Warning(component, message)

#define ANYWP_LOG_ERROR(component, message) \
  anywp_engine::Logger::Instance().Error(component, message)

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_LOGGER_H_

