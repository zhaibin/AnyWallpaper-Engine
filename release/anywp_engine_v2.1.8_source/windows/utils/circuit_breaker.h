#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_CIRCUIT_BREAKER_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_CIRCUIT_BREAKER_H_

#include "logger.h"
#include <chrono>
#include <mutex>
#include <string>

namespace anywp_engine {

// Circuit breaker pattern for preventing cascading failures
// 
// States:
// - CLOSED: Normal operation, requests pass through
// - OPEN: Failure threshold exceeded, requests are blocked
// - HALF_OPEN: Testing if system recovered, limited requests allowed
class CircuitBreaker {
public:
  enum class State {
    CLOSED,     // Normal operation
    OPEN,       // Blocking requests due to failures
    HALF_OPEN   // Testing recovery
  };
  
  // Configuration
  struct Config {
    int failure_threshold = 5;                              // Failures before opening
    std::chrono::seconds timeout = std::chrono::seconds(30); // Time before trying again
    int success_threshold = 2;                              // Successes to close from half-open
    bool log_state_changes = true;
    
    Config() = default;
    
    Config(int failures, std::chrono::seconds timeout_duration)
        : failure_threshold(failures), timeout(timeout_duration) {}
  };
  
  CircuitBreaker(const std::string& name, const Config& config = Config())
      : name_(name),
        config_(config),
        state_(State::CLOSED),
        failure_count_(0),
        success_count_(0),
        last_failure_time_(std::chrono::steady_clock::now()) {}
  
  // Check if operation can execute
  bool CanExecute() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
      case State::CLOSED:
        return true;
        
      case State::OPEN: {
        // Check if timeout has elapsed
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - last_failure_time_);
        
        if (elapsed >= config_.timeout) {
          // Try half-open state
          TransitionTo(State::HALF_OPEN);
          return true;
        }
        
        return false;
      }
      
      case State::HALF_OPEN:
        // Allow limited requests to test recovery
        return true;
    }
    
    return false;
  }
  
  // Record successful operation
  void RecordSuccess() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
      case State::CLOSED:
        // Reset failure count on success
        failure_count_ = 0;
        break;
        
      case State::HALF_OPEN:
        success_count_++;
        
        if (success_count_ >= config_.success_threshold) {
          // Recovered - transition to closed
          failure_count_ = 0;
          success_count_ = 0;
          TransitionTo(State::CLOSED);
        }
        break;
        
      case State::OPEN:
        // Shouldn't happen, but reset if it does
        break;
    }
  }
  
  // Record failed operation
  void RecordFailure() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    last_failure_time_ = std::chrono::steady_clock::now();
    
    switch (state_) {
      case State::CLOSED:
        failure_count_++;
        
        if (failure_count_ >= config_.failure_threshold) {
          // Too many failures - open circuit
          TransitionTo(State::OPEN);
        }
        break;
        
      case State::HALF_OPEN:
        // Failed during recovery - back to open
        failure_count_ = config_.failure_threshold;  // Keep it open
        success_count_ = 0;
        TransitionTo(State::OPEN);
        break;
        
      case State::OPEN:
        // Already open, just update time
        break;
    }
  }
  
  // Get current state
  State GetState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
  }
  
  // Get state as string
  std::string GetStateString() const {
    switch (GetState()) {
      case State::CLOSED: return "CLOSED";
      case State::OPEN: return "OPEN";
      case State::HALF_OPEN: return "HALF_OPEN";
      default: return "UNKNOWN";
    }
  }
  
  // Get failure count
  int GetFailureCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return failure_count_;
  }
  
  // Get success count (in half-open state)
  int GetSuccessCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return success_count_;
  }
  
  // Reset circuit breaker to closed state
  void Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != State::CLOSED) {
      failure_count_ = 0;
      success_count_ = 0;
      TransitionTo(State::CLOSED);
    }
  }
  
  // Get name
  const std::string& GetName() const {
    return name_;
  }
  
private:
  // Transition to new state (must hold mutex)
  void TransitionTo(State new_state) {
    if (state_ == new_state) {
      return;
    }
    
    State old_state = state_;
    state_ = new_state;
    
    if (config_.log_state_changes) {
      Logger::Instance().Info("CircuitBreaker",
        name_ + ": " + StateToString(old_state) + " -> " + StateToString(new_state));
    }
  }
  
  // Convert state to string (for logging)
  static std::string StateToString(State state) {
    switch (state) {
      case State::CLOSED: return "CLOSED";
      case State::OPEN: return "OPEN";
      case State::HALF_OPEN: return "HALF_OPEN";
      default: return "UNKNOWN";
    }
  }
  
  std::string name_;
  Config config_;
  State state_;
  int failure_count_;
  int success_count_;
  std::chrono::steady_clock::time_point last_failure_time_;
  mutable std::mutex mutex_;
};

// RAII helper for circuit breaker execution
class CircuitBreakerExecutor {
public:
  CircuitBreakerExecutor(CircuitBreaker& breaker)
      : breaker_(breaker), executed_(false), success_(false) {}
  
  ~CircuitBreakerExecutor() {
    if (executed_) {
      if (success_) {
        breaker_.RecordSuccess();
      } else {
        breaker_.RecordFailure();
      }
    }
  }
  
  // Check if can execute
  bool CanExecute() {
    if (breaker_.CanExecute()) {
      executed_ = true;
      return true;
    }
    return false;
  }
  
  // Mark as success
  void MarkSuccess() {
    success_ = true;
  }
  
  // Mark as failure (default)
  void MarkFailure() {
    success_ = false;
  }
  
private:
  CircuitBreaker& breaker_;
  bool executed_;
  bool success_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_CIRCUIT_BREAKER_H_

