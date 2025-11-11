#include "error_handler.h"
#include "logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

namespace anywp_engine {

ErrorHandler& ErrorHandler::Instance() {
  static ErrorHandler instance;
  return instance;
}

ErrorHandler::ErrorHandler() {
  Logger::Instance().Info("ErrorHandler", "Error handling system initialized");
}

ErrorHandler::~ErrorHandler() {
  // Generate final report
  if (total_errors_.load() > 0 || total_warnings_.load() > 0) {
    std::ostringstream oss;
    oss << "Final error statistics: "
        << "errors=" << total_errors_.load()
        << " warnings=" << total_warnings_.load()
        << " fatals=" << total_fatals_.load();
    Logger::Instance().Info("ErrorHandler", oss.str());
  }
}

void ErrorHandler::Report(
    ErrorLevel level,
    ErrorCategory category,
    const std::string& module,
    const std::string& operation,
    const std::string& message,
    const std::exception* ex) {
  
  auto& instance = Instance();
  
  // Check minimum level
  if (level < instance.min_level_) {
    return;  // Skip logging
  }
  
  // Create error info
  ErrorInfo error;
  error.level = level;
  error.category = category;
  error.module = module;
  error.operation = operation;
  error.message = message;
  error.timestamp = std::chrono::system_clock::now();
  error.count = 1;
  
  if (ex) {
    error.exception_type = typeid(*ex).name();
    error.exception_message = ex->what();
  }
  
  // Report internally
  instance.ReportInternal(error);
}

void ErrorHandler::Report(
    ErrorLevel level,
    const std::string& module,
    const std::string& message,
    const std::exception* ex) {
  
  auto& instance = Instance();
  
  // Auto-detect category from message
  ErrorCategory category = instance.DetectCategory(message);
  
  Report(level, category, module, "operation", message, ex);
}

void ErrorHandler::ReportInternal(const ErrorInfo& error) {
  // Update statistics
  {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Module stats
    error_stats_[error.module]++;
    
    // Level stats
    level_stats_[error.level]++;
    
    // Category stats
    category_stats_[error.category]++;
    
    // Global counters
    switch (error.level) {
      case ErrorLevel::WARNING:
        total_warnings_.fetch_add(1, std::memory_order_relaxed);
        break;
      case ErrorLevel::ERROR:
        total_errors_.fetch_add(1, std::memory_order_relaxed);
        break;
      case ErrorLevel::FATAL:
        total_fatals_.fetch_add(1, std::memory_order_relaxed);
        break;
      default:
        break;
    }
    
    // Add to history
    error_history_.push_back(error);
    
    // Limit history size
    if (error_history_.size() > max_history_size_) {
      error_history_.erase(error_history_.begin());
    }
    
    // Deduplicate if needed
    if (error_history_.size() > 100) {
      DeduplicateErrors();
    }
  }
  
  // Log to Logger system
  std::ostringstream oss;
  oss << "[" << CategoryToString(error.category) << "] "
      << error.operation << ": " << error.message;
  
  if (!error.exception_message.empty()) {
    oss << " (Exception: " << error.exception_message << ")";
  }
  
  switch (error.level) {
    case ErrorLevel::DEBUG:
      Logger::Instance().Debug(error.module, oss.str());
      break;
    case ErrorLevel::INFO:
      Logger::Instance().Info(error.module, oss.str());
      break;
    case ErrorLevel::WARNING:
      Logger::Instance().Warning(error.module, oss.str());
      break;
    case ErrorLevel::ERROR:
    case ErrorLevel::FATAL:
      Logger::Instance().Error(error.module, oss.str());
      break;
  }
  
  // Notify callbacks
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : callbacks_) {
      try {
        pair.second(error);
      } catch (...) {
        // Ignore callback exceptions to prevent recursive errors
      }
    }
  }
}

bool ErrorHandler::TryRecover(
    const std::string& operation,
    std::function<bool()> retry_func,
    int max_retries,
    int retry_delay_ms) {
  
  int attempt = 0;
  
  while (attempt < max_retries) {
    try {
      if (retry_func()) {
        if (attempt > 0) {
          Report(ErrorLevel::INFO, "ErrorHandler",
            operation + " succeeded after " + std::to_string(attempt) + " retries");
        }
        return true;
      }
    } catch (const std::exception& e) {
      Report(ErrorLevel::WARNING, ErrorCategory::UNKNOWN, "ErrorHandler",
        operation, "Retry attempt " + std::to_string(attempt + 1) + " failed", &e);
    } catch (...) {
      Report(ErrorLevel::WARNING, ErrorCategory::UNKNOWN, "ErrorHandler",
        operation, "Retry attempt " + std::to_string(attempt + 1) + " failed (unknown exception)");
    }
    
    attempt++;
    
    if (attempt < max_retries) {
      // Delay before next retry
      std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
    }
  }
  
  // All retries failed
  Report(ErrorLevel::ERROR, ErrorCategory::UNKNOWN, "ErrorHandler",
    operation, "All " + std::to_string(max_retries) + " retry attempts failed");
  
  return false;
}

