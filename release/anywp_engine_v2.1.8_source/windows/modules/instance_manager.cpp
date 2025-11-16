// CRITICAL: Include anywp_engine_plugin.h FIRST to get full type definitions
#include "../utils/logger.h"
#include <iostream>

// This header defines MonitorInfo and WallpaperInstance
#include "../anywp_engine_plugin.h"

// Now include our own header
#include "instance_manager.h"

namespace anywp_engine {

InstanceManager::InstanceManager()
    : instances_ref_(nullptr)
    , monitors_ref_(nullptr)
    , instances_mutex_ref_(nullptr) {
  Logger::Instance().Info("InstanceManager", "Module initialized");
}

InstanceManager::~InstanceManager() {
  Logger::Instance().Info("InstanceManager", "Module destroyed");
}

void InstanceManager::Initialize(
    RemoveMouseHookCallback remove_mouse_hook,
    ClearDefaultUrlCallback clear_default_url,
    UntrackWindowCallback untrack_window,
    std::vector<WallpaperInstance>* instances_ref,
    std::vector<MonitorInfo>* monitors_ref,
    std::mutex* instances_mutex_ref) {
  
  remove_mouse_hook_ = remove_mouse_hook;
  clear_default_url_ = clear_default_url;
  untrack_window_ = untrack_window;
  instances_ref_ = instances_ref;
  monitors_ref_ = monitors_ref;
  instances_mutex_ref_ = instances_mutex_ref;
  
  Logger::Instance().Info("InstanceManager", "Callbacks initialized");
}

WallpaperInstance* InstanceManager::GetInstanceForMonitor(int monitor_index) {
  if (!instances_ref_ || !instances_mutex_ref_) {
    Logger::Instance().Error("InstanceManager", "References not initialized");
    return nullptr;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  
  for (auto& instance : *instances_ref_) {
    if (instance.monitor_index == monitor_index) {
      return &instance;
    }
  }
  
  return nullptr;
}

WallpaperInstance* InstanceManager::GetInstanceAtPoint(int x, int y) {
  if (!instances_ref_ || !monitors_ref_ || !instances_mutex_ref_) {
    Logger::Instance().Error("InstanceManager", "References not initialized");
    return nullptr;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  
  for (auto& instance : *instances_ref_) {
    // Find which monitor this instance belongs to
    for (const auto& monitor : *monitors_ref_) {
      if (monitor.index == instance.monitor_index) {
        int right = monitor.left + monitor.width;
        int bottom = monitor.top + monitor.height;
        
        if (x >= monitor.left && x < right && y >= monitor.top && y < bottom) {
          return &instance;
        }
        break;
      }
    }
  }
  
  return nullptr;
}

bool InstanceManager::HasInstance(int monitor_index) const {
  if (!instances_ref_ || !instances_mutex_ref_) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  
  for (const auto& instance : *instances_ref_) {
    if (instance.monitor_index == monitor_index) {
      return true;
    }
  }
  
  return false;
}

size_t InstanceManager::GetInstanceCount() const {
  if (!instances_ref_ || !instances_mutex_ref_) {
    return 0;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  return instances_ref_->size();
}

bool InstanceManager::IsEmpty() const {
  return GetInstanceCount() == 0;
}

void InstanceManager::AddInstance(const WallpaperInstance& instance) {
  if (!instances_ref_ || !instances_mutex_ref_) {
    Logger::Instance().Error("InstanceManager", "Cannot add instance: references not initialized");
    return;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  instances_ref_->push_back(instance);
  
  Logger::Instance().Info("InstanceManager", 
    "Added instance for monitor " + std::to_string(instance.monitor_index) +
    " (Total: " + std::to_string(instances_ref_->size()) + ")");
}

bool InstanceManager::RemoveInstance(int monitor_index) {
  if (!instances_ref_ || !instances_mutex_ref_) {
    Logger::Instance().Error("InstanceManager", "Cannot remove instance: references not initialized");
    return false;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  
  auto original_size = instances_ref_->size();
  instances_ref_->erase(
    std::remove_if(instances_ref_->begin(), instances_ref_->end(),
      [monitor_index](const WallpaperInstance& inst) {
        return inst.monitor_index == monitor_index;
      }),
    instances_ref_->end()
  );
  
  bool removed = (instances_ref_->size() < original_size);
  if (removed) {
    Logger::Instance().Info("InstanceManager",
      "Removed instance for monitor " + std::to_string(monitor_index) +
      " (Remaining: " + std::to_string(instances_ref_->size()) + ")");
    
    // Trigger cleanup callbacks if all instances are gone
    if (instances_ref_->empty()) {
      Logger::Instance().Info("InstanceManager", "All instances removed, triggering cleanup");
      if (remove_mouse_hook_) {
        remove_mouse_hook_();
      }
      if (clear_default_url_) {
        clear_default_url_();
      }
    }
  }
  
  return removed;
}

void InstanceManager::ClearAllInstances() {
  if (!instances_ref_ || !instances_mutex_ref_) {
    Logger::Instance().Error("InstanceManager", "Cannot clear instances: references not initialized");
    return;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  instances_ref_->clear();
  
  Logger::Instance().Info("InstanceManager", "All instances cleared");
}

bool InstanceManager::CleanupInstance(int monitor_index) {
  std::cout << "[InstanceManager] Cleaning up instance for monitor " << monitor_index << std::endl;
  
  WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
  if (!instance) {
    std::cout << "[InstanceManager] No instance found on monitor " << monitor_index << std::endl;
    return false;
  }
  
  // Close WebView
  if (instance->webview_controller) {
    try {
      instance->webview_controller->Close();
      std::cout << "[InstanceManager] WebView controller closed" << std::endl;
    } catch (...) {
      std::cout << "[InstanceManager] WARNING: Exception while closing WebView controller" << std::endl;
    }
    instance->webview_controller = nullptr;
  }
  
  instance->webview = nullptr;
  
  // Destroy window
  if (instance->webview_host_hwnd) {
    if (IsWindow(instance->webview_host_hwnd)) {
      if (untrack_window_) {
        untrack_window_(instance->webview_host_hwnd);
      }
      
      if (!DestroyWindow(instance->webview_host_hwnd)) {
        DWORD error = GetLastError();
        std::cout << "[InstanceManager] WARNING: Failed to destroy window, error: " << error << std::endl;
      } else {
        std::cout << "[InstanceManager] Window destroyed successfully" << std::endl;
        Sleep(50); // Wait for window destruction
      }
    } else {
      std::cout << "[InstanceManager] WARNING: Window already destroyed" << std::endl;
      if (untrack_window_) {
        untrack_window_(instance->webview_host_hwnd);
      }
    }
    instance->webview_host_hwnd = nullptr;
  }
  
  // Clear iframe data
  instance->iframes.clear();
  
  // Remove instance from list
  bool removed = RemoveInstance(monitor_index);
  
  std::cout << "[InstanceManager] Cleanup " << (removed ? "succeeded" : "failed") 
            << " for monitor " << monitor_index << std::endl;
  
  return removed;
}

}  // namespace anywp_engine

