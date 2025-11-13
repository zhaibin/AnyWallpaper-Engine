#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_WINDOW_MANAGER_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_WINDOW_MANAGER_H_

#include <windows.h>
#include <string>

// Forward declarations
namespace anywp_engine {
  struct MonitorInfo;
}

namespace anywp_engine {

// Window management module
// v2.0.0+ Phase2: Extracted from main plugin to improve modularity
// Handles window creation, validation, and Z-order management
class WindowManager {
public:
  WindowManager();
  ~WindowManager();

  // Disable copy
  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;

  // Window creation
  HWND CreateWebViewHostWindow(
      bool enable_mouse_transparent,
      const MonitorInfo* monitor,
      HWND parent_window);

  // Window validation
  static bool IsValidWindowHandle(HWND hwnd);
  static bool IsValidParentWindow(HWND hwnd);

  // Z-order management
  bool SetWallpaperZOrder(HWND hwnd, HWND worker_w);

  // WorkerW helper
  HWND FindSHELLDLL_DefView(HWND worker_w);

  // New desktop wallpaper embedding APIs (v2.0.1+)
  /// Initialize window as desktop wallpaper
  /// Embeds the window into the second WorkerW layer (below desktop icons)
  /// @param hwnd The window to embed
  /// @param enable_transparent Whether to enable mouse transparent initially
  /// @return true if successful
  bool InitializeAsWallpaper(HWND hwnd, bool enable_transparent);

  /// Set window interactive mode
  /// Dynamically toggle WS_EX_TRANSPARENT for mouse interaction
  /// @param hwnd The window to modify
  /// @param interactive true = can receive clicks, false = mouse transparent
  /// @return true if successful
  bool SetInteractive(HWND hwnd, bool interactive);

  /// Find the second WorkerW window (wallpaper layer)
  /// @return HWND of second WorkerW, or nullptr if not found
  HWND FindSecondWorkerW();

private:
  // Window dimension calculation
  bool CalculateWindowDimensions(
      const MonitorInfo* monitor,
      int& out_x,
      int& out_y,
      int& out_width,
      int& out_height);

  // Window dimension validation
  bool ValidateDimensions(int width, int height);
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_WINDOW_MANAGER_H_

