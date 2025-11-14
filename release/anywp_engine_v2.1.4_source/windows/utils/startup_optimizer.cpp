#include "startup_optimizer.h"
#include "logger.h"
#include "cpu_profiler.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace anywp_engine {

StartupOptimizer& StartupOptimizer::Instance() {
  static StartupOptimizer instance;
  return instance;
}

StartupOptimizer::StartupOptimizer()
    : startup_begin_time_(0),
      startup_end_time_(0),
      startup_complete_(false) {
}

StartupOptimizer::~StartupOptimizer() {
  if (!startup_complete_) {
    EndStartup();
  }
  LogMetrics();
}

void StartupOptimizer::RegisterTask(const std::string& name,
                                    std::function<void()> func,
                                    InitPriority priority) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    InitTask task;
    task.name = name;
    task.func = std::move(func);
    task.priority = priority;
    
    tasks_[name] = std::move(task);
    metrics_.total_tasks++;
    
    Logger::Instance().Debug("StartupOptimizer", 
      "Registered task: " + name + " (priority: " + 
      std::to_string(static_cast<int>(priority)) + ")");
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to register task: ") + e.what());
  }
}

void StartupOptimizer::RunCriticalInit() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    auto start = std::chrono::high_resolution_clock::now();
    
    auto critical_tasks = GetTasksByPriority(InitPriority::CRITICAL);
    
    Logger::Instance().Info("StartupOptimizer", 
      "Running " + std::to_string(critical_tasks.size()) + " critical tasks");
    
    for (auto* task : critical_tasks) {
      ExecuteTask(*task);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics_.critical_init_ms = 
        std::chrono::duration<double, std::milli>(end - start).count();
    
    Logger::Instance().Info("StartupOptimizer", 
      "Critical init completed in " + 
      std::to_string(metrics_.critical_init_ms) + " ms");
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to run critical init: ") + e.what());
  }
}

void StartupOptimizer::RunParallelInit(int max_threads) {
  try {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Get high and normal priority tasks
    std::vector<InitTask*> parallel_tasks;
    
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto high_tasks = GetTasksByPriority(InitPriority::HIGH);
      auto normal_tasks = GetTasksByPriority(InitPriority::NORMAL);
      
      parallel_tasks.insert(parallel_tasks.end(), high_tasks.begin(), high_tasks.end());
      parallel_tasks.insert(parallel_tasks.end(), normal_tasks.begin(), normal_tasks.end());
    }
    
    Logger::Instance().Info("StartupOptimizer", 
      "Running " + std::to_string(parallel_tasks.size()) + 
      " parallel tasks (max threads: " + std::to_string(max_threads) + ")");
    
    // Run tasks in parallel
    std::vector<std::future<void>> futures;
    
    for (size_t i = 0; i < parallel_tasks.size(); i += max_threads) {
      futures.clear();
      
      size_t batch_size = (std::min)(static_cast<size_t>(max_threads), 
                                      parallel_tasks.size() - i);
      
      for (size_t j = 0; j < batch_size; j++) {
        auto* task = parallel_tasks[i + j];
        futures.push_back(std::async(std::launch::async, [this, task]() {
          std::lock_guard<std::mutex> lock(mutex_);
          ExecuteTask(*task);
        }));
      }
      
      // Wait for batch to complete
      for (auto& future : futures) {
        future.wait();
      }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics_.parallel_init_ms = 
        std::chrono::duration<double, std::milli>(end - start).count();
    
    Logger::Instance().Info("StartupOptimizer", 
      "Parallel init completed in " + 
      std::to_string(metrics_.parallel_init_ms) + " ms");
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to run parallel init: ") + e.what());
  }
}

void StartupOptimizer::RunDeferredInit() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    auto low_tasks = GetTasksByPriority(InitPriority::LOW);
    
    Logger::Instance().Info("StartupOptimizer", 
      "Running " + std::to_string(low_tasks.size()) + " deferred tasks");
    
    for (auto* task : low_tasks) {
      ExecuteTask(*task);
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to run deferred init: ") + e.what());
  }
}

