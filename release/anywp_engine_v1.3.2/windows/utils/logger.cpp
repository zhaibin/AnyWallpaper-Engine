#include "logger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace anywp_engine {

Logger::Logger() 
    : min_level_(Level::INFO),
      console_enabled_(true),
      file_enabled_(false) {
}

Logger::~Logger() {
  if (log_file_.is_open()) {
    log_file_.close();
  }
}

// ========== Public Logging Methods ==========

void Logger::Debug(const std::string& component, const std::string& message) {
  Log(Level::DEBUG, component, message);
}

void Logger::Info(const std::string& component, const std::string& message) {
  Log(Level::INFO, component, message);
}

void Logger::Warning(const std::string& component, const std::string& message) {
  Log(Level::WARNING, component, message);
}

void Logger::Error(const std::string& component, const std::string& message) {
  Log(Level::ERROR, component, message);
}

void Logger::Log(Level level, const std::string& component, const std::string& message) {
  // Check log level filter
  if (level < min_level_) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  
  std::string formatted = FormatMessage(level, component, message);
  
  if (console_enabled_) {
    WriteToConsole(formatted);
  }
  
  if (file_enabled_) {
    WriteToFile(formatted);
  }
}

// ========== Configuration ==========

void Logger::SetMinLevel(Level level) {
  std::lock_guard<std::mutex> lock(mutex_);
  min_level_ = level;
}

void Logger::EnableFileLogging(const std::string& file_path) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (log_file_.is_open()) {
    log_file_.close();
  }
  
  log_file_path_ = file_path;
  log_file_.open(log_file_path_, std::ios::app);
  
  if (log_file_.is_open()) {
    file_enabled_ = true;
    std::cout << "[AnyWP] [Logger] File logging enabled: " << log_file_path_ << std::endl;
  } else {
    file_enabled_ = false;
    std::cout << "[AnyWP] [Logger] ERROR: Failed to open log file: " << log_file_path_ << std::endl;
  }
}

void Logger::DisableFileLogging() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (log_file_.is_open()) {
    log_file_.close();
  }
  
  file_enabled_ = false;
}

void Logger::EnableConsoleLogging(bool enable) {
  std::lock_guard<std::mutex> lock(mutex_);
  console_enabled_ = enable;
}

// ========== Internal Helpers ==========

std::string Logger::GetTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()) % 1000;
  
  std::tm tm_now;
  localtime_s(&tm_now, &time_t_now);
  
  std::ostringstream oss;
  oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
  oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
  
  return oss.str();
}

std::string Logger::LevelToString(Level level) {
  switch (level) {
    case Level::DEBUG:   return "DEBUG";
    case Level::INFO:    return "INFO";
    case Level::WARNING: return "WARNING";
    case Level::ERROR:   return "ERROR";
    default:             return "UNKNOWN";
  }
}

std::string Logger::FormatMessage(Level level, const std::string& component, const std::string& message) {
  std::ostringstream oss;
  oss << "[" << GetTimestamp() << "] "
      << "[" << LevelToString(level) << "] "
      << "[" << component << "] "
      << message;
  return oss.str();
}

void Logger::WriteToConsole(const std::string& message) {
  std::cout << message << std::endl;
}

void Logger::WriteToFile(const std::string& message) {
  if (log_file_.is_open()) {
    log_file_ << message << std::endl;
    log_file_.flush();  // Ensure immediate write
  }
}

}  // namespace anywp_engine

