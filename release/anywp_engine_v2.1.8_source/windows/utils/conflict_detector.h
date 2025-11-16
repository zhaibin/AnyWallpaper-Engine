#ifndef ANYWP_ENGINE_CONFLICT_DETECTOR_H_
#define ANYWP_ENGINE_CONFLICT_DETECTOR_H_

#include <windows.h>
#include <string>
#include <vector>
#include <psapi.h>

namespace anywp_engine {

/**
 * WallpaperConflictInfo - Information about a detected wallpaper software
 */
struct WallpaperConflictInfo {
  std::string name;              // Display name (e.g., "Wallpaper Engine")
  std::wstring process_name;     // Process name (e.g., "wallpaper32.exe")
  bool detected;                 // Whether the process is currently running
  DWORD process_id;              // Process ID (if detected)
};

/**
 * ConflictDetector - Detect conflicts with other wallpaper software
 * 
 * Features:
 * - Detect running wallpaper software
 * - Configurable wallpaper list
 * - User notification support
 * - Resolution suggestions
 * 
 * Known Wallpaper Software:
 * - Wallpaper Engine
 * - Lively Wallpaper
 * - DeskScapes
 * - RainWallpaper
 * - Push Video Wallpaper
 */
class ConflictDetector {
public:
  ConflictDetector();
  ~ConflictDetector();

  /**
   * Detect conflicts with known wallpaper software
   * @param[out] conflicts Vector of detected conflicts
   * @return true if conflicts found, false otherwise
   */
  bool DetectConflicts(std::vector<WallpaperConflictInfo>& conflicts);

  /**
   * Add a wallpaper software to the detection list
   * @param name Display name
   * @param process_name Process executable name
   */
  void AddWallpaperSoftware(const std::string& name, const std::wstring& process_name);

  /**
   * Clear the detection list
   */
  void ClearWallpaperList();

  /**
   * Get the default list of known wallpaper software
   */
  static std::vector<WallpaperConflictInfo> GetDefaultWallpaperList();

  /**
   * Check if a specific process is running
   * @param process_name Process executable name (case-insensitive)
   * @return Process ID if found, 0 otherwise
   */
  static DWORD IsProcessRunning(const std::wstring& process_name);

private:
  std::vector<WallpaperConflictInfo> known_wallpapers_;

  // Helper: Enumerate all processes
  bool EnumerateProcesses(std::vector<DWORD>& process_ids);

  // Helper: Get process name from PID
  std::wstring GetProcessName(DWORD process_id);
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_CONFLICT_DETECTOR_H_

