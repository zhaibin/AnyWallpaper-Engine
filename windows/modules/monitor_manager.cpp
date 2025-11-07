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
  
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 
                     reinterpret_cast<LPARAM>(&monitors));
  
  cached_monitors_ = monitors;
  return monitors;
}

MonitorInfo* MonitorManager::GetMonitorByIndex(int index) {
  if (index >= 0 && index < static_cast<int>(cached_monitors_.size())) {
    return &cached_monitors_[index];
  }
  return nullptr;
}

MonitorInfo* MonitorManager::GetPrimaryMonitor() {
  for (auto& monitor : cached_monitors_) {
    if (monitor.is_primary) {
      return &monitor;
    }
  }
  return nullptr;
}

void MonitorManager::StartMonitoring() {
  std::cout << "[MonitorManager] Starting monitor change detection..." << std::endl;
  // TODO: Implement listener window
}

void MonitorManager::StopMonitoring() {
  if (listener_hwnd_) {
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
  for (auto& monitor : cached_monitors_) {
    if (x >= monitor.left && x < monitor.left + monitor.width &&
        y >= monitor.top && y < monitor.top + monitor.height) {
      return &monitor;
    }
  }
  return nullptr;
}

BOOL CALLBACK MonitorManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
  
  MONITORINFOEXW info = {};
  info.cbSize = sizeof(MONITORINFOEXW);
  GetMonitorInfoW(hMonitor, &info);
  
  MonitorInfo monitor;
  monitor.index = static_cast<int>(monitors->size());
  monitor.left = lprcMonitor->left;
  monitor.top = lprcMonitor->top;
  monitor.width = lprcMonitor->right - lprcMonitor->left;
  monitor.height = lprcMonitor->bottom - lprcMonitor->top;
  monitor.is_primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
  monitor.handle = hMonitor;
  
  // Convert device name
  int size = WideCharToMultiByte(CP_UTF8, 0, info.szDevice, -1, nullptr, 0, nullptr, nullptr);
  std::string device_name(size - 1, 0);
  WideCharToMultiByte(CP_UTF8, 0, info.szDevice, -1, &device_name[0], size, nullptr, nullptr);
  monitor.device_name = device_name;
  
  monitors->push_back(monitor);
  return TRUE;
}

LRESULT CALLBACK MonitorManager::DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_DISPLAYCHANGE && instance_ && instance_->on_changed_) {
    std::cout << "[MonitorManager] Display change detected!" << std::endl;
    auto new_monitors = instance_->GetMonitors();
    instance_->on_changed_(new_monitors);
  }
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

}  // namespace anywp_engine