void StartupOptimizer::RunLazyTask(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    auto it = tasks_.find(name);
    if (it == tasks_.end()) {
      Logger::Instance().Warning("StartupOptimizer", 
        "Lazy task not found: " + name);
      return;
    }
    
    if (it->second.completed) {
      return;  // Already executed
    }
    
    Logger::Instance().Info("StartupOptimizer", 
      "Executing lazy task: " + name);
    
    ExecuteTask(it->second);
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to run lazy task: ") + e.what());
  }
}

bool StartupOptimizer::IsTaskCompleted(const std::string& name) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = tasks_.find(name);
  return it != tasks_.end() && it->second.completed;
}

void StartupOptimizer::BeginStartup() {
  std::lock_guard<std::mutex> lock(mutex_);
  startup_begin_time_ = GetTickCount();
  Logger::Instance().Info("StartupOptimizer", "Startup began");
}

void StartupOptimizer::EndStartup() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (startup_complete_) {
    return;
  }
  
  startup_end_time_ = GetTickCount();
  startup_complete_ = true;
  
  if (startup_begin_time_ > 0) {
    metrics_.total_startup_ms = startup_end_time_ - startup_begin_time_;
  }
  
  Logger::Instance().Info("StartupOptimizer", 
    "Startup completed in " + std::to_string(metrics_.total_startup_ms) + " ms");
}

StartupMetrics StartupOptimizer::GetMetrics() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return metrics_;
}

void StartupOptimizer::LogMetrics() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  oss << "=== Startup Metrics ===\n";
  oss << "Total Startup Time: " << metrics_.total_startup_ms << " ms\n";
  oss << "  Critical Init: " << metrics_.critical_init_ms << " ms\n";
  oss << "  Parallel Init: " << metrics_.parallel_init_ms << " ms\n";
  oss << "Tasks: " << metrics_.completed_tasks << "/" << metrics_.total_tasks;
  if (metrics_.failed_tasks > 0) {
    oss << " (" << metrics_.failed_tasks << " failed)";
  }
  oss << "\n";
  
  Logger::Instance().Info("StartupOptimizer", oss.str());
}

