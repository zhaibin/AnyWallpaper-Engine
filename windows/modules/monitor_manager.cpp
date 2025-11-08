#include "monitor_manager.h"
#include <iostream>

namespace anywp_engine {

MonitorManager* MonitorManager::instance_ = nullptr;

MonitorManager::MonitorManager()
    : listener_hwnd_(nullptr) {
  instance_ = this;
}

MonitorManager::~MonitorManager() {
  StopMonitoring();
  instance_ = nullptr;
}

std::vector<MonitorInfo> MonitorManager::GetMonitors() {
  std::vector<MonitorInfo> monitors;
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
  
  std::cout << "[AnyWP] [MonitorManager] Found " << monitors.size() << " monitor(s)" << std::endl;
  
  // Cache for later use
  cached_monitors_ = monitors;
  
  return monitors;
}

MonitorInfo* MonitorManager::GetMonitorByIndex(int index) {
  for (auto& monitor : cached_monitors_) {
    if (monitor.index == index) {
      return &monitor;
    }
  }
  return nullptr;
}

MonitorInfo* MonitorManager::GetPrimaryMonitor() {
  for (auto& monitor : cached_monitors_) {
    if (monitor.is_primary) {
      return &monitor;
    }
  }
  return cached_monitors_.empty() ? nullptr : &cached_monitors_[0];
}

void MonitorManager::StartMonitoring() {
  if (listener_hwnd_) {
    return;  // Already monitoring
  }
  
  try {
    std::cout << "[AnyWP] [MonitorManager] Starting display change monitoring..." << std::endl;
    
    // Register window class
    WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
    wc.lpfnWndProc = DisplayChangeWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"AnyWPMonitorChangeListener";
    
    if (!RegisterClassExW(&wc)) {
      DWORD error = GetLastError();
      if (error != ERROR_CLASS_ALREADY_EXISTS) {
        std::cout << "[AnyWP] [MonitorManager] ERROR: Failed to register window class: " << error << std::endl;
        return;
      }
    }
    
    // Create hidden listener window
    listener_hwnd_ = CreateWindowExW(
      0,
      L"AnyWPMonitorChangeListener",
      L"AnyWP Monitor Change Listener",
      WS_OVERLAPPED,
      0, 0, 0, 0,
      nullptr,
      nullptr,
      GetModuleHandle(nullptr),
      nullptr
    );
    
    if (listener_hwnd_) {
      std::cout << "[AnyWP] [MonitorManager] Listener window created: " << listener_hwnd_ << std::endl;
    } else {
      DWORD error = GetLastError();
      std::cout << "[AnyWP] [MonitorManager] ERROR: Failed to create listener window: " << error << std::endl;
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [MonitorManager] ERROR: Exception in StartMonitoring: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "[AnyWP] [MonitorManager] ERROR: Unknown exception in StartMonitoring" << std::endl;
  }
}

void MonitorManager::StopMonitoring() {
  if (listener_hwnd_) {
    std::cout << "[AnyWP] [MonitorManager] Stopping display change monitoring..." << std::endl;
    DestroyWindow(listener_hwnd_);
    listener_hwnd_ = nullptr;
  }
}

bool MonitorManager::IsMonitoring() const {
  return listener_hwnd_ != nullptr;
}

void MonitorManager::SetOnMonitorChanged(MonitorChangeCallback callback) {
  on_changed_ = callback;
}

MonitorInfo* MonitorManager::GetMonitorAtPoint(int x, int y) {
  POINT pt = {x, y};
  HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
  
  if (!hMonitor) {
    return nullptr;
  }
  
  for (auto& monitor : cached_monitors_) {
    if (monitor.handle == hMonitor) {
      return &monitor;
    }
  }
  
  return nullptr;
}

BOOL CALLBACK MonitorManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
  
  MONITORINFOEXW monitor_info;
  monitor_info.cbSize = sizeof(MONITORINFOEXW);
  
  if (GetMonitorInfoW(hMonitor, &monitor_info)) {
    MonitorInfo info;
    info.index = static_cast<int>(monitors->size());
    
    // Convert device name to string
    char device_name[256] = {0};
    WideCharToMultiByte(CP_UTF8, 0, monitor_info.szDevice, -1, 
                       device_name, sizeof(device_name), nullptr, nullptr);
    info.device_name = device_name;
    
    info.left = monitor_info.rcMonitor.left;
    info.top = monitor_info.rcMonitor.top;
    info.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    info.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
    info.is_primary = (monitor_info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    info.handle = hMonitor;
    
    monitors->push_back(info);
  }
  
  return TRUE;  // Continue enumeration
}

LRESULT CALLBACK MonitorManager::DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_DISPLAYCHANGE) {
    std::cout << "[AnyWP] [MonitorManager] Display configuration changed!" << std::endl;
    std::cout << "[AnyWP] [MonitorManager] New resolution: " 
              << LOWORD(lParam) << "x" << HIWORD(lParam) << std::endl;
    
    if (instance_) {
      // Re-enumerate monitors
      std::vector<MonitorInfo> new_monitors = instance_->GetMonitors();
      
      // Notify callback
      if (instance_->on_changed_) {
        instance_->on_changed_(new_monitors);
      }
    }
  }
  
  return DefWindowProc(hwnd, message, wParam, lParam);
}

}  // namespace anywp_engine
