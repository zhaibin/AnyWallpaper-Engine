#include "desktop_wallpaper_helper.h"
#include "logger.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace anywp_engine {

DesktopWallpaperHelper::DesktopWallpaperHelper() {
}

DesktopWallpaperHelper::~DesktopWallpaperHelper() {
}

bool DesktopWallpaperHelper::FindProgman() {
  info_.progman = FindWindowW(L"Progman", nullptr);
  
  if (!info_.progman) {
    Logger::Instance().Error("DesktopWallpaperHelper", "Progman window not found");
    return false;
  }
  
  Logger::Instance().Info("DesktopWallpaperHelper", 
    "Progman found: " + std::to_string((long long)info_.progman));
  return true;
}

HWND DesktopWallpaperHelper::FindChildWindowByClass(HWND parent, const wchar_t* class_name) {
  if (!parent) {
    return nullptr;
  }

  HWND child = nullptr;
  while ((child = FindWindowExW(parent, child, nullptr, nullptr)) != nullptr) {
    wchar_t current_class[256] = {0};
    if (GetClassNameW(child, current_class, 256) > 0) {
      if (_wcsicmp(current_class, class_name) == 0) {
        return child;
      }
    }

    HWND nested = FindChildWindowByClass(child, class_name);
    if (nested) {
      return nested;
    }
  }

  return nullptr;
}

bool DesktopWallpaperHelper::HasSHELLDLL(HWND hwnd) {
  HWND shelldll = FindChildWindowByClass(hwnd, L"SHELLDLL_DefView");
  if (shelldll) {
    info_.shelldll = shelldll;
    return true;
  }
  return false;
}

bool DesktopWallpaperHelper::TriggerWorkerWCreation() {
  if (!info_.progman) {
    if (!FindProgman()) {
      return false;
    }
  }

  // Send 0x052C message to Progman to create WorkerW windows
  SendMessageTimeoutW(info_.progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
  Logger::Instance().Info("DesktopWallpaperHelper", "Sent 0x052C to Progman");
  
  // Wait a bit for Windows to process
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  
  return true;
}

bool DesktopWallpaperHelper::EnumerateWorkerW() {
  HWND hwnd = nullptr;
  info_.workerw_count = 0;
  info_.found_shelldll = false;
  info_.icon_layer = nullptr;
  info_.wallpaper_layer = nullptr;
  info_.shelldll = nullptr;

  HWND last_workerw = nullptr;

  // Enumerate all WorkerW windows
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    info_.workerw_count++;
    last_workerw = hwnd;
    
    Logger::Instance().Debug("DesktopWallpaperHelper", 
      "Found WorkerW #" + std::to_string(info_.workerw_count) + ": " + std::to_string((long long)hwnd));
    
    // Check if this WorkerW contains SHELLDLL_DefView
    if (HasSHELLDLL(hwnd)) {
      info_.icon_layer = hwnd;
      info_.found_shelldll = true;
      
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "Found icon layer (WorkerW with SHELLDLL_DefView): " + std::to_string((long long)hwnd));
      if (info_.shelldll) {
        Logger::Instance().Debug("DesktopWallpaperHelper", 
          "SHELLDLL_DefView handle: " + std::to_string((long long)info_.shelldll));
      }
      
      // The next WorkerW after the icon layer is the wallpaper layer
      HWND next_workerw = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      if (next_workerw) {
        info_.wallpaper_layer = next_workerw;
        Logger::Instance().Info("DesktopWallpaperHelper", 
          "Found wallpaper layer (next WorkerW): " + std::to_string((long long)next_workerw));
      } else {
        // No next WorkerW, use the icon layer as wallpaper layer
        info_.wallpaper_layer = hwnd;
        Logger::Instance().Info("DesktopWallpaperHelper", 
          "No next WorkerW, using icon layer as wallpaper layer");
      }
      
      return true;
    }
  }

  Logger::Instance().Warning("DesktopWallpaperHelper", 
    "No WorkerW with SHELLDLL_DefView found (total WorkerW: " + std::to_string(info_.workerw_count) + ")");
  
  // Fallback 1: Check if SHELLDLL_DefView is in Progman
  if (info_.progman && HasSHELLDLL(info_.progman)) {
    HWND shelldll = info_.shelldll;
    HWND icon_parent = shelldll ? GetParent(shelldll) : nullptr;
    if (icon_parent && icon_parent != info_.progman) {
      info_.icon_layer = icon_parent;
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "SHELLDLL_DefView parent is WorkerW: " + std::to_string((long long)icon_parent));
    } else {
      info_.icon_layer = info_.progman;
    }

    Logger::Instance().Info("DesktopWallpaperHelper", 
      "SHELLDLL_DefView found in Progman, attempting to find WorkerW behind it");

    // When SHELLDLL is in Progman, we need WorkerW that is BEHIND Progman in Z-order
    // Use Progman itself as the parent - this is the correct approach for this scenario
    info_.wallpaper_layer = info_.progman;
    Logger::Instance().Info("DesktopWallpaperHelper", 
      "Using Progman as wallpaper parent (SHELLDLL is inside Progman)");

    info_.found_shelldll = true;
    return true;
  }
  
  // Fallback 2: Use first WorkerW found (if any)
  if (info_.workerw_count > 0) {
    // Find the first WorkerW
    HWND first_workerw = FindWindowExW(nullptr, nullptr, L"WorkerW", nullptr);
    if (first_workerw) {
      Logger::Instance().Warning("DesktopWallpaperHelper", 
        "No SHELLDLL_DefView found, using first WorkerW as fallback: " + 
        std::to_string((long long)first_workerw));
      info_.wallpaper_layer = first_workerw;
      info_.found_shelldll = false;  // Mark that we didn't find proper structure
      return true;
    }
  }
  
  // Fallback 3: Use Progman directly (last resort)
  if (info_.progman) {
    Logger::Instance().Warning("DesktopWallpaperHelper", 
      "No WorkerW found, using Progman as fallback: " + 
      std::to_string((long long)info_.progman));
    info_.wallpaper_layer = info_.progman;
    info_.found_shelldll = false;
    return true;
  }
  
  Logger::Instance().Error("DesktopWallpaperHelper", 
    "Failed to find any suitable window for wallpaper layer");
  return false;
}

