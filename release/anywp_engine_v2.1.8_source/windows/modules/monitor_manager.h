#ifndef ANYWP_ENGINE_MONITOR_MANAGER_H_
#define ANYWP_ENGINE_MONITOR_MANAGER_H_

#include <windows.h>
#include <string>
#include <vector>
#include <functional>

namespace anywp_engine {

/**
 * MonitorInfo - Information about a display monitor
 */
struct MonitorInfo {
  int index;
  std::string device_name;
  int left;
  int top;
  int width;
  int height;
  bool is_primary;
  HMONITOR handle;
};

/**
 * MonitorManager - Manage multiple displays and handle hot-plug events
 * 
 * Features:
 * - Enumerate all monitors
 * - Monitor hot-plug detection (WM_DISPLAYCHANGE)
 * - Monitor count change notification
 * - Screen resolution change handling
 */
class MonitorManager {
public:
  MonitorManager();
  ~MonitorManager();

  // Monitor enumeration
  std::vector<MonitorInfo> GetMonitors();
  MonitorInfo* GetMonitorByIndex(int index);
  MonitorInfo* GetPrimaryMonitor();
  
  // Hot-plug monitoring
  void StartMonitoring();
  void StopMonitoring();
  bool IsMonitoring() const;
  
  // Callbacks
  using MonitorChangeCallback = std::function<void(const std::vector<MonitorInfo>&)>;
  void SetOnMonitorChanged(MonitorChangeCallback callback);
  
  // Helpers
  MonitorInfo* GetMonitorAtPoint(int x, int y);

private:
  static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
  static LRESULT CALLBACK DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static MonitorManager* instance_;
  
  std::vector<MonitorInfo> cached_monitors_;
  HWND listener_hwnd_;
  MonitorChangeCallback on_changed_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_MONITOR_MANAGER_H_

