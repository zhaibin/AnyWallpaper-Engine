#include "event_dispatcher.h"
#include "../anywp_engine_plugin.h"
#include "../utils/logger.h"
#include <iostream>
#include <sstream>

namespace anywp_engine {

EventDispatcher::EventDispatcher() {
  Logger::Instance().Info("EventDispatcher", "Module created");
}

EventDispatcher::~EventDispatcher() {
  Logger::Instance().Info("EventDispatcher", "Module destroyed");
  
  // Log final statistics
  auto stats = GetEventStats();
  if (total_events_.load() > 0) {
    double cache_hit_rate = (cache_hits_.load() * 100.0) / total_events_.load();
    
    std::ostringstream oss;
    oss << "Final statistics: "
        << "total=" << total_events_.load()
        << " cache_hits=" << cache_hits_.load()
        << " cache_misses=" << cache_misses_.load()
        << " hit_rate=" << cache_hit_rate << "%";
    Logger::Instance().Info("EventDispatcher", oss.str());
  }
}

void EventDispatcher::Initialize(
    std::vector<WallpaperInstance>* instances,
    std::vector<MonitorInfo>* monitors,
    std::mutex* instances_mutex) {
  
  if (!instances || !monitors || !instances_mutex) {
    Logger::Instance().Error("EventDispatcher", "Invalid initialization parameters");
    return;
  }
  
  instances_ = instances;
  monitors_ = monitors;
  instances_mutex_ = instances_mutex;
  
  // Build initial cache
  RebuildHwndCache();
  
  Logger::Instance().Info("EventDispatcher", "Module initialized successfully");
}

void EventDispatcher::RebuildHwndCache() {
  std::lock_guard<std::mutex> cache_lock(cache_mutex_);
  
  // Clear existing cache
  hwnd_cache_.clear();
  
  if (!instances_ || !instances_mutex_) {
    Logger::Instance().Warning("EventDispatcher", "Cannot rebuild cache: not initialized");
    return;
  }
  
  // Rebuild cache from current instances
  std::lock_guard<std::mutex> instances_lock(*instances_mutex_);
  
  for (auto& instance : *instances_) {
    if (instance.webview_host_hwnd) {
      hwnd_cache_[instance.webview_host_hwnd] = &instance;
    }
  }
  
  std::ostringstream oss;
  oss << "Cache rebuilt: " << hwnd_cache_.size() << " entries";
  Logger::Instance().Info("EventDispatcher", oss.str());
}

WallpaperInstance* EventDispatcher::FindInstanceByPoint(int x, int y) {
  total_events_.fetch_add(1, std::memory_order_relaxed);
  
  if (!instances_ || !monitors_ || !instances_mutex_) {
    Logger::Instance().Error("EventDispatcher", "FindInstanceByPoint: not initialized");
    return nullptr;
  }
  
  // First, find which monitor contains the point
  MonitorInfo* target_monitor = FindMonitorAtPoint(x, y);
  if (!target_monitor) {
    cache_misses_.fetch_add(1, std::memory_order_relaxed);
    return nullptr;
  }
  
  // Try cache lookup first (O(1) performance)
  {
    std::lock_guard<std::mutex> cache_lock(cache_mutex_);
    std::lock_guard<std::mutex> instances_lock(*instances_mutex_);
    
    // Find instance for this monitor
    for (auto& instance : *instances_) {
      if (instance.monitor_index == target_monitor->index) {
        // Check if in cache
        auto it = hwnd_cache_.find(instance.webview_host_hwnd);
        if (it != hwnd_cache_.end()) {
          cache_hits_.fetch_add(1, std::memory_order_relaxed);
          return it->second;
        }
        
        // Not in cache, add it
        if (instance.webview_host_hwnd) {
          hwnd_cache_[instance.webview_host_hwnd] = &instance;
          cache_hits_.fetch_add(1, std::memory_order_relaxed);
          return &instance;
        }
      }
    }
  }
  
  cache_misses_.fetch_add(1, std::memory_order_relaxed);
  return nullptr;
}

MonitorInfo* EventDispatcher::FindMonitorAtPoint(int x, int y) {
  if (!monitors_) {
    return nullptr;
  }
  
  // No lock needed - monitors_ is only modified during initialization/display change
  for (auto& monitor : *monitors_) {
    int left = monitor.left;
    int top = monitor.top;
    int right = left + monitor.width;
    int bottom = top + monitor.height;
    
    if (x >= left && x < right && y >= top && y < bottom) {
      return &monitor;
    }
  }
  
  return nullptr;
}

void EventDispatcher::DispatchMouseEvent(int x, int y, const char* event_type) {
  // Find target instance
  WallpaperInstance* instance = FindInstanceByPoint(x, y);
  
  // Create event structure
  MouseEvent event;
  event.x = x;
  event.y = y;
  event.event_type = event_type;
  event.timestamp = std::chrono::steady_clock::now();
  event.target_instance = instance;
  
  // Dispatch
  DispatchEventInternal(event);
  
  // Update statistics
  UpdateStats(event_type);
}

void EventDispatcher::DispatchBatchEvents(const std::vector<MouseEvent>& events) {
  for (const auto& event : events) {
    DispatchEventInternal(event);
    UpdateStats(event.event_type);
  }
  
  Logger::Instance().Info("EventDispatcher", 
    "Dispatched batch of " + std::to_string(events.size()) + " events");
}

void EventDispatcher::DispatchEventInternal(const MouseEvent& event) {
  Microsoft::WRL::ComPtr<ICoreWebView2> target_webview;
  
  // Get target webview
  if (event.target_instance && event.target_instance->webview) {
    target_webview = event.target_instance->webview;
  } else if (legacy_webview_) {
    // Fallback to legacy webview
    target_webview = legacy_webview_;
  } else {
    // No target available
    LogEvent(event);
    return;
  }
  
  // Build JSON message
  std::wstringstream json;
  json << L"{"
       << L"\"type\":\"mouseEvent\","
       << L"\"eventType\":\"" << std::wstring(event.event_type, event.event_type + strlen(event.event_type)) << L"\","
       << L"\"x\":" << event.x << L","
       << L"\"y\":" << event.y << L","
       << L"\"button\":0"
       << L"}";
  
  // Send via WebMessage
  try {
    HRESULT hr = target_webview->PostWebMessageAsJson(json.str().c_str());
    
    if (FAILED(hr)) {
      // Only log errors for non-mousemove events
      if (strcmp(event.event_type, "mousemove") != 0) {
        std::ostringstream oss;
        oss << "PostWebMessage failed: 0x" << std::hex << hr;
        Logger::Instance().Error("EventDispatcher", oss.str());
      }
    } else {
      // Log success with throttling
      LogEvent(event);
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("EventDispatcher", 
      std::string("Exception in DispatchEventInternal: ") + e.what());
  } catch (...) {
    Logger::Instance().Error("EventDispatcher", 
      "Unknown exception in DispatchEventInternal");
  }
}

void EventDispatcher::LogEvent(const MouseEvent& event) {
  int count = event_count_.fetch_add(1, std::memory_order_relaxed);
  int throttle = log_throttle_.load(std::memory_order_relaxed);
  
  // Skip logging if throttled (except for mousedown/mouseup)
  bool is_mousemove = (strcmp(event.event_type, "mousemove") == 0);
  if (is_mousemove && throttle > 0 && (count % throttle) != 0) {
    return;  // Throttled
  }
  
  // Log event
  std::ostringstream oss;
  oss << "Event #" << count << ": " << event.event_type
      << " at (" << event.x << "," << event.y << ")"
      << " target=" << (event.target_instance ? "found" : "none");
  
  if (is_mousemove) {
    Logger::Instance().Debug("EventDispatcher", oss.str());
  } else {
    Logger::Instance().Info("EventDispatcher", oss.str());
  }
}

void EventDispatcher::UpdateStats(const char* event_type) {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  event_stats_[event_type]++;
}

void EventDispatcher::SetLogThrottle(int every_n_events) {
  log_throttle_.store(every_n_events, std::memory_order_relaxed);
  
  Logger::Instance().Info("EventDispatcher", 
    "Log throttle set to: " + std::to_string(every_n_events));
}

int EventDispatcher::GetLogThrottle() const {
  return log_throttle_.load(std::memory_order_relaxed);
}

std::unordered_map<std::string, uint64_t> EventDispatcher::GetEventStats() const {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  return event_stats_;
}

void EventDispatcher::ResetStats() {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  event_stats_.clear();
  event_count_.store(0, std::memory_order_relaxed);
  total_events_.store(0, std::memory_order_relaxed);
  cache_hits_.store(0, std::memory_order_relaxed);
  cache_misses_.store(0, std::memory_order_relaxed);
  
  Logger::Instance().Info("EventDispatcher", "Statistics reset");
}

void EventDispatcher::SetLegacyWebView(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    HWND webview_host_hwnd) {
  
  legacy_webview_ = webview;
  legacy_webview_host_hwnd_ = webview_host_hwnd;
  
  Logger::Instance().Info("EventDispatcher", "Legacy WebView set");
}

}  // namespace anywp_engine

