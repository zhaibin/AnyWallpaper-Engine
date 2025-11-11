#include "logger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
// Undef Windows macros that conflict with our enum
#ifdef ERROR
#undef ERROR
#endif
#ifdef WARNING
#undef WARNING
#endif
#ifdef INFO
#undef INFO
#endif
#endif

namespace anywp_engine {

Logger::Logger() 
    : min_level_(Level::INFO),
      console_enabled_(true),
      file_enabled_(false),
      buffering_enabled_(false),
      buffer_size_(100),
      rotation_enabled_(false),
      max_file_size_(10 * 1024 * 1024),  // 10MB
      current_file_size_(0) {
}

Logger::~Logger() {
  // Flush any buffered logs before destruction
  if (buffering_enabled_) {
    Flush();
  }
  
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
  
  // Update statistics
  log_counts_[level]++;
  
  std::string formatted = FormatLogMessage(level, component, message);
  
  if (console_enabled_) {
    WriteToConsole(formatted);
  }
  
  if (file_enabled_) {
    if (buffering_enabled_) {
      // Add to buffer
      buffer_.push_back(formatted);
      
      // Auto-flush if buffer is full
      if (buffer_.size() >= buffer_size_) {
        FlushInternal();
      }
    } else {
      // Direct write
      WriteToFile(formatted);
    }
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

std::string Logger::FormatLogMessage(Level level, const std::string& component, const std::string& message) {
  std::ostringstream oss;
  oss << "[" << GetTimestamp() << "] "
      << "[" << LevelToString(level) << "] "
      << "[" << component << "] "
      << message;
  return oss.str();
}

void Logger::WriteToConsole(const std::string& message) {
#ifdef _WIN32
  // Set console output to UTF-8 to fix Chinese character encoding
  static bool console_utf8_initialized = false;
  if (!console_utf8_initialized) {
    SetConsoleOutputCP(CP_UTF8);
    console_utf8_initialized = true;
  }
#endif
  std::cout << message << std::endl;
}

void Logger::WriteToFile(const std::string& message) {
  if (log_file_.is_open()) {
    log_file_ << message << std::endl;
    log_file_.flush();  // Ensure immediate write
    
    current_file_size_ += message.length() + 1;  // +1 for newline
    
    // Check if rotation is needed
    if (rotation_enabled_ && ShouldRotate()) {
      RotateLogFile();
    }
  }
}

// v2.1.0+ Enhanced features implementation

void Logger::Flush() {
  std::lock_guard<std::mutex> lock(mutex_);
  FlushInternal();
}

void Logger::FlushInternal() {
  if (!file_enabled_ || !log_file_.is_open()) {
    return;
  }
  
  for (const auto& msg : buffer_) {
    log_file_ << msg << std::endl;
    current_file_size_ += msg.length() + 1;
  }
  
  log_file_.flush();
  buffer_.clear();
  
  // Check if rotation is needed after flush
  if (rotation_enabled_ && ShouldRotate()) {
    RotateLogFile();
  }
}

void Logger::SetBuffering(bool enabled, size_t buffer_size) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Flush existing buffer if disabling
  if (buffering_enabled_ && !enabled) {
    FlushInternal();
  }
  
  buffering_enabled_ = enabled;
  buffer_size_ = buffer_size;
  buffer_.reserve(buffer_size);
}

void Logger::EnableRotation(size_t max_file_size) {
  std::lock_guard<std::mutex> lock(mutex_);
  rotation_enabled_ = true;
  max_file_size_ = max_file_size;
}

bool Logger::ShouldRotate() const {
  return current_file_size_ >= max_file_size_;
}

void Logger::RotateLogFile() {
  if (!log_file_.is_open()) {
    return;
  }
  
  // Close current file
  log_file_.close();
  
  // Generate rotated filename
  std::string rotated_name = log_file_path_ + "." + 
      std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
  
  // Rename current file
  std::rename(log_file_path_.c_str(), rotated_name.c_str());
  
  // Open new file
  log_file_.open(log_file_path_, std::ios::app);
  current_file_size_ = 0;
  
  if (log_file_.is_open()) {
    std::cout << "[AnyWP] [Logger] Log file rotated: " << rotated_name << std::endl;
  }
}

std::map<std::string, size_t> Logger::GetStatistics() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::map<std::string, size_t> stats;
  stats["DEBUG"] = log_counts_.count(Level::DEBUG) ? log_counts_.at(Level::DEBUG) : 0;
  stats["INFO"] = log_counts_.count(Level::INFO) ? log_counts_.at(Level::INFO) : 0;
  stats["WARNING"] = log_counts_.count(Level::WARNING) ? log_counts_.at(Level::WARNING) : 0;
  stats["ERROR"] = log_counts_.count(Level::ERROR) ? log_counts_.at(Level::ERROR) : 0;
  stats["Total"] = stats["DEBUG"] + stats["INFO"] + stats["WARNING"] + stats["ERROR"];
  
  return stats;
}

}  // namespace anywp_engine