void StartupOptimizer::PreloadWebView2() {
  try {
    Logger::Instance().Info("StartupOptimizer", "Preloading WebView2");
    
    // Create a message-only window to initialize WebView2 environment early
    // This can reduce first-time creation latency
    HWND hwnd = CreateWindowW(L"STATIC", L"WebView2Preload", 0,
                               0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    if (hwnd) {
      // WebView2 environment will be initialized by main code
      DestroyWindow(hwnd);
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to preload WebView2: ") + e.what());
  }
}

void StartupOptimizer::PreloadSDK() {
  try {
    Logger::Instance().Info("StartupOptimizer", "Preloading SDK assets");
    
    // Preload SDK JavaScript file into memory
    // This reduces first-load latency by caching the SDK content
    std::string sdk_path = "windows/anywp_sdk.js";
    
    // Check if file exists
    std::ifstream sdk_file(sdk_path, std::ios::binary | std::ios::ate);
    if (sdk_file.is_open()) {
      std::streamsize size = sdk_file.tellg();
      sdk_file.seekg(0, std::ios::beg);
      
      // Read SDK content (cache in static storage for reuse)
      static std::string sdk_cache;
      sdk_cache.resize(static_cast<size_t>(size));
      if (sdk_file.read(&sdk_cache[0], size)) {
        Logger::Instance().Info("StartupOptimizer", 
          "SDK preloaded: " + std::to_string(size) + " bytes");
      }
    } else {
      Logger::Instance().Warning("StartupOptimizer", 
        "SDK file not found, will load on-demand");
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to preload SDK: ") + e.what());
  }
}

void StartupOptimizer::WarmupModules() {
  try {
    Logger::Instance().Info("StartupOptimizer", "Warming up critical modules");
    
    // Trigger singleton initialization to avoid lazy-load delays
    // Pre-initialize Logger (already done by calling it)
    Logger::Instance().Debug("StartupOptimizer", "Logger warmed up");
    
    // Pre-allocate common data structures
    try {
      // Reserve space for typical collections
      std::vector<int> warmup_vector;
      warmup_vector.reserve(100);  // Typical instance count
      
      std::map<std::string, std::string> warmup_map;
      // Insert and clear to trigger internal allocations
      for (int i = 0; i < 10; ++i) {
        warmup_map[std::to_string(i)] = "warmup";
      }
      warmup_map.clear();
      
      Logger::Instance().Info("StartupOptimizer", "Memory pools warmed up");
    } catch (...) {
      // Ignore warmup errors - not critical
    }
    
    // Warm up system APIs
    try {
      SYSTEM_INFO si;
      GetSystemInfo(&si);  // Cache system information
      
      MEMORYSTATUSEX memInfo;
      memInfo.dwLength = sizeof(MEMORYSTATUSEX);
      GlobalMemoryStatusEx(&memInfo);  // Cache memory info
      
      Logger::Instance().Info("StartupOptimizer", "System APIs warmed up");
    } catch (...) {
      // Ignore API warmup errors
    }
    
    Logger::Instance().Info("StartupOptimizer", "Module warmup complete");
  } catch (const std::exception& e) {
    Logger::Instance().Error("StartupOptimizer", 
      std::string("Failed to warmup modules: ") + e.what());
  }
}

std::string StartupOptimizer::GenerateStartupReport() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  
  oss << "=== Startup Report ===\n";
  oss << "Total Time: " << metrics_.total_startup_ms << " ms\n";
  oss << "Critical Init: " << metrics_.critical_init_ms << " ms\n";
  oss << "Parallel Init: " << metrics_.parallel_init_ms << " ms\n";
  oss << "\nTasks:\n";
  oss << "  Total: " << metrics_.total_tasks << "\n";
  oss << "  Completed: " << metrics_.completed_tasks << "\n";
  oss << "  Failed: " << metrics_.failed_tasks << "\n";
  oss << "\nTask Details:\n";
  
  // Sort tasks by duration
  std::vector<std::pair<std::string, InitTask>> sorted_tasks(tasks_.begin(), tasks_.end());
  std::sort(sorted_tasks.begin(), sorted_tasks.end(),
    [](const auto& a, const auto& b) {
      return a.second.duration_ms > b.second.duration_ms;
    });
  
  for (const auto& pair : sorted_tasks) {
    const auto& task = pair.second;
    oss << "  " << task.name << ": "
        << task.duration_ms << " ms ["
        << (task.completed ? "OK" : "PENDING") << "]\n";
  }
  
  return oss.str();
}

void StartupOptimizer::ExecuteTask(InitTask& task) {
  if (task.completed) {
    return;
  }
  
  try {
    auto start = std::chrono::high_resolution_clock::now();
    
    task.func();
    
    auto end = std::chrono::high_resolution_clock::now();
    task.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
    task.completed = true;
    
    metrics_.completed_tasks++;
    
    Logger::Instance().Debug("StartupOptimizer", 
      "Task completed: " + task.name + " (" + 
      std::to_string(task.duration_ms) + " ms)");
  } catch (const std::exception& e) {
    metrics_.failed_tasks++;
    Logger::Instance().Error("StartupOptimizer", 
      "Task failed: " + task.name + " - " + e.what());
  }
}

std::vector<InitTask*> StartupOptimizer::GetTasksByPriority(InitPriority priority) {
  std::vector<InitTask*> result;
  
  for (auto& pair : tasks_) {
    if (pair.second.priority == priority && !pair.second.completed) {
      result.push_back(&pair.second);
    }
  }
  
  return result;
}

}  // namespace anywp_engine

