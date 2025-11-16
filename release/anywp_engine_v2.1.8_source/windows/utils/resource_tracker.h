#ifndef ANYWP_ENGINE_RESOURCE_TRACKER_H_
#define ANYWP_ENGINE_RESOURCE_TRACKER_H_

#include <windows.h>
#include <set>
#include <mutex>
#include <string>

namespace anywp_engine {

/**
 * ResourceTracker - Track and manage system resources
 * 
 * Features:
 * - Track window handles (HWND)
 * - Automatic cleanup on destruction
 * - Memory leak detection
 * - Thread-safe operations
 * 
 * Use Case: Ensure all created windows are properly destroyed
 */
class ResourceTracker {
public:
  static ResourceTracker& Instance() {
    static ResourceTracker instance;
    return instance;
  }

  // Disable copy
  ResourceTracker(const ResourceTracker&) = delete;
  ResourceTracker& operator=(const ResourceTracker&) = delete;

  /**
   * Track a window handle
   * @param hwnd Window handle to track
   * @param tag Optional tag for identification (e.g., "webview", "listener")
   */
  void TrackWindow(HWND hwnd, const std::string& tag = "");

  /**
   * Untrack a window handle (call this when manually destroying)
   * @param hwnd Window handle to untrack
   */
  void UntrackWindow(HWND hwnd);

  /**
   * Check if a window is being tracked
   */
  bool IsTracked(HWND hwnd) const;

  /**
   * Cleanup all tracked windows
   */
  void CleanupAll();

  /**
   * Get number of tracked windows
   */
  size_t GetTrackedCount() const;

  /**
   * Get list of tracked windows (for debugging)
   */
  std::set<HWND> GetTrackedWindows() const;

  /**
   * Report current tracked resources
   */
  void Report() const;

private:
  ResourceTracker() = default;
  ~ResourceTracker();

  struct TrackedWindow {
    HWND hwnd;
    std::string tag;
  };

  mutable std::mutex mutex_;
  std::set<HWND> tracked_windows_;
  // Optional: Store additional metadata
  // std::map<HWND, std::string> window_tags_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_RESOURCE_TRACKER_H_

