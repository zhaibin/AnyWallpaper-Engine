// CRITICAL: Include anywp_engine_plugin.h FIRST to get full type definitions
#include "../utils/logger.h"
#include "../utils/resource_tracker.h"
#include <iostream>

// This header defines MonitorInfo
#include "../anywp_engine_plugin.h"

// Now include our own header
#include "window_manager.h"

namespace anywp_engine {

WindowManager::WindowManager() {
  Logger::Instance().Info("WindowManager", "Module initialized");
}

WindowManager::~WindowManager() {
  Logger::Instance().Info("WindowManager", "Module destroyed");
}

HWND WindowManager::CreateWebViewHostWindow(
    bool enable_mouse_transparent,
    const MonitorInfo* monitor,
    HWND parent_window) {
  
  std::cout << "[WindowManager] Creating WebView host window..." << std::endl;

  if (!parent_window) {
    std::cout << "[WindowManager] ERROR: No parent window (WorkerW) available" << std::endl;
    Logger::Instance().Error("WindowManager", "No parent window");
    return nullptr;
  }
  
  std::cout << "[WindowManager] Using parent window (WorkerW): " << parent_window << std::endl;

  // Validate parent window
  if (!IsValidWindowHandle(parent_window)) {
    std::cout << "[WindowManager] ERROR: Parent window (WorkerW) is invalid" << std::endl;
    Logger::Instance().Error("WindowManager", "Invalid parent window");
    return nullptr;
  }

  int x, y, width, height;
  
  // Calculate window dimensions
  if (!CalculateWindowDimensions(monitor, x, y, width, height)) {
    Logger::Instance().Error("WindowManager", "Failed to calculate window dimensions");
    return nullptr;
  }
  
  // Validate dimensions
  if (!ValidateDimensions(width, height)) {
    std::cout << "[WindowManager] ERROR: Invalid window dimensions: " << width << "x" << height << std::endl;
    Logger::Instance().Error("WindowManager", "Invalid dimensions: " + 
      std::to_string(width) + "x" + std::to_string(height));
    return nullptr;
  }
  
  std::cout << "[WindowManager] Creating child window: " << width << "x" << height 
            << " at (" << x << "," << y << ")" << std::endl;
  std::cout << "[WindowManager] Mouse transparent: " << (enable_mouse_transparent ? "enabled" : "disabled") << std::endl;

  // Set extended styles based on mouse transparent mode
  DWORD ex_style = WS_EX_NOACTIVATE;  // Always prevent focus stealing to avoid interfering with MouseHook
  if (enable_mouse_transparent) {
    // Transparent mode: prevent focus + mouse pass-through
    ex_style |= WS_EX_TRANSPARENT;
    std::cout << "[WindowManager] Extended styles: WS_EX_NOACTIVATE | WS_EX_TRANSPARENT (pass-through mode)" << std::endl;
  } else {
    // Interactive mode: prevent focus (keep WS_EX_NOACTIVATE), but remove WS_EX_TRANSPARENT
    // Why keep WS_EX_NOACTIVATE? Because we use MouseHook to inject events, focus changes interfere with this.
    std::cout << "[WindowManager] Extended styles: WS_EX_NOACTIVATE (interactive mode - MouseHook injects events)" << std::endl;
  }

  // Create as CHILD window of WorkerW
  HWND hwnd = CreateWindowExW(
      ex_style,
      L"STATIC",
      L"AnyWallpaperHost",
      WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
      x, y, width, height,
      parent_window,
      nullptr,
      GetModuleHandle(nullptr),
      nullptr);

  if (!hwnd) {
    DWORD error = GetLastError();
    std::cout << "[WindowManager] ERROR: Failed to create window, error: " << error << std::endl;
    Logger::Instance().Error("WindowManager", "CreateWindow failed: error " + std::to_string(error));
    return nullptr;
  }

  // Validate created window
  if (!IsValidWindowHandle(hwnd)) {
    std::cout << "[WindowManager] ERROR: Created window is invalid" << std::endl;
    Logger::Instance().Error("WindowManager", "Created window is invalid");
    DestroyWindow(hwnd);
    return nullptr;
  }

  std::cout << "[WindowManager] WebView host window created successfully: " << hwnd << std::endl;
  
  // Track window for cleanup
  ResourceTracker::Instance().TrackWindow(hwnd);
  
  return hwnd;
}

bool WindowManager::IsValidWindowHandle(HWND hwnd) {
  // Only check if handle is valid, not visibility
  // WorkerW and SHELLDLL_DefView might be hidden
  return (hwnd != nullptr && IsWindow(hwnd));
}

bool WindowManager::IsValidParentWindow(HWND hwnd) {
  if (!hwnd || !IsWindow(hwnd)) {
    return false;
  }
  
  // Additional checks for parent window suitability
  LONG style = GetWindowLongW(hwnd, GWL_STYLE);
  if (style == 0 && GetLastError() != 0) {
    return false;
  }
  
  // Parent window should not be minimized or destroyed
  if (IsIconic(hwnd)) {
    return false;
  }
  
  return true;
}

bool WindowManager::SetWallpaperZOrder(HWND hwnd, HWND worker_w) {
  if (!hwnd || !worker_w) {
    Logger::Instance().Error("WindowManager", "Invalid window handles for Z-order");
    return false;
  }
  
  // Try to find SHELLDLL_DefView recursively in worker_w
  HWND shelldll = FindSHELLDLL_DefView(worker_w);
  
  // If not found in worker_w, try Progman (common scenario after Windows 10 Fall Creators Update)
  if (!shelldll) {
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (progman) {
      shelldll = FindSHELLDLL_DefView(progman);
      if (shelldll) {
        std::cout << "[WindowManager] SHELLDLL_DefView found in Progman instead of WorkerW" << std::endl;
      }
    }
  }
  
  if (!shelldll) {
    std::cout << "[WindowManager] WARNING: Could not find SHELLDLL_DefView, trying HWND_BOTTOM fallback" << std::endl;
    // Fallback: Just place window at bottom of Z-order
    BOOL result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    if (result) {
      std::cout << "[WindowManager] Z-order set using HWND_BOTTOM fallback" << std::endl;
      return true;
    }
    return false;
  }
  
  // Set Z-order: WebView behind SHELLDLL_DefView (desktop icons)
  BOOL result = SetWindowPos(hwnd, shelldll, 0, 0, 0, 0, 
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  
  if (result) {
    std::cout << "[WindowManager] Z-order set: Icons on top, WebView below" << std::endl;
    return true;
  } else {
    DWORD error = GetLastError();
    std::cout << "[WindowManager] WARNING: Failed to set Z-order, error: " << error << std::endl;
    // Try fallback
    result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return result != FALSE;
  }
}

namespace {
HWND FindChildWindowByClassRecursive(HWND parent, const wchar_t* class_name) {
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

    HWND nested = FindChildWindowByClassRecursive(child, class_name);
    if (nested) {
      return nested;
    }
  }

  return nullptr;
}
}  // namespace

HWND WindowManager::FindSHELLDLL_DefView(HWND worker_w) {
  return FindChildWindowByClassRecursive(worker_w, L"SHELLDLL_DefView");
}

bool WindowManager::CalculateWindowDimensions(
    const MonitorInfo* monitor,
    int& out_x,
    int& out_y,
    int& out_width,
    int& out_height) {
  
  if (monitor) {
    // Use specific monitor's dimensions and position
    // WorkerW is a global window covering all monitors (virtual desktop)
    // Use absolute coordinates relative to the virtual desktop origin
    out_x = monitor->left;
    out_y = monitor->top;
    out_width = monitor->width;
    out_height = monitor->height;
    
    std::cout << "[WindowManager] Using monitor: " << monitor->device_name 
              << " [" << out_width << "x" << out_height << "]"
              << " at virtual desktop position (" << out_x << "," << out_y << ")" << std::endl;
    
    return true;
  } else {
    // Legacy: Get work area (desktop minus taskbar)
    RECT workArea = {0};
    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0)) {
      std::cout << "[WindowManager] ERROR: Failed to get work area, using screen dimensions" << std::endl;
      workArea.left = 0;
      workArea.top = 0;
      workArea.right = GetSystemMetrics(SM_CXSCREEN);
      workArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    
    out_x = 0;
    out_y = 0;
    out_width = workArea.right - workArea.left;
    out_height = workArea.bottom - workArea.top;
    
    return true;
  }
}

