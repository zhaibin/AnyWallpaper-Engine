#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_RETRY_HANDLER_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_RETRY_HANDLER_H_

#include "logger.h"
#include <functional>
#include <chrono>
#include <thread>
#include <exception>
#include <string>

namespace anywp_engine {

// Retry handler with exponential backoff for transient failures
class RetryHandler {
public:
  // Retry policy configuration
  struct RetryPolicy {
    int max_attempts = 3;
    std::chrono::milliseconds initial_delay = std::chrono::milliseconds(100);
    double backoff_multiplier = 2.0;
    std::chrono::milliseconds max_delay = std::chrono::milliseconds(5000);
    bool log_attempts = true;
    
    RetryPolicy() = default;
    
    RetryPolicy(int attempts, std::chrono::milliseconds delay)
        : max_attempts(attempts), initial_delay(delay) {}
  };
  
  // Execute function with retry logic (returns bool)
  // func should return true on success, false on failure
  template<typename Func>
  static bool Execute(
      Func func,
      const std::string& operation_name = "operation",
      const RetryPolicy& policy = RetryPolicy()) {
    
    std::chrono::milliseconds current_delay = policy.initial_delay;
    
    for (int attempt = 1; attempt <= policy.max_attempts; ++attempt) {
      try {
        if (policy.log_attempts && attempt > 1) {
          Logger::Instance().Info("RetryHandler",
            operation_name + " attempt " + std::to_string(attempt) + 
            "/" + std::to_string(policy.max_attempts));
        }
        
        // Execute function
        bool success = func();
        
        if (success) {
          if (policy.log_attempts && attempt > 1) {
            Logger::Instance().Info("RetryHandler",
              operation_name + " succeeded on attempt " + std::to_string(attempt));
          }
          return true;
        }
        
        // Function returned false - retry if attempts remaining
        if (attempt < policy.max_attempts) {
          if (policy.log_attempts) {
            Logger::Instance().Warn("RetryHandler",
              operation_name + " failed on attempt " + std::to_string(attempt) +
              ", retrying in " + std::to_string(current_delay.count()) + "ms");
          }
          
          std::this_thread::sleep_for(current_delay);
          
          // Exponential backoff
          current_delay = std::min(
              std::chrono::milliseconds(
                  static_cast<long long>(current_delay.count() * policy.backoff_multiplier)),
              policy.max_delay);
        }
      }
      catch (const std::exception& e) {
        if (policy.log_attempts) {
          Logger::Instance().Warn("RetryHandler",
            operation_name + " threw exception on attempt " + 
            std::to_string(attempt) + ": " + e.what());
        }
        
        if (attempt < policy.max_attempts) {
          std::this_thread::sleep_for(current_delay);
          current_delay = std::min(
              std::chrono::milliseconds(
                  static_cast<long long>(current_delay.count() * policy.backoff_multiplier)),
              policy.max_delay);
        }
      }
      catch (...) {
        if (policy.log_attempts) {
          Logger::Instance().Warn("RetryHandler",
            operation_name + " threw unknown exception on attempt " + 
            std::to_string(attempt));
        }
        
        if (attempt < policy.max_attempts) {
          std::this_thread::sleep_for(current_delay);
          current_delay = std::min(
              std::chrono::milliseconds(
                  static_cast<long long>(current_delay.count() * policy.backoff_multiplier)),
              policy.max_delay);
        }
      }
    }
    
    // All attempts failed
    if (policy.log_attempts) {
      Logger::Instance().Error("RetryHandler",
        operation_name + " failed after " + 
        std::to_string(policy.max_attempts) + " attempts");
    }
    
    return false;
  }
  
  // Execute function with retry logic (returns optional value)
  template<typename T, typename Func>
  static bool ExecuteWithResult(
      Func func,
      T& result,
      const std::string& operation_name = "operation",
      const RetryPolicy& policy = RetryPolicy()) {
    
    std::chrono::milliseconds current_delay = policy.initial_delay;
    
    for (int attempt = 1; attempt <= policy.max_attempts; ++attempt) {
      try {
        if (policy.log_attempts && attempt > 1) {
          Logger::Instance().Info("RetryHandler",
            operation_name + " attempt " + std::to_string(attempt) + 
            "/" + std::to_string(policy.max_attempts));
        }
        
        // Execute function (returns optional or pair<bool, T>)
        auto exec_result = func();
        
        if (exec_result.first) {  // Success
          result = exec_result.second;
          
          if (policy.log_attempts && attempt > 1) {
            Logger::Instance().Info("RetryHandler",
              operation_name + " succeeded on attempt " + std::to_string(attempt));
          }
          return true;
        }
        
        // Function returned failure - retry if attempts remaining
        if (attempt < policy.max_attempts) {
          if (policy.log_attempts) {
            Logger::Instance().Warn("RetryHandler",
              operation_name + " failed on attempt " + std::to_string(attempt));
          }
          
          std::this_thread::sleep_for(current_delay);
          current_delay = std::min(
              std::chrono::milliseconds(
                  static_cast<long long>(current_delay.count() * policy.backoff_multiplier)),
              policy.max_delay);
        }
      }
      catch (const std::exception& e) {
        if (policy.log_attempts) {
          Logger::Instance().Warn("RetryHandler",
            operation_name + " threw exception on attempt " + 
            std::to_string(attempt) + ": " + e.what());
        }
        
        if (attempt < policy.max_attempts) {
          std::this_thread::sleep_for(current_delay);
          current_delay = std::min(
              std::chrono::milliseconds(
                  static_cast<long long>(current_delay.count() * policy.backoff_multiplier)),
              policy.max_delay);
        }
      }
    }
    
    // All attempts failed
    if (policy.log_attempts) {
      Logger::Instance().Error("RetryHandler",
        operation_name + " failed after " + 
        std::to_string(policy.max_attempts) + " attempts");
    }
    
    return false;
  }
  
  // Simple retry with default policy
  template<typename Func>
  static bool ExecuteSimple(Func func, const std::string& operation_name = "operation") {
    return Execute(func, operation_name, RetryPolicy());
  }
  
  // Retry with custom attempts and delay
  template<typename Func>
  static bool ExecuteCustom(
      Func func,
      const std::string& operation_name,
      int max_attempts,
      std::chrono::milliseconds initial_delay) {
    
    return Execute(func, operation_name, RetryPolicy(max_attempts, initial_delay));
  }
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_RETRY_HANDLER_H_

