#include "resource_tracker.h"
#include "logger.h"
#include <iostream>

namespace anywp_engine {

ResourceTracker::~ResourceTracker() {
  CleanupAll();
}

void ResourceTracker::TrackWindow(HWND hwnd, const std::string& tag) {
  if (!hwnd || !IsWindow(hwnd)) {
    Logger::Instance().Warning("ResourceTracker", "Attempt to track invalid window");
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  
  if (tracked_windows_.find(hwnd) != tracked_windows_.end()) {
    Logger::Instance().Warning("ResourceTracker", 
      "Window already tracked: " + std::to_string((long long)hwnd));
    return;
  }

  tracked_windows_.insert(hwnd);
  
  std::string log_msg = "Tracking window: " + std::to_string((long long)hwnd);
  if (!tag.empty()) {
    log_msg += " [" + tag + "]";
  }
  log_msg += " (Total: " + std::to_string(tracked_windows_.size()) + ")";
  
  Logger::Instance().Info("ResourceTracker", log_msg);
}

void ResourceTracker::UntrackWindow(HWND hwnd) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = tracked_windows_.find(hwnd);
  if (it == tracked_windows_.end()) {
    Logger::Instance().Warning("ResourceTracker", 
      "Attempt to untrack window that is not tracked: " + std::to_string((long long)hwnd));
    return;
  }

  tracked_windows_.erase(it);
  
  Logger::Instance().Info("ResourceTracker", 
    "Untracked window: " + std::to_string((long long)hwnd) + 
    " (Remaining: " + std::to_string(tracked_windows_.size()) + ")");
}

bool ResourceTracker::IsTracked(HWND hwnd) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return tracked_windows_.find(hwnd) != tracked_windows_.end();
}

// v2.6.7 FIX: Callback for EnumWindows to find orphaned AnyWallpaperHost windows
static BOOL CALLBACK EnumOrphanedWindowsProc(HWND hwnd, LPARAM lParam) {
  std::vector<HWND>* orphans = reinterpret_cast<std::vector<HWND>*>(lParam);
  
  wchar_t class_name[256] = {0};
  wchar_t window_text[256] = {0};
  
  if (GetClassNameW(hwnd, class_name, 256) > 0 && 
      GetWindowTextW(hwnd, window_text, 256) > 0) {
    if (wcscmp(class_name, L"STATIC") == 0 && 
        wcscmp(window_text, L"AnyWallpaperHost") == 0) {
      orphans->push_back(hwnd);
    }
  }
  
  // Also check child windows (our windows are children of WorkerW)
  EnumChildWindows(hwnd, [](HWND child, LPARAM lp) -> BOOL {
    std::vector<HWND>* list = reinterpret_cast<std::vector<HWND>*>(lp);
    
    wchar_t cls[256] = {0};
    wchar_t txt[256] = {0};
    
    if (GetClassNameW(child, cls, 256) > 0 && 
        GetWindowTextW(child, txt, 256) > 0) {
      if (wcscmp(cls, L"STATIC") == 0 && 
          wcscmp(txt, L"AnyWallpaperHost") == 0) {
        list->push_back(child);
      }
    }
    return TRUE;
  }, lParam);
  
  return TRUE;
}

void ResourceTracker::CleanupAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  size_t destroyed_count = 0;
  size_t already_destroyed_count = 0;
  
  // Cleanup tracked windows
  if (!tracked_windows_.empty()) {
    Logger::Instance().Info("ResourceTracker", 
      "Cleaning up " + std::to_string(tracked_windows_.size()) + " tracked windows");

    for (HWND hwnd : tracked_windows_) {
      if (IsWindow(hwnd)) {
        if (DestroyWindow(hwnd)) {
          destroyed_count++;
          Logger::Instance().Debug("ResourceTracker", 
            "Destroyed window: " + std::to_string((long long)hwnd));
        } else {
          DWORD error = GetLastError();
          Logger::Instance().Warning("ResourceTracker", 
            "Failed to destroy window: " + std::to_string((long long)hwnd) + 
            " (Error: " + std::to_string(error) + ")");
        }
      } else {
        already_destroyed_count++;
        Logger::Instance().Debug("ResourceTracker", 
          "Window already destroyed: " + std::to_string((long long)hwnd));
      }
    }

    tracked_windows_.clear();
  }
  
  // v2.6.7 FIX: Find and destroy any orphaned AnyWallpaperHost windows
  // Use EnumWindows to find child windows (our windows are children of WorkerW)
  std::vector<HWND> orphans;
  EnumWindows(EnumOrphanedWindowsProc, reinterpret_cast<LPARAM>(&orphans));
  
  size_t orphan_count = 0;
  for (HWND orphan : orphans) {
    if (IsWindow(orphan)) {
      if (DestroyWindow(orphan)) {
        orphan_count++;
        Logger::Instance().Warning("ResourceTracker", 
          "Destroyed orphaned AnyWallpaperHost window: " + std::to_string((long long)orphan));
      } else {
        DWORD error = GetLastError();
        Logger::Instance().Warning("ResourceTracker", 
          "Failed to destroy orphaned window: " + std::to_string((long long)orphan) +
          " (Error: " + std::to_string(error) + ")");
      }
    }
  }

  Logger::Instance().Info("ResourceTracker", 
    "Cleanup complete: " + std::to_string(destroyed_count) + " tracked, " +
    std::to_string(already_destroyed_count) + " already destroyed, " +
    std::to_string(orphan_count) + " orphaned");
}

size_t ResourceTracker::GetTrackedCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return tracked_windows_.size();
}

std::set<HWND> ResourceTracker::GetTrackedWindows() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return tracked_windows_;
}

void ResourceTracker::Report() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  Logger::Instance().Info("ResourceTracker", 
    "=== Resource Tracker Report ===");
  Logger::Instance().Info("ResourceTracker", 
    "Tracked windows: " + std::to_string(tracked_windows_.size()));
  
  if (tracked_windows_.empty()) {
    Logger::Instance().Info("ResourceTracker", "No windows tracked");
  } else {
    int index = 1;
    for (HWND hwnd : tracked_windows_) {
      bool is_valid = IsWindow(hwnd);
      std::string status = is_valid ? "Valid" : "Invalid/Destroyed";
      
      Logger::Instance().Info("ResourceTracker", 
        "  " + std::to_string(index++) + ". HWND: " + 
        std::to_string((long long)hwnd) + " [" + status + "]");
    }
  }
  
  Logger::Instance().Info("ResourceTracker", "===============================");
}

}  // namespace anywp_engine

