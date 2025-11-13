#ifndef ANYWP_ENGINE_STARTUP_OPTIMIZER_H_
#define ANYWP_ENGINE_STARTUP_OPTIMIZER_H_

#include <windows.h>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <mutex>
#include <future>

namespace anywp_engine {

/**
 * @brief Initialization task priority
 */
enum class InitPriority {
  CRITICAL = 0,   // Must complete before startup
  HIGH = 1,       // Should complete early
  NORMAL = 2,     // Can complete during startup
  LOW = 3,        // Can be deferred
  LAZY = 4        // Load on first use
};

/**
 * @brief Initialization task
 */
struct InitTask {
  std::string name;
  std::function<void()> func;
  InitPriority priority;
  bool completed = false;
  double duration_ms = 0.0;
};

/**
 * @brief Startup performance metrics
 */
struct StartupMetrics {
  double total_startup_ms = 0.0;
  double critical_init_ms = 0.0;
  double parallel_init_ms = 0.0;
  int total_tasks = 0;
  int completed_tasks = 0;
  int failed_tasks = 0;
};

/**
 * @brief Startup optimizer for faster initialization
 */
class StartupOptimizer {
 public:
  static StartupOptimizer& Instance();
  
  // Task registration
  void RegisterTask(const std::string& name, 
                   std::function<void()> func, 
                   InitPriority priority);
  
  // Initialization execution
  void RunCriticalInit();
  void RunParallelInit(int max_threads = 4);
  void RunDeferredInit();
  
  // Lazy loading
  void RunLazyTask(const std::string& name);
  bool IsTaskCompleted(const std::string& name) const;
  
  // Startup tracking
  void BeginStartup();
  void EndStartup();
  StartupMetrics GetMetrics() const;
  void LogMetrics();
  
  // Preloading
  void PreloadWebView2();
  void PreloadSDK();
  void WarmupModules();
  
  // Reporting
  std::string GenerateStartupReport();
  
 private:
  StartupOptimizer();
  ~StartupOptimizer();
  
  StartupOptimizer(const StartupOptimizer&) = delete;
  StartupOptimizer& operator=(const StartupOptimizer&) = delete;
  
  // Internal helpers
  void ExecuteTask(InitTask& task);
  std::vector<InitTask*> GetTasksByPriority(InitPriority priority);
  
  mutable std::mutex mutex_;
  std::map<std::string, InitTask> tasks_;
  
  DWORD startup_begin_time_;
  DWORD startup_end_time_;
  bool startup_complete_;
  
  StartupMetrics metrics_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_STARTUP_OPTIMIZER_H_