bool WindowManager::ValidateDimensions(int width, int height) {
  // Validate dimensions are within reasonable bounds
  return (width > 0 && height > 0 && width <= 10000 && height <= 10000);
}

// ========== Desktop Wallpaper Embedding APIs (v2.0.1+) ==========

bool WindowManager::InitializeAsWallpaper(HWND hwnd, bool enable_transparent) {
  if (!hwnd || !IsWindow(hwnd)) {
    Logger::Instance().Error("WindowManager", "Invalid window handle for wallpaper initialization");
    return false;
  }

  try {
    std::cout << "[WindowManager] Initializing window as desktop wallpaper..." << std::endl;
    std::cout << "[WindowManager] Window handle: " << hwnd << std::endl;
    std::cout << "[WindowManager] Transparent mode: " << (enable_transparent ? "enabled" : "disabled") << std::endl;

    // Step 1: Send 0x052C message to Progman to create second WorkerW
    std::cout << "[WindowManager] Step 1: Sending 0x052C to Progman..." << std::endl;
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman) {
      Logger::Instance().Error("WindowManager", "Failed to find Progman window");
      return false;
    }
    std::cout << "[WindowManager] Progman found: " << progman << std::endl;

    // This message triggers Windows to create the second WorkerW
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    std::cout << "[WindowManager] Message sent successfully" << std::endl;

    // Step 2: Find the second WorkerW window
    std::cout << "[WindowManager] Step 2: Finding second WorkerW..." << std::endl;
    HWND workerw = FindSecondWorkerW();
    if (!workerw) {
      Logger::Instance().Error("WindowManager", "Failed to find second WorkerW window");
      return false;
    }
    std::cout << "[WindowManager] Second WorkerW found: " << workerw << std::endl;

    // Step 3: Set window as child of WorkerW
    std::cout << "[WindowManager] Step 3: Embedding window into WorkerW..." << std::endl;
    HWND old_parent = SetParent(hwnd, workerw);
    if (!old_parent && GetLastError() != 0) {
      DWORD error = GetLastError();
      Logger::Instance().Error("WindowManager", 
        "SetParent failed: error " + std::to_string(error));
      return false;
    }
    std::cout << "[WindowManager] Window embedded successfully (old parent: " << old_parent << ")" << std::endl;

    // Step 4: Set window styles
    std::cout << "[WindowManager] Step 4: Setting window styles..." << std::endl;
    
    // Base styles: WS_CHILD + WS_VISIBLE + WS_CLIPSIBLINGS + WS_CLIPCHILDREN
    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    style |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    SetWindowLongW(hwnd, GWL_STYLE, style);
    std::cout << "[WindowManager] Base styles set: WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN" << std::endl;

    // Extended styles: Always keep WS_EX_NOACTIVATE, conditionally add WS_EX_TRANSPARENT
    LONG ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
    // Remove WS_EX_TRANSPARENT first
    ex_style &= ~WS_EX_TRANSPARENT;
    // Always set WS_EX_NOACTIVATE (prevent focus stealing, avoid interfering with MouseHook)
    ex_style |= WS_EX_NOACTIVATE;
    
    if (enable_transparent) {
      // Transparent mode: prevent focus + mouse pass-through
      ex_style |= WS_EX_TRANSPARENT;
      std::cout << "[WindowManager] Extended styles: WS_EX_NOACTIVATE | WS_EX_TRANSPARENT (pass-through mode)" << std::endl;
    } else {
      // Interactive mode: prevent focus (keep WS_EX_NOACTIVATE), but remove WS_EX_TRANSPARENT
      std::cout << "[WindowManager] Extended styles: WS_EX_NOACTIVATE (interactive mode - MouseHook injects events)" << std::endl;
    }
    SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style);

    // Step 5: Position window to cover entire screen
    std::cout << "[WindowManager] Step 5: Positioning window..." << std::endl;
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    std::cout << "[WindowManager] Screen size: " << screen_width << "x" << screen_height << std::endl;

    BOOL pos_result = SetWindowPos(
      hwnd,
      nullptr,
      0, 0,
      screen_width, screen_height,
      SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW
    );

    if (!pos_result) {
      DWORD error = GetLastError();
      std::cout << "[WindowManager] WARNING: SetWindowPos failed with error " << error << std::endl;
    } else {
      std::cout << "[WindowManager] Window positioned successfully" << std::endl;
    }

    // Force window update
    UpdateWindow(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    
    std::cout << "[WindowManager] ✓ Wallpaper initialization complete!" << std::endl;
    Logger::Instance().Info("WindowManager", "Window embedded as desktop wallpaper successfully");
    return true;

  } catch (const std::exception& e) {
    Logger::Instance().Error("WindowManager", 
      std::string("Exception in InitializeAsWallpaper: ") + e.what());
    return false;
  }
}

