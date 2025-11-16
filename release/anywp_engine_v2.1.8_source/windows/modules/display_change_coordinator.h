#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_DISPLAY_CHANGE_COORDINATOR_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_DISPLAY_CHANGE_COORDINATOR_H_

#include <windows.h>
#include <vector>
#include <string>
#include <mutex>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

// Forward declarations
namespace anywp_engine {
  // Forward declarations for MonitorInfo and WallpaperInstance
  // Full definitions will be available when anywp_engine_plugin.h is included
  struct MonitorInfo;
  struct WallpaperInstance;
  class MonitorManager;
}

namespace anywp_engine {

// Display change coordination module
// v2.0.0+ Phase2: Extracted from main plugin to improve modularity
class DisplayChangeCoordinator {
public:
  // Callback types
  using GetMonitorsCallback = std::function<std::vector<MonitorInfo>()>;
  using StopWallpaperCallback = std::function<bool(int monitor_index)>;
  using UpdateWallpaperBoundsCallback = std::function<void(int monitor_index, RECT bounds)>;
  using NotifyUICallback = std::function<void()>;

  DisplayChangeCoordinator();
  ~DisplayChangeCoordinator();

  // Disable copy
  DisplayChangeCoordinator(const DisplayChangeCoordinator&) = delete;
  DisplayChangeCoordinator& operator=(const DisplayChangeCoordinator&) = delete;

  // Initialize with callbacks
  void Initialize(
      GetMonitorsCallback get_monitors,
      StopWallpaperCallback stop_wallpaper,
      NotifyUICallback notify_ui,
      std::vector<MonitorInfo>* monitors_ref,
      std::vector<WallpaperInstance>* instances_ref,
      std::mutex* instances_mutex_ref);

  // Start/Stop monitoring (delegates to MonitorManager)
  void StartMonitoring(MonitorManager* monitor_manager);
  void StopMonitoring(MonitorManager* monitor_manager);

  // Main display change handler
  void HandleDisplayChange();

  // Set default URL for auto-starting wallpapers on new monitors
  void SetDefaultWallpaperUrl(const std::string& url);

private:
  // Sub-handlers
  void HandleMonitorCountChange(
      const std::vector<MonitorInfo>& old_monitors,
      const std::vector<MonitorInfo>& new_monitors);
  void UpdateWallpaperSizes();
  void NotifyMonitorChange();
  void SafeNotifyMonitorChange();

  // Callbacks
  GetMonitorsCallback get_monitors_;
  StopWallpaperCallback stop_wallpaper_;
  NotifyUICallback notify_ui_;

  // References to plugin data (not owned)
  std::vector<MonitorInfo>* monitors_ref_;
  std::vector<WallpaperInstance>* instances_ref_;
  std::mutex* instances_mutex_ref_;

  // Configuration
  std::string default_wallpaper_url_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_DISPLAY_CHANGE_COORDINATOR_H_

