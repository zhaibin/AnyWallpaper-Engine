// CRITICAL: Include anywp_engine_plugin.h FIRST to get full type definitions
// before including display_change_coordinator.h which uses those types
#include "../utils/logger.h"
#include "monitor_manager.h"
#include <iostream>

// This header defines MonitorInfo and WallpaperInstance
#include "../anywp_engine_plugin.h"

// Now include our own header (which forward-declares the types above)
#include "display_change_coordinator.h"

namespace anywp_engine {

DisplayChangeCoordinator::DisplayChangeCoordinator()
    : monitors_ref_(nullptr)
    , instances_ref_(nullptr)
    , instances_mutex_ref_(nullptr) {
  Logger::Instance().Info("DisplayChangeCoordinator", "Module initialized");
}

DisplayChangeCoordinator::~DisplayChangeCoordinator() {
  Logger::Instance().Info("DisplayChangeCoordinator", "Module destroyed");
}

void DisplayChangeCoordinator::Initialize(
    GetMonitorsCallback get_monitors,
    StopWallpaperCallback stop_wallpaper,
    NotifyUICallback notify_ui,
    std::vector<MonitorInfo>* monitors_ref,
    std::vector<WallpaperInstance>* instances_ref,
    std::mutex* instances_mutex_ref) {
  
  get_monitors_ = get_monitors;
  stop_wallpaper_ = stop_wallpaper;
  notify_ui_ = notify_ui;
  monitors_ref_ = monitors_ref;
  instances_ref_ = instances_ref;
  instances_mutex_ref_ = instances_mutex_ref;
  
  Logger::Instance().Info("DisplayChangeCoordinator", "Callbacks initialized");
}

void DisplayChangeCoordinator::StartMonitoring(MonitorManager* monitor_manager) {
  if (monitor_manager) {
    try {
      monitor_manager->StartMonitoring();
      Logger::Instance().Info("DisplayChangeCoordinator", "Monitoring started");
    } catch (const std::exception& e) {
      Logger::Instance().Error("DisplayChangeCoordinator",
        "StartMonitoring failed: " + std::string(e.what()));
    }
  } else {
    Logger::Instance().Error("DisplayChangeCoordinator", "MonitorManager is null");
  }
}

void DisplayChangeCoordinator::StopMonitoring(MonitorManager* monitor_manager) {
  if (monitor_manager) {
    try {
      monitor_manager->StopMonitoring();
      Logger::Instance().Info("DisplayChangeCoordinator", "Monitoring stopped");
    } catch (const std::exception& e) {
      Logger::Instance().Error("DisplayChangeCoordinator",
        "StopMonitoring failed: " + std::string(e.what()));
    }
  } else {
    Logger::Instance().Error("DisplayChangeCoordinator", "MonitorManager is null");
  }
}

void DisplayChangeCoordinator::SetDefaultWallpaperUrl(const std::string& url) {
  default_wallpaper_url_ = url;
  Logger::Instance().Info("DisplayChangeCoordinator",
    "Default wallpaper URL set: " + url);
}

void DisplayChangeCoordinator::HandleDisplayChange() {
  Logger::Instance().Info("DisplayChangeCoordinator",
    "========== Handling display change ==========");
  
  // Wait for system to stabilize
  Sleep(200);
  
  // Get old and new monitor lists
  if (!monitors_ref_) {
    Logger::Instance().Error("DisplayChangeCoordinator", "monitors_ref_ is null");
    return;
  }
  
  std::vector<MonitorInfo> old_monitors = *monitors_ref_;
  std::vector<MonitorInfo> new_monitors = get_monitors_();
  
  std::cout << "[DisplayChange] Monitor count: " << old_monitors.size() 
            << " -> " << new_monitors.size() << std::endl;
  
  bool should_notify_ui = false;
  
  // Check if monitor count changed
  if (old_monitors.size() != new_monitors.size()) {
    std::cout << "[DisplayChange] Monitor count changed - will notify UI" << std::endl;
    
    // Handle removed monitors FIRST (cleanup)
    if (new_monitors.size() < old_monitors.size()) {
      std::cout << "[DisplayChange] Monitor(s) removed - cleaning up first" << std::endl;
      HandleMonitorCountChange(old_monitors, new_monitors);
    }
    
    should_notify_ui = true;
    
    // Update remaining wallpaper sizes AFTER cleanup
    UpdateWallpaperSizes();
    
    // Handle added monitors AFTER size update
    if (new_monitors.size() > old_monitors.size()) {
      std::cout << "[DisplayChange] Monitor(s) added - handling new monitors" << std::endl;
      HandleMonitorCountChange(old_monitors, new_monitors);
    }
  } else {
    std::cout << "[DisplayChange] Monitor count unchanged - checking for other changes" << std::endl;
    // Even if count is same, monitors might have changed (resolution, position, etc.)
    UpdateWallpaperSizes();
  }
  
  // Notify UI if needed
  if (should_notify_ui) {
    std::cout << "[DisplayChange] Queuing UI notification..." << std::endl;
    SafeNotifyMonitorChange();
  }
  
  Logger::Instance().Info("DisplayChangeCoordinator",
    "========== Display change handled ==========");
}

void DisplayChangeCoordinator::HandleMonitorCountChange(
    const std::vector<MonitorInfo>& old_monitors,
    const std::vector<MonitorInfo>& new_monitors) {
  
  std::cout << "[DisplayChange] Handling monitor count change" << std::endl;
  
  // Detect new monitors
  if (new_monitors.size() > old_monitors.size()) {
    std::cout << "[DisplayChange] New monitor(s) detected!" << std::endl;
    
    // Find which monitors are new
    for (const auto& new_mon : new_monitors) {
      bool is_new = true;
      
      // Check if this monitor existed before (by comparing device name)
      for (const auto& old_mon : old_monitors) {
        if (new_mon.device_name == old_mon.device_name) {
          is_new = false;
          break;
        }
      }
      
      if (is_new) {
        std::cout << "[DisplayChange] New monitor " << new_mon.index 
                  << ": " << new_mon.device_name << " [" << new_mon.width << "x" << new_mon.height << "]" << std::endl;
        
        // Check if we should auto-start wallpaper
        if (!default_wallpaper_url_.empty()) {
          std::cout << "[DisplayChange] Will auto-start wallpaper on new monitor " << new_mon.index << std::endl;
          std::cout << "[DisplayChange] Using URL: " << default_wallpaper_url_ << std::endl;
          std::cout << "[DisplayChange] Note: Auto-start should be implemented by user" << std::endl;
        } else {
          std::cout << "[DisplayChange] No default URL, user can manually start wallpaper" << std::endl;
        }
      }
    }
  }
  // Detect removed monitors
  else if (new_monitors.size() < old_monitors.size()) {
    std::cout << "[DisplayChange] Monitor(s) removed!" << std::endl;
    
    if (!instances_ref_ || !instances_mutex_ref_) {
      Logger::Instance().Error("DisplayChangeCoordinator",
        "instances_ref_ or instances_mutex_ref_ is null");
      return;
    }
    
    // Find which monitors were removed
    for (const auto& old_mon : old_monitors) {
      bool still_exists = false;
      
      for (const auto& new_mon : new_monitors) {
        if (old_mon.device_name == new_mon.device_name) {
          still_exists = true;
          break;
        }
      }
      
      if (!still_exists) {
        std::cout << "[DisplayChange] Monitor removed: " << old_mon.device_name 
                  << " (index: " << old_mon.index << ")" << std::endl;
        
        // Check if wallpaper is running on removed monitor (thread-safe check)
        bool has_wallpaper = false;
        {
          std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
          for (const auto& inst : *instances_ref_) {
            if (inst.monitor_index == old_mon.index) {
              has_wallpaper = true;
              break;
            }
          }
        }
        
        if (has_wallpaper) {
          std::cout << "[DisplayChange] Found wallpaper on removed monitor " << old_mon.index 
                    << ", cleaning up..." << std::endl;
          
          // Clean up wallpaper on removed monitor
          bool cleanup_success = stop_wallpaper_(old_mon.index);
          std::cout << "[DisplayChange] Cleanup " << (cleanup_success ? "succeeded" : "failed") << std::endl;
        } else {
          std::cout << "[DisplayChange] No wallpaper found on removed monitor " << old_mon.index << std::endl;
        }
      }
    }
  }
}

void DisplayChangeCoordinator::UpdateWallpaperSizes() {
  if (!instances_ref_ || !instances_mutex_ref_ || !monitors_ref_) {
    Logger::Instance().Error("DisplayChangeCoordinator",
      "Required references are null");
    return;
  }
  
  std::lock_guard<std::mutex> lock(*instances_mutex_ref_);
  
  std::cout << "[DisplayChange] Updating " << instances_ref_->size() << " wallpaper instance(s)..." << std::endl;
  
  // Use index-based loop to avoid iterator invalidation
  for (size_t i = 0; i < instances_ref_->size(); ++i) {
    auto& instance = (*instances_ref_)[i];
    
    std::cout << "[DisplayChange] Checking instance " << i << " (monitor " << instance.monitor_index << ")" << std::endl;
    
    // Find current monitor info
    const MonitorInfo* monitor = nullptr;
    for (const auto& m : *monitors_ref_) {
      if (m.index == instance.monitor_index) {
        monitor = &m;
        break;
      }
    }
    
    if (!monitor) {
      std::cout << "[DisplayChange] Monitor " << instance.monitor_index << " not found, skipping" << std::endl;
      continue;
    }
    
    if (!instance.webview_host_hwnd || !IsWindow(instance.webview_host_hwnd)) {
      std::cout << "[DisplayChange] Window for monitor " << instance.monitor_index << " is invalid, skipping" << std::endl;
      continue;
    }
    
    // Update window position and size
    BOOL success = SetWindowPos(
      instance.webview_host_hwnd,
      nullptr,
      monitor->left,
      monitor->top,
      monitor->width,
      monitor->height,
      SWP_NOZORDER | SWP_NOACTIVATE
    );
    
    if (success) {
      std::cout << "[DisplayChange] Updated monitor " << instance.monitor_index 
                << " window to " << monitor->width << "x" << monitor->height 
                << " @ (" << monitor->left << "," << monitor->top << ")" << std::endl;
      
      // Update WebView bounds
      if (instance.webview_controller) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = monitor->width;
        bounds.bottom = monitor->height;
        
        HRESULT hr = instance.webview_controller->put_Bounds(bounds);
        if (SUCCEEDED(hr)) {
          std::cout << "[DisplayChange] Updated WebView bounds for monitor " << instance.monitor_index << std::endl;
        } else {
          std::cout << "[DisplayChange] Failed to update WebView bounds: " << std::hex << hr << std::endl;
        }
      }
    } else {
      std::cout << "[DisplayChange] Failed to update window for monitor " << instance.monitor_index 
                << ", error: " << GetLastError() << std::endl;
    }
  }
  
  std::cout << "[DisplayChange] Update complete" << std::endl;
}

void DisplayChangeCoordinator::NotifyMonitorChange() {
  std::cout << "[DisplayChange] Notifying monitor change..." << std::endl;
  
  // CRITICAL: InvokeMethod causes deadlock/crash
  // Solution: Use polling from Dart side instead (Timer.periodic)
  // Do NOT call InvokeMethod - it will hang the application
  
  std::cout << "[DisplayChange] Skipping InvokeMethod to prevent deadlock" << std::endl;
  std::cout << "[DisplayChange] Dart side will detect changes via polling" << std::endl;
  
  Logger::Instance().Info("DisplayChangeCoordinator", "Notification completed");
}

void DisplayChangeCoordinator::SafeNotifyMonitorChange() {
  NotifyMonitorChange();
}

}  // namespace anywp_engine

