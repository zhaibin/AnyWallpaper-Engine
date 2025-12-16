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

// v2.6.7 FIX: Callback for EnumChildWindows to find orphaned AnyWallpaperHost windows
static BOOL CALLBACK EnumChildOrphanedWindowsProc(HWND hwnd, LPARAM lParam) {
  std::vector<HWND>* orphans = reinterpret_cast<std::vector<HWND>*>(lParam);
  
  wchar_t class_name[256] = {0};
  wchar_t window_text[256] = {0};
  
  if (GetClassNameW(hwnd, class_name, 256) > 0) {
    GetWindowTextW(hwnd, window_text, 256);
    
    // Check for our AnyWallpaperHost window
    if (wcscmp(class_name, L"STATIC") == 0 && 
        wcscmp(window_text, L"AnyWallpaperHost") == 0) {
      orphans->push_back(hwnd);
    }
  }
  
  return TRUE;
}

// v2.6.7 FIX: Callback for EnumWindows to find orphaned AnyWallpaperHost windows
static BOOL CALLBACK EnumOrphanedWindowsProc(HWND hwnd, LPARAM lParam) {
  std::vector<HWND>* orphans = reinterpret_cast<std::vector<HWND>*>(lParam);
  
  wchar_t class_name[256] = {0};
  wchar_t window_text[256] = {0};
  
  if (GetClassNameW(hwnd, class_name, 256) > 0) {
    GetWindowTextW(hwnd, window_text, 256);
    
    // Check for our AnyWallpaperHost window
    if (wcscmp(class_name, L"STATIC") == 0 && 
        wcscmp(window_text, L"AnyWallpaperHost") == 0) {
      orphans->push_back(hwnd);
    }
    
    // Also check if this is WorkerW or Progman (desktop parents) - search their children
    if (wcscmp(class_name, L"WorkerW") == 0 || wcscmp(class_name, L"Progman") == 0) {
      EnumChildWindows(hwnd, EnumChildOrphanedWindowsProc, lParam);
    }
  }
  
  return TRUE;
}

// v2.6.7 FIX: Helper to forcibly hide a window
static void ForceHideWindow(HWND hwnd, bool change_title = false) {
  if (!IsWindow(hwnd)) return;
  
  // Hide the window
  ShowWindow(hwnd, SW_HIDE);
  
  // Move off screen
  SetWindowPos(hwnd, nullptr, -32000, -32000, 0, 0, 
    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  
  // Make fully transparent
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, 
    GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
  
  // Set window size to 0
  SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  
  // v2.6.7 FIX: Change the window title so FindWindowW won't find it again
  if (change_title) {
    SetWindowTextW(hwnd, L"");
  }
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
        // v2.6.7 FIX: Hide first to prevent visual artifacts
        ForceHideWindow(hwnd);
        
        if (DestroyWindow(hwnd)) {
          destroyed_count++;
          Logger::Instance().Debug("ResourceTracker", 
            "Destroyed window: " + std::to_string((long long)hwnd));
        } else {
          DWORD error = GetLastError();
          Logger::Instance().Warning("ResourceTracker", 
            "Failed to destroy window: " + std::to_string((long long)hwnd) + 
            " (Error: " + std::to_string(error) + ") - window is hidden");
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
  
  // v2.6.7 FIX: Also use FindWindowW to find any remaining AnyWallpaperHost windows
  // This is more reliable than EnumWindows for finding our specific windows
  HWND found = nullptr;
  int find_iterations = 0;
  const int MAX_FIND_ITERATIONS = 100; // Prevent infinite loop
  
  while ((found = FindWindowW(L"STATIC", L"AnyWallpaperHost")) != nullptr && 
         find_iterations < MAX_FIND_ITERATIONS) {
    find_iterations++;
    
    // Check if we already have this window in our list
    bool already_in_list = false;
    for (HWND h : orphans) {
      if (h == found) {
        already_in_list = true;
        break;
      }
    }
    
    if (!already_in_list) {
      orphans.push_back(found);
      Logger::Instance().Info("ResourceTracker", 
        "Found AnyWallpaperHost via FindWindowW: " + std::to_string((long long)found));
    }
    
    // Hide and change title so FindWindowW won't find it again
    ForceHideWindow(found, true);
    
    // Try to destroy
    DestroyWindow(found);
  }
  
  size_t orphan_count = 0;
  size_t orphan_hidden = 0;
  for (HWND orphan : orphans) {
    if (IsWindow(orphan)) {
      // Hide first to prevent white screen
      ForceHideWindow(orphan);
      
      if (DestroyWindow(orphan)) {
        orphan_count++;
        Logger::Instance().Warning("ResourceTracker", 
          "Destroyed orphaned AnyWallpaperHost window: " + std::to_string((long long)orphan));
      } else {
        DWORD error = GetLastError();
        Logger::Instance().Warning("ResourceTracker", 
          "Failed to destroy orphaned window: " + std::to_string((long long)orphan) +
          " (Error: " + std::to_string(error) + "), window is hidden");
        orphan_hidden++;
      }
    }
  }

  Logger::Instance().Info("ResourceTracker", 
    "Cleanup complete: " + std::to_string(destroyed_count) + " tracked, " +
    std::to_string(already_destroyed_count) + " already destroyed, " +
    std::to_string(orphan_count) + " orphaned destroyed, " +
    std::to_string(orphan_hidden) + " orphaned hidden, " +
    std::to_string(find_iterations) + " FindWindowW iterations");
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