void ErrorHandler::HandleFatalError(
    const std::string& module,
    const std::string& message,
    bool terminate) {
  
  Report(ErrorLevel::FATAL, ErrorCategory::UNKNOWN, module,
    "Fatal Error", message);
  
  // Generate error report
  auto& instance = Instance();
  std::string report = instance.GenerateReport();
  
  // Log report
  Logger::Instance().Error(module, "Fatal error report:\n" + report);
  
  // Also print to console
  std::cerr << "[FATAL] " << module << ": " << message << std::endl;
  std::cerr << "Error report:\n" << report << std::endl;
  
  if (terminate) {
    std::exit(1);
  }
}

std::map<std::string, int> ErrorHandler::GetErrorStats() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return error_stats_;
}

std::vector<ErrorHandler::ErrorInfo> ErrorHandler::GetErrorHistory(size_t max_count) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (max_count == 0 || max_count >= error_history_.size()) {
    return error_history_;
  }
  
  // Return last max_count errors
  return std::vector<ErrorInfo>(
    error_history_.end() - max_count,
    error_history_.end()
  );
}

std::vector<ErrorHandler::ErrorInfo> ErrorHandler::GetErrorsByModule(const std::string& module) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<ErrorInfo> result;
  for (const auto& error : error_history_) {
    if (error.module == module) {
      result.push_back(error);
    }
  }
  
  return result;
}

std::vector<ErrorHandler::ErrorInfo> ErrorHandler::GetErrorsByLevel(ErrorLevel min_level) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<ErrorInfo> result;
  for (const auto& error : error_history_) {
    if (error.level >= min_level) {
      result.push_back(error);
    }
  }
  
  return result;
}

void ErrorHandler::ResetStats() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  error_stats_.clear();
  level_stats_.clear();
  category_stats_.clear();
  error_history_.clear();
  
  total_errors_.store(0, std::memory_order_relaxed);
  total_warnings_.store(0, std::memory_order_relaxed);
  total_fatals_.store(0, std::memory_order_relaxed);
  
  Logger::Instance().Info("ErrorHandler", "Statistics reset");
}

std::string ErrorHandler::GenerateReport() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::ostringstream oss;
  
  oss << "========== Error Report ==========\n";
  oss << "Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";
  
  // Overall statistics
  oss << "Overall Statistics:\n";
  oss << "  Total Errors: " << total_errors_.load() << "\n";
  oss << "  Total Warnings: " << total_warnings_.load() << "\n";
  oss << "  Total Fatals: " << total_fatals_.load() << "\n\n";
  
  // By module
  oss << "Errors by Module:\n";
  for (const auto& pair : error_stats_) {
    oss << "  " << pair.first << ": " << pair.second << "\n";
  }
  oss << "\n";
  
  // By level
  oss << "Errors by Level:\n";
  for (const auto& pair : level_stats_) {
    oss << "  " << LevelToString(pair.first) << ": " << pair.second << "\n";
  }
  oss << "\n";
  
  // By category
  oss << "Errors by Category:\n";
  for (const auto& pair : category_stats_) {
    oss << "  " << CategoryToString(pair.first) << ": " << pair.second << "\n";
  }
  oss << "\n";
  
  // Recent errors
  oss << "Recent Errors (last 10):\n";
  size_t start = error_history_.size() > 10 ? error_history_.size() - 10 : 0;
  for (size_t i = start; i < error_history_.size(); i++) {
    const auto& error = error_history_[i];
    oss << "  [" << LevelToString(error.level) << "] "
        << error.module << ": " << error.message;
    if (!error.exception_message.empty()) {
      oss << " (" << error.exception_message << ")";
    }
    oss << "\n";
  }
  
  oss << "==================================\n";
  
  return oss.str();
}

std::string ErrorHandler::ExportToJSON() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::ostringstream oss;
  oss << "{\n";
  oss << "  \"total_errors\": " << total_errors_.load() << ",\n";
  oss << "  \"total_warnings\": " << total_warnings_.load() << ",\n";
  oss << "  \"total_fatals\": " << total_fatals_.load() << ",\n";
  oss << "  \"errors\": [\n";
  
  for (size_t i = 0; i < error_history_.size(); i++) {
    const auto& error = error_history_[i];
    oss << "    {\n";
    oss << "      \"level\": \"" << LevelToString(error.level) << "\",\n";
    oss << "      \"category\": \"" << CategoryToString(error.category) << "\",\n";
    oss << "      \"module\": \"" << error.module << "\",\n";
    oss << "      \"operation\": \"" << error.operation << "\",\n";
    oss << "      \"message\": \"" << error.message << "\",\n";
    oss << "      \"exception_type\": \"" << error.exception_type << "\",\n";
    oss << "      \"exception_message\": \"" << error.exception_message << "\",\n";
    oss << "      \"count\": " << error.count << "\n";
    oss << "    }";
    if (i < error_history_.size() - 1) {
      oss << ",";
    }
    oss << "\n";
  }
  
  oss << "  ]\n";
  oss << "}\n";
  
  return oss.str();
}

