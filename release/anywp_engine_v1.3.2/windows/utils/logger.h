#ifndef ANYWP_ENGINE_LOGGER_H_
#define ANYWP_ENGINE_LOGGER_H_

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>

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
 */
class Logger {
public:
  enum class Level {
    DEBUG,
    INFO,
    WARNING,
    ERROR
  };

  static Logger& Instance() {
    static Logger instance;
    return instance;
  }

  // Logging methods
  void Debug(const std::string& component, const std::string& message);
  void Info(const std::string& component, const std::string& message);
  void Warning(const std::string& component, const std::string& message);
  void Error(const std::string& component, const std::string& message);
  
  // Generic log method
  void Log(Level level, const std::string& component, const std::string& message);

  // Configuration
  void SetMinLevel(Level level);
  void EnableFileLogging(const std::string& file_path);
  void DisableFileLogging();
  void EnableConsoleLogging(bool enable);

private:
  Logger();
  ~Logger();
  
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  std::string GetTimestamp();
  std::string LevelToString(Level level);
  std::string FormatMessage(Level level, const std::string& component, const std::string& message);
  
  void WriteToConsole(const std::string& message);
  void WriteToFile(const std::string& message);

  Level min_level_;
  bool console_enabled_;
  bool file_enabled_;
  std::string log_file_path_;
  std::ofstream log_file_;
  std::mutex mutex_;
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

