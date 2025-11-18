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

  // v2.3.1+ Enhanced: Aggressive WorkerW creation strategy (Lively-style)
  // Send 0x052C message multiple times with short intervals for better compatibility
  Logger::Instance().Info("DesktopWallpaperHelper", "Triggering WorkerW creation (aggressive mode)");
  
  // CRITICAL FIX: Use SMTO_NORMAL instead of SMTO_ABORTIFHUNG
  // SMTO_ABORTIFHUNG may return too early without waiting for Progman to process
  // Also increase timeout to 1000ms (Lively uses this)
  for (int i = 0; i < 3; i++) {
    DWORD_PTR result = 0;
    LRESULT ret = SendMessageTimeoutW(info_.progman, 0x052C, 0, 0, 
                                       SMTO_NORMAL, 1000, &result);
    if (ret == 0) {
      DWORD error = GetLastError();
      Logger::Instance().Warning("DesktopWallpaperHelper", 
        "SendMessageTimeout failed or timed out (attempt " + std::to_string(i + 1) + 
        "), error: " + std::to_string(error));
    } else {
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "Sent 0x052C to Progman successfully (attempt " + std::to_string(i + 1) + "/3), result: " + std::to_string(result));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }
  
  // Additional trick: Create and immediately destroy a temporary window
  // This helps refresh the desktop layer hierarchy (Lively technique)
  // 
  // IMPORTANT: We ONLY destroy our own temporary window, NEVER destroy system windows!
  // Destroying WorkerW/Progman/SHELLDLL_DefView is dangerous and can crash Explorer.
  // This technique is safe because we create and destroy our own window.
  HWND tmp = CreateWindowExW(WS_EX_TOOLWINDOW, L"STATIC", L"TempWorkerWTrigger",
                              WS_POPUP | WS_VISIBLE, 0, 0, 1, 1,
                              nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
  if (tmp) {
    ShowWindow(tmp, SW_HIDE);
    DestroyWindow(tmp);  // Safe: destroying our own window
    Logger::Instance().Info("DesktopWallpaperHelper", "Created and destroyed temporary trigger window (safe)");
  }
  
  // Wait longer for system to process (v2.3.1+: increased from 200ms to 500ms)
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  
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

  // Step 2: Trigger WorkerW creation (v2.3.1+ CRITICAL FIX: Only trigger ONCE!)
  // Triggering multiple times can confuse Progman and prevent WorkerW creation
  // Lively only triggers once and it works reliably
  if (!TriggerWorkerWCreation()) {
    return false;
  }
  
  // Wait for Progman to process and create WorkerW structure
  // This is critical! Don't immediately check, give system time to reorganize desktop layers
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Step 3: Enumerate WorkerW windows with retry
  auto start_time = std::chrono::steady_clock::now();
  int retry_count = 0;
  
  while (retry_count < 15) {  // v2.1.10+ Increased retries for Windows 11 (was 10)
    if (EnumerateWorkerW()) {
      // v2.1.10+ Fix: Priority - Check if we found a proper WorkerW (not Progman)
      if (info_.wallpaper_layer && info_.wallpaper_layer != info_.progman) {
        Logger::Instance().Info("DesktopWallpaperHelper",
          "WorkerW found successfully on attempt " + std::to_string(retry_count + 1) + " (ideal case)");
        return true;
      }
      // v2.1.10+ Fix: Check if any WorkerW exists at all (even if we use Progman as parent)
      else if (info_.workerw_count > 0 && info_.wallpaper_layer == info_.progman) {
        // v2.1.10+ Lively Integration: If WorkerW exists but we use Progman,
        // this might be because Lively has already set up the correct structure
        Logger::Instance().Info("DesktopWallpaperHelper",
          "WorkerW exists but using Progman parent (possible Lively integration) on attempt " + std::to_string(retry_count + 1));
        Logger::Instance().Info("DesktopWallpaperHelper",
          "Found " + std::to_string(info_.workerw_count) + " WorkerW windows, but SHELLDLL_DefView in Progman");
        return true;
      }
      else if (info_.wallpaper_layer == info_.progman) {
        // v2.1.10+ Windows 11 Fix: When SHELLDLL_DefView is in Progman, this is NORMAL
        // In Windows 11, SHELLDLL_DefView is often inside Progman, not WorkerW
        // Using Progman as parent is the correct approach for Windows 11
        Logger::Instance().Info("DesktopWallpaperHelper",
          "Windows 11 detected: SHELLDLL_DefView in Progman, using Progman as wallpaper parent (this is normal)");
        Logger::Instance().Info("DesktopWallpaperHelper",
          "WorkerW search completed on attempt " + std::to_string(retry_count + 1) + " (Progman mode)");
        return true;
      }
    }
    
    // Check timeout
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    
    if (elapsed.count() >= timeout_ms) {
      Logger::Instance().Error("DesktopWallpaperHelper", 
        "WorkerW search timeout after " + std::to_string(elapsed.count()) + "ms");
      break;
    }
    
    // Wait before retry (longer wait for Windows 11)
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    retry_count++;
    
    // v2.3.1+ CRITICAL FIX: Don't re-trigger on every retry!
    // Only re-trigger after multiple failed attempts (e.g., every 5 retries)
    // Over-triggering can confuse Progman
    if (retry_count % 5 == 0) {
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "Re-triggering WorkerW creation after " + std::to_string(retry_count) + " failed attempts");
      TriggerWorkerWCreation();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
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

// v2.3.1+ Enhanced: Aggressive SHELLDLL_DefView finding (Lively-style)
HWND DesktopWallpaperHelper::FindSHELLDLL_DefView_Aggressive() {
  HWND result = nullptr;
  
  // Strategy 1: Enumerate all top-level windows to find WorkerW containing SHELLDLL_DefView
  EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
    HWND* out = (HWND*)lParam;
    HWND def = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (def) {
      *out = def;
      return FALSE;  // Stop enumeration
    }
    return TRUE;
  }, (LPARAM)&result);
  
  if (result) {
    Logger::Instance().Info("DesktopWallpaperHelper", 
      "Found SHELLDLL_DefView via window enumeration");
    return result;
  }
  
  // Strategy 2: Check Progman directly (Win11 common case)
  if (info_.progman) {
    result = FindWindowExW(info_.progman, nullptr, L"SHELLDLL_DefView", nullptr);
    if (result) {
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "Found SHELLDLL_DefView in Progman");
      return result;
    }
  }
  
  // Strategy 3: Check Desktop window (fallback)
  HWND desktop = GetDesktopWindow();
  if (desktop) {
    result = FindWindowExW(desktop, nullptr, L"SHELLDLL_DefView", nullptr);
    if (result) {
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "Found SHELLDLL_DefView under Desktop");
      return result;
    }
  }
  
  // Strategy 4: Recursive search in all WorkerW windows
  HWND hwnd = nullptr;
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    result = FindChildWindowByClass(hwnd, L"SHELLDLL_DefView");
    if (result) {
      Logger::Instance().Info("DesktopWallpaperHelper", 
        "Found SHELLDLL_DefView via recursive search in WorkerW");
      return result;
    }
  }
  
  Logger::Instance().Warning("DesktopWallpaperHelper", 
    "Could not find SHELLDLL_DefView with any strategy");
  return nullptr;
}

// v2.3.1+ Enhanced: Get Explorer process ID for restart detection
DWORD DesktopWallpaperHelper::GetExplorerProcessId() {
  // Find Shell_TrayWnd (taskbar window created by explorer.exe)
  HWND hShell = FindWindowW(L"Shell_TrayWnd", nullptr);
  if (!hShell) {
    Logger::Instance().Warning("DesktopWallpaperHelper", 
      "Could not find Shell_TrayWnd for Explorer PID detection");
    return 0;
  }
  
  DWORD pid = 0;
  GetWindowThreadProcessId(hShell, &pid);
  
  if (pid > 0) {
    Logger::Instance().Debug("DesktopWallpaperHelper", 
      "Explorer process ID: " + std::to_string(pid));
  }
  
  return pid;
}

}  // namespace anywp_engine