bool DesktopWallpaperHelper::FindWorkerW(int timeout_ms) {
  Logger::Instance().Info("DesktopWallpaperHelper", "Starting WorkerW search...");
  std::cout << "[DesktopWallpaperHelper] ========== WorkerW Search Start (v2.1.5+ Enhanced) ==========" << std::endl;
  
  // Step 1: Find Progman
  if (!FindProgman()) {
    return false;
  }

  // Step 2: Trigger WorkerW creation
  if (!TriggerWorkerWCreation()) {
    return false;
  }

  // Step 3: Enumerate WorkerW windows with retry
  auto start_time = std::chrono::steady_clock::now();
  int retry_count = 0;
  
  while (retry_count < 10) {  // Max 10 retries
    if (EnumerateWorkerW()) {
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "WorkerW found successfully on attempt " + std::to_string(retry_count + 1));
      return true;
    }
    
    // Check timeout
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    
    if (elapsed.count() >= timeout_ms) {
      Logger::Instance().Error("DesktopWallpaperHelper", 
        "WorkerW search timeout after " + std::to_string(elapsed.count()) + "ms");
      break;
    }
    
    // Wait before retry
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    retry_count++;
    
    // Re-trigger on retry
    TriggerWorkerWCreation();
  }

  Logger::Instance().Error("DesktopWallpaperHelper", 
    "Failed to find WorkerW after " + std::to_string(retry_count) + " attempts");
  return false;
}

HWND DesktopWallpaperHelper::GetWallpaperParent() const {
  if (!IsValid()) {
    Logger::Instance().Warning("DesktopWallpaperHelper", 
      "GetWallpaperParent called but WorkerW not valid");
    return nullptr;
  }
  
  return info_.wallpaper_layer;
}

void DesktopWallpaperHelper::Reset() {
  Logger::Instance().Info("DesktopWallpaperHelper", "Resetting cached WorkerW information");
  
  info_.progman = nullptr;
  info_.icon_layer = nullptr;
  info_.wallpaper_layer = nullptr;
  info_.shelldll = nullptr;
  info_.workerw_count = 0;
  info_.found_shelldll = false;
}

}  // namespace anywp_engine

