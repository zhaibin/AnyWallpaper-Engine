#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_INSTANCE_MANAGER_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_INSTANCE_MANAGER_H_

#include <windows.h>
#include <vector>
#include <mutex>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

// Forward declarations
namespace anywp_engine {
  struct MonitorInfo;
  struct WallpaperInstance;
}

namespace anywp_engine {

// Wallpaper instance management module
// v2.0.0+ Phase2: Extracted from main plugin to improve modularity
class InstanceManager {
public:
  // Callback types
  using RemoveMouseHookCallback = std::function<void()>;
  using ClearDefaultUrlCallback = std::function<void()>;
  using UntrackWindowCallback = std::function<void(HWND)>;

  InstanceManager();
  ~InstanceManager();

  // Disable copy
  InstanceManager(const InstanceManager&) = delete;
  InstanceManager& operator=(const InstanceManager&) = delete;

  // Initialize with callbacks
  void Initialize(
      RemoveMouseHookCallback remove_mouse_hook,
      ClearDefaultUrlCallback clear_default_url,
      UntrackWindowCallback untrack_window,
      std::vector<WallpaperInstance>* instances_ref,
      std::vector<MonitorInfo>* monitors_ref,
      std::mutex* instances_mutex_ref);

  // Instance queries
  WallpaperInstance* GetInstanceForMonitor(int monitor_index);
  WallpaperInstance* GetInstanceAtPoint(int x, int y);
  bool HasInstance(int monitor_index) const;
  size_t GetInstanceCount() const;
  bool IsEmpty() const;

  // Instance lifecycle
  void AddInstance(const WallpaperInstance& instance);
  bool RemoveInstance(int monitor_index);
  void ClearAllInstances();

  // Instance cleanup
  bool CleanupInstance(int monitor_index);

private:
  // Callbacks
  RemoveMouseHookCallback remove_mouse_hook_;
  ClearDefaultUrlCallback clear_default_url_;
  UntrackWindowCallback untrack_window_;

  // References to plugin data (not owned)
  std::vector<WallpaperInstance>* instances_ref_;
  std::vector<MonitorInfo>* monitors_ref_;
  std::mutex* instances_mutex_ref_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_INSTANCE_MANAGER_H_

