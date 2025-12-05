// CRITICAL: Include anywp_engine_plugin.h FIRST to get full type definitions
// before including display_change_coordinator.h which uses those types
#include "../utils/logger.h"
#include "monitor_manager.h"
#include <sstream>

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
  
  Logger::Instance().Info("DisplayChangeCoordinator", 
    "Monitor count: " + std::to_string(old_monitors.size()) + " -> " + std::to_string(new_monitors.size()));
  
  bool should_notify_ui = false;
  
  // Check if monitor count changed
  if (old_monitors.size() != new_monitors.size()) {
    Logger::Instance().Info("DisplayChangeCoordinator", "Monitor count changed - will notify UI");
    
    // Handle removed monitors FIRST (cleanup)
    if (new_monitors.size() < old_monitors.size()) {
      Logger::Instance().Info("DisplayChangeCoordinator", "Monitor(s) removed - cleaning up first");
      HandleMonitorCountChange(old_monitors, new_monitors);
    }
    
    should_notify_ui = true;
    
    // Update remaining wallpaper sizes AFTER cleanup
    UpdateWallpaperSizes();
    
    // Handle added monitors AFTER size update
    if (new_monitors.size() > old_monitors.size()) {
      Logger::Instance().Info("DisplayChangeCoordinator", "Monitor(s) added - handling new monitors");
      HandleMonitorCountChange(old_monitors, new_monitors);
    }
  } else {
    Logger::Instance().Info("DisplayChangeCoordinator", "Monitor count unchanged - checking for other changes");
    // Even if count is same, monitors might have changed (resolution, position, etc.)
    UpdateWallpaperSizes();
  }
  
  // Notify UI if needed
  if (should_notify_ui) {
    Logger::Instance().Info("DisplayChangeCoordinator", "Queuing UI notification...");
    SafeNotifyMonitorChange();
  }
  
  Logger::Instance().Info("DisplayChangeCoordinator",
    "========== Display change handled ==========");
}

void DisplayChangeCoordinator::HandleMonitorCountChange(
    const std::vector<MonitorInfo>& old_monitors,
    const std::vector<MonitorInfo>& new_monitors) {
  
  Logger::Instance().Info("DisplayChangeCoordinator", "Handling monitor count change");
  
  // Detect new monitors
  if (new_monitors.size() > old_monitors.size()) {
    Logger::Instance().Info("DisplayChangeCoordinator", "New monitor(s) detected!");
    
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
        Logger::Instance().Info("DisplayChangeCoordinator", 
          "New monitor " + std::to_string(new_mon.index) + ": " + new_mon.device_name + 
          " [" + std::to_string(new_mon.width) + "x" + std::to_string(new_mon.height) + "]");
        
        // Check if we should auto-start wallpaper
        if (!default_wallpaper_url_.empty()) {
          Logger::Instance().Info("DisplayChangeCoordinator", "Will auto-start wallpaper on new monitor " + std::to_string(new_mon.index));
          Logger::Instance().Info("DisplayChangeCoordinator", "Using URL: " + default_wallpaper_url_);
          Logger::Instance().Info("DisplayChangeCoordinator", "Note: Auto-start should be implemented by user");
        } else {
          Logger::Instance().Info("DisplayChangeCoordinator", "No default URL, user can manually start wallpaper");
        }
      }
    }
  }
  // Detect removed monitors
  else if (new_monitors.size() < old_monitors.size()) {
    Logger::Instance().Info("DisplayChangeCoordinator", "Monitor(s) removed!");
    
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
        Logger::Instance().Info("DisplayChangeCoordinator", 
          "Monitor removed: " + old_mon.device_name + " (index: " + std::to_string(old_mon.index) + ")");
        
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
          Logger::Instance().Info("DisplayChangeCoordinator", 
            "Found wallpaper on removed monitor " + std::to_string(old_mon.index) + ", cleaning up...");
          
          // Clean up wallpaper on removed monitor
          bool cleanup_success = stop_wallpaper_(old_mon.index);
          Logger::Instance().Info("DisplayChangeCoordinator", 
            "Cleanup " + std::string(cleanup_success ? "succeeded" : "failed"));
        } else {
          Logger::Instance().Info("DisplayChangeCoordinator", 
            "No wallpaper found on removed monitor " + std::to_string(old_mon.index));
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
  
  Logger::Instance().Info("DisplayChangeCoordinator", 
    "Updating " + std::to_string(instances_ref_->size()) + " wallpaper instance(s)...");
  
  // Use index-based loop to avoid iterator invalidation
  for (size_t i = 0; i < instances_ref_->size(); ++i) {
    auto& instance = (*instances_ref_)[i];
    
    Logger::Instance().Info("DisplayChangeCoordinator", 
      "Checking instance " + std::to_string(i) + " (monitor " + std::to_string(instance.monitor_index) + ")");
    
    // Find current monitor info
    const MonitorInfo* monitor = nullptr;
    for (const auto& m : *monitors_ref_) {
      if (m.index == instance.monitor_index) {
        monitor = &m;
        break;
      }
    }
    
    if (!monitor) {
      Logger::Instance().Info("DisplayChangeCoordinator", 
        "Monitor " + std::to_string(instance.monitor_index) + " not found, skipping");
      continue;
    }
    
    if (!instance.webview_host_hwnd || !IsWindow(instance.webview_host_hwnd)) {
      Logger::Instance().Info("DisplayChangeCoordinator", 
        "Window for monitor " + std::to_string(instance.monitor_index) + " is invalid, skipping");
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
      Logger::Instance().Info("DisplayChangeCoordinator", 
        "Updated monitor " + std::to_string(instance.monitor_index) + 
        " window to " + std::to_string(monitor->width) + "x" + std::to_string(monitor->height) + 
        " @ (" + std::to_string(monitor->left) + "," + std::to_string(monitor->top) + ")");
      
      // Update WebView bounds
      if (instance.webview_controller) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = monitor->width;
        bounds.bottom = monitor->height;
        
        HRESULT hr = instance.webview_controller->put_Bounds(bounds);
        if (SUCCEEDED(hr)) {
          Logger::Instance().Info("DisplayChangeCoordinator", 
            "Updated WebView bounds for monitor " + std::to_string(instance.monitor_index));
        } else {
          std::ostringstream oss;
          oss << std::hex << hr;
          Logger::Instance().Error("DisplayChangeCoordinator", 
            "Failed to update WebView bounds: " + oss.str());
        }
      }
    } else {
      Logger::Instance().Error("DisplayChangeCoordinator", 
        "Failed to update window for monitor " + std::to_string(instance.monitor_index) + 
        ", error: " + std::to_string(GetLastError()));
    }
  }
  
  Logger::Instance().Info("DisplayChangeCoordinator", "Update complete");
}

void DisplayChangeCoordinator::NotifyMonitorChange() {
  Logger::Instance().Info("DisplayChangeCoordinator", "Notifying monitor change...");
  
  // CRITICAL: InvokeMethod causes deadlock/crash
  // Solution: Use polling from Dart side instead (Timer.periodic)
  // Do NOT call InvokeMethod - it will hang the application
  
  Logger::Instance().Info("DisplayChangeCoordinator", "Skipping InvokeMethod to prevent deadlock");
  Logger::Instance().Info("DisplayChangeCoordinator", "Dart side will detect changes via polling");
  
  Logger::Instance().Info("DisplayChangeCoordinator", "Notification completed");
}

void DisplayChangeCoordinator::SafeNotifyMonitorChange() {
  NotifyMonitorChange();
}

}  // namespace anywp_engine

