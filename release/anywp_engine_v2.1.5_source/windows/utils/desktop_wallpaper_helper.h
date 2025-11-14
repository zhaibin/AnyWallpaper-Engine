#ifndef ANYWP_ENGINE_DESKTOP_WALLPAPER_HELPER_H_
#define ANYWP_ENGINE_DESKTOP_WALLPAPER_HELPER_H_

#include <windows.h>
#include <string>

namespace anywp_engine {

/**
 * WorkerWInfo - Information about Windows desktop WorkerW windows
 */
struct WorkerWInfo {
  HWND progman = nullptr;         // Progman window
  HWND icon_layer = nullptr;      // WorkerW containing SHELLDLL_DefView (icon layer)
  HWND wallpaper_layer = nullptr; // Wallpaper layer WorkerW
  int workerw_count = 0;
  bool found_shelldll = false;
};

/**
 * DesktopWallpaperHelper - Manage Windows desktop wallpaper layer
 * 
 * Features:
 * - Find WorkerW windows for wallpaper placement
 * - Reliable detection with retry mechanism
 * - Support for different Windows versions
 * - Detailed logging and error handling
 * 
 * Background:
 * Windows desktop has multiple layers:
 * 1. Progman - Program Manager (top-level)
 * 2. WorkerW (with SHELLDLL_DefView) - Icon layer
 * 3. WorkerW (below icons) - Wallpaper layer ← We insert here
 */
class DesktopWallpaperHelper {
public:
  // Singleton instance
  static DesktopWallpaperHelper& Instance() {
    static DesktopWallpaperHelper instance;
    return instance;
  }

  /**
   * Find WorkerW windows for wallpaper placement
   * @param timeout_ms Maximum time to wait for WorkerW (default: 2000ms)
   * @return true if found, false otherwise
   */
  bool FindWorkerW(int timeout_ms = 2000);

  /**
   * Get the WorkerW information
   */
  const WorkerWInfo& GetWorkerWInfo() const { return info_; }

  /**
   * Get the parent window for wallpaper placement
   * This is usually the wallpaper_layer WorkerW
   */
  HWND GetWallpaperParent() const;

  /**
   * Check if WorkerW has been found
   */
  bool IsValid() const { return info_.found_shelldll && info_.wallpaper_layer != nullptr; }

  /**
   * Reset cached information (call this after display changes)
   */
  void Reset();

  /**
   * Trigger Windows to create WorkerW windows
   * Sends 0x052C message to Progman
   */
  bool TriggerWorkerWCreation();

private:
  DesktopWallpaperHelper();
  ~DesktopWallpaperHelper();
  
  // Disable copy
  DesktopWallpaperHelper(const DesktopWallpaperHelper&) = delete;
  DesktopWallpaperHelper& operator=(const DesktopWallpaperHelper&) = delete;
  
  WorkerWInfo info_;
  
  // Helper: Find Progman window
  bool FindProgman();
  
  // Helper: Enumerate WorkerW windows
  bool EnumerateWorkerW();
  
  // Helper: Check for SHELLDLL_DefView in window
  bool HasSHELLDLL(HWND hwnd);
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_DESKTOP_WALLPAPER_HELPER_H_