bool WindowManager::SetInteractive(HWND hwnd, bool interactive) {
  if (!hwnd || !IsWindow(hwnd)) {
    Logger::Instance().Error("WindowManager", "Invalid window handle for SetInteractive");
    return false;
  }

  try {
    LONG ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
    if (ex_style == 0 && GetLastError() != 0) {
      Logger::Instance().Error("WindowManager", "Failed to get window extended style");
      return false;
    }

    if (interactive) {
      // Remove WS_EX_TRANSPARENT - window can receive mouse events via MouseHook
      // Note: Window stays below desktop icons (HWND_BOTTOM), MouseHook forwards events
      ex_style &= ~WS_EX_TRANSPARENT;
      std::cout << "[WindowManager] SetInteractive(true): Removing WS_EX_TRANSPARENT" << std::endl;
    } else {
      // Add WS_EX_TRANSPARENT - mouse passes through
      ex_style |= WS_EX_TRANSPARENT;
      std::cout << "[WindowManager] SetInteractive(false): Adding WS_EX_TRANSPARENT" << std::endl;
    }

    // Always keep WS_EX_NOACTIVATE to prevent focus stealing
    ex_style |= WS_EX_NOACTIVATE;

    SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style);
    
    // Force window to update its style
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    std::cout << "[WindowManager] Interactive mode: " << (interactive ? "ENABLED" : "DISABLED") << std::endl;
    Logger::Instance().Info("WindowManager", 
      std::string("Window interactive mode changed to ") + (interactive ? "true" : "false"));
    return true;

  } catch (const std::exception& e) {
    Logger::Instance().Error("WindowManager", 
      std::string("Exception in SetInteractive: ") + e.what());
    return false;
  }
}

HWND WindowManager::FindSecondWorkerW() {
  HWND workerw = nullptr;
  HWND hwnd = nullptr;
  
  std::cout << "[WindowManager] Enumerating WorkerW windows..." << std::endl;
  
  // Enumerate all WorkerW windows
  int count = 0;
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    count++;
    std::cout << "[WindowManager] Found WorkerW #" << count << ": " << hwnd << std::endl;
    
    // Check if this WorkerW contains SHELLDLL_DefView
    HWND shelldll = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shelldll) {
      std::cout << "[WindowManager] This WorkerW contains SHELLDLL_DefView (desktop icons)" << std::endl;
      
      // The WorkerW we want is the NEXT one (sibling)
      workerw = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      if (workerw) {
        std::cout << "[WindowManager] Found second WorkerW (wallpaper layer): " << workerw << std::endl;
        return workerw;
      }
    }
  }
  
  std::cout << "[WindowManager] Total WorkerW windows found: " << count << std::endl;
  std::cout << "[WindowManager] WARNING: Could not find second WorkerW" << std::endl;
  
  return nullptr;
}

}  // namespace anywp_engine