void ErrorHandler::SetMinLevel(ErrorLevel min_level) {
  min_level_ = min_level;
  Logger::Instance().Info("ErrorHandler",
    "Minimum error level set to: " + LevelToString(min_level));
}

ErrorHandler::ErrorLevel ErrorHandler::GetMinLevel() const {
  return min_level_;
}

void ErrorHandler::SetMaxHistorySize(size_t max_size) {
  std::lock_guard<std::mutex> lock(mutex_);
  max_history_size_ = max_size;
  
  // Trim history if needed
  if (error_history_.size() > max_history_size_) {
    error_history_.erase(
      error_history_.begin(),
      error_history_.begin() + (error_history_.size() - max_history_size_)
    );
  }
  
  Logger::Instance().Info("ErrorHandler",
    "Maximum history size set to: " + std::to_string(max_size));
}

int ErrorHandler::RegisterCallback(std::function<void(const ErrorInfo&)> callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  int id = next_callback_id_++;
  callbacks_[id] = callback;
  
  Logger::Instance().Info("ErrorHandler",
    "Error callback registered with ID: " + std::to_string(id));
  
  return id;
}

void ErrorHandler::UnregisterCallback(int callback_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  callbacks_.erase(callback_id);
  
  Logger::Instance().Info("ErrorHandler",
    "Error callback unregistered: " + std::to_string(callback_id));
}

void ErrorHandler::DeduplicateErrors() {
  // Group errors by (module, operation, message)
  std::map<std::string, std::vector<size_t>> groups;
  
  for (size_t i = 0; i < error_history_.size(); i++) {
    const auto& error = error_history_[i];
    std::string key = error.module + "|" + error.operation + "|" + error.message;
    groups[key].push_back(i);
  }
  
  // Merge duplicates
  std::vector<ErrorInfo> new_history;
  for (const auto& pair : groups) {
    const auto& indices = pair.second;
    if (indices.empty()) continue;
    
    // Keep the first error and merge counts
    ErrorInfo merged = error_history_[indices[0]];
    merged.count = static_cast<int>(indices.size());
    new_history.push_back(merged);
  }
  
  // Replace history with deduplicated version
  error_history_ = new_history;
}

ErrorHandler::ErrorCategory ErrorHandler::DetectCategory(const std::string& message) const {
  // Simple keyword-based detection
  if (message.find("init") != std::string::npos ||
      message.find("Initialize") != std::string::npos) {
    return ErrorCategory::INITIALIZATION;
  }
  
  if (message.find("resource") != std::string::npos ||
      message.find("memory") != std::string::npos ||
      message.find("leak") != std::string::npos) {
    return ErrorCategory::RESOURCE;
  }
  
  if (message.find("network") != std::string::npos ||
      message.find("connect") != std::string::npos ||
      message.find("socket") != std::string::npos) {
    return ErrorCategory::NETWORK;
  }
  
  if (message.find("permission") != std::string::npos ||
      message.find("access denied") != std::string::npos ||
      message.find("security") != std::string::npos) {
    return ErrorCategory::PERMISSION;
  }
  
  if (message.find("invalid state") != std::string::npos ||
      message.find("not initialized") != std::string::npos) {
    return ErrorCategory::INVALID_STATE;
  }
  
  if (message.find("HRESULT") != std::string::npos ||
      message.find("Win32") != std::string::npos ||
      message.find("WebView2") != std::string::npos) {
    return ErrorCategory::EXTERNAL_API;
  }
  
  return ErrorCategory::UNKNOWN;
}

std::string ErrorHandler::LevelToString(ErrorLevel level) {
  switch (level) {
    case ErrorLevel::DEBUG: return "DEBUG";
    case ErrorLevel::INFO: return "INFO";
    case ErrorLevel::WARNING: return "WARNING";
    case ErrorLevel::ERROR: return "ERROR";
    case ErrorLevel::FATAL: return "FATAL";
    default: return "UNKNOWN";
  }
}

std::string ErrorHandler::CategoryToString(ErrorCategory category) {
  switch (category) {
    case ErrorCategory::INITIALIZATION: return "INITIALIZATION";
    case ErrorCategory::RESOURCE: return "RESOURCE";
    case ErrorCategory::NETWORK: return "NETWORK";
    case ErrorCategory::PERMISSION: return "PERMISSION";
    case ErrorCategory::INVALID_STATE: return "INVALID_STATE";
    case ErrorCategory::EXTERNAL_API: return "EXTERNAL_API";
    case ErrorCategory::UNKNOWN: return "UNKNOWN";
    default: return "UNKNOWN";
  }
}

}  // namespace anywp_engine

