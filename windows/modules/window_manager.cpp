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
  // Note: When Explorer restarts, child windows will be destroyed automatically.
  // The recovery mechanism will detect this and recreate everything (Lively-style approach)
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
  
  // v2.1.10+ Fix: Verify window visibility immediately after creation
  std::cout << "[WindowManager] Verifying window visibility..." << std::endl;
  BOOL is_visible = IsWindowVisible(hwnd);
  BOOL is_window = IsWindow(hwnd);
  HWND actual_parent = GetParent(hwnd);
  
  std::cout << "[WindowManager] Window validation:" << std::endl;
  std::cout << "[WindowManager]   - IsWindow: " << (is_window ? "YES" : "NO") << std::endl;
  std::cout << "[WindowManager]   - IsWindowVisible: " << (is_visible ? "YES" : "NO") << std::endl;
  std::cout << "[WindowManager]   - Parent window: " << actual_parent << " (expected: " << parent_window << ")" << std::endl;
  
  // If window is not visible, force show it
  if (!is_visible) {
    std::cout << "[WindowManager] WARNING: Window is not visible, forcing ShowWindow..." << std::endl;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    
    // Verify again
    is_visible = IsWindowVisible(hwnd);
    std::cout << "[WindowManager] After ShowWindow, IsWindowVisible: " << (is_visible ? "YES" : "NO") << std::endl;
  }
  
  // Verify parent window is valid and visible
  if (actual_parent && actual_parent != parent_window) {
    std::cout << "[WindowManager] WARNING: Parent window mismatch!" << std::endl;
  }
  
  if (actual_parent && IsWindow(actual_parent)) {
    BOOL parent_visible = IsWindowVisible(actual_parent);
    std::cout << "[WindowManager] Parent window visible: " << (parent_visible ? "YES" : "NO") << std::endl;
    if (!parent_visible) {
      std::cout << "[WindowManager] WARNING: Parent window (WorkerW) is not visible!" << std::endl;
    }
  }
  
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
  
  // v2.1.10+ Windows 11 Fix: Check if worker_w is actually Progman
  // In Windows 11, when SHELLDLL_DefView is in Progman, we use Progman as parent
  HWND progman = FindWindowW(L"Progman", nullptr);
  bool is_progman_parent = (progman && worker_w == progman);
  
  // Try to find SHELLDLL_DefView recursively in worker_w
  HWND shelldll = FindSHELLDLL_DefView(worker_w);
  
  // If not found in worker_w, try Progman (common scenario after Windows 10 Fall Creators Update)
  if (!shelldll) {
    if (progman) {
      shelldll = FindSHELLDLL_DefView(progman);
      if (shelldll) {
        std::cout << "[WindowManager] SHELLDLL_DefView found in Progman instead of WorkerW" << std::endl;
      }
    }
  }
  
  // v2.1.10+ Windows 11 Fix: When using Progman as parent, ensure window is properly positioned
  if (is_progman_parent && shelldll) {
    std::cout << "[WindowManager] Windows 11 mode: Using Progman as parent, SHELLDLL_DefView found" << std::endl;
    std::cout << "[WindowManager] CRITICAL: Window MUST be below desktop icons (SHELLDLL_DefView)" << std::endl;
  }
  
  if (!shelldll) {
    std::cout << "[WindowManager] WARNING: Could not find SHELLDLL_DefView, trying HWND_BOTTOM fallback" << std::endl;
    // Fallback: Just place window at bottom of Z-order
    BOOL result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    if (result) {
      std::cout << "[WindowManager] Z-order set using HWND_BOTTOM fallback" << std::endl;
      // v2.1.10+ Fix: Force window update after Z-order change
      UpdateWindow(hwnd);
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
      
      // Verify window is still visible after Z-order change
      BOOL still_visible = IsWindowVisible(hwnd);
      if (!still_visible) {
        std::cout << "[WindowManager] WARNING: Window became invisible after Z-order change, forcing show..." << std::endl;
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
      }
      
      return true;
    }
    return false;
  }
  
  // v2.1.10+ CRITICAL: Set Z-order: WebView behind SHELLDLL_DefView (desktop icons)
  // This ensures desktop icons are always visible on top of the wallpaper
  std::cout << "[WindowManager] Setting Z-order: Window behind SHELLDLL_DefView (desktop icons)" << std::endl;
  std::cout << "[WindowManager] SHELLDLL_DefView HWND: " << shelldll << std::endl;
  
  // v2.1.10+ Fix: Check if window is a child window
  LONG style = GetWindowLongW(hwnd, GWL_STYLE);
  bool is_child = (style & WS_CHILD) != 0;
  HWND parent = GetParent(hwnd);
  
  std::cout << "[WindowManager] Window is child: " << (is_child ? "YES" : "NO") << std::endl;
  if (is_child) {
    std::cout << "[WindowManager] Parent window: " << parent << std::endl;
  }
  
  BOOL result = FALSE;

  // v2.1.10+ Fix: For child windows, we need to set Z-order within the parent's child window chain
  // If both windows are children of the same parent, use SetWindowPos with shelldll
  // Otherwise, use HWND_BOTTOM to place at bottom of parent's child chain
  if (is_child && parent) {
    HWND shelldll_parent = GetParent(shelldll);
    std::cout << "[WindowManager] SHELLDLL_DefView parent: " << shelldll_parent << std::endl;

    if (shelldll_parent == parent) {
      // Both are children of the same parent - can use SetWindowPos with shelldll
      std::cout << "[WindowManager] Both windows are siblings, using SetWindowPos with SHELLDLL_DefView" << std::endl;
      result = SetWindowPos(hwnd, shelldll, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

      // v2.1.10+ CRITICAL FIX: If SetWindowPos succeeded but window is at bottom,
      // this indicates the Z-order setting didn't work as expected
      if (result) {
        std::cout << "[WindowManager] SetWindowPos succeeded, checking if window is at bottom..." << std::endl;

        // Check if our window is at the bottom of Z-order (indicating failure)
        HWND window_after = GetWindow(hwnd, GW_HWNDNEXT);
        if (window_after == nullptr) {
          std::cout << "[WindowManager] ⚠️  WARNING: Window is at Z-order bottom despite successful SetWindowPos!" << std::endl;
          std::cout << "[WindowManager] ⚠️  This suggests Z-order setting failed. Trying alternative approach..." << std::endl;

          // Alternative approach: Find SHELLDLL_DefView's position in sibling chain
          // and place our window right after it
          std::cout << "[WindowManager] Finding SHELLDLL_DefView position in sibling chain..." << std::endl;

          // Start from the first sibling
          HWND current = FindWindowExW(parent, nullptr, nullptr, nullptr);
          HWND shelldll_next = nullptr;

          // Find the window after SHELLDLL_DefView
          while (current != nullptr) {
            if (current == shelldll) {
              shelldll_next = GetWindow(current, GW_HWNDNEXT);
              break;
            }
            current = GetWindow(current, GW_HWNDNEXT);
          }

          if (shelldll_next != nullptr) {
            std::cout << "[WindowManager] Found window after SHELLDLL_DefView: " << shelldll_next << std::endl;
            std::cout << "[WindowManager] Placing our window before this window..." << std::endl;
            result = SetWindowPos(hwnd, shelldll_next, 0, 0, 0, 0,
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            std::cout << "[WindowManager] Alternative Z-order setting result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
          } else {
            std::cout << "[WindowManager] SHELLDLL_DefView is the last window, placing at bottom..." << std::endl;
            result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
          }
        }
      }
    } else {
      // Different parents - need to place at bottom of parent's child chain
      std::cout << "[WindowManager] Windows have different parents, using HWND_BOTTOM in parent's child chain" << std::endl;
      // First, try to find the first child of parent to place before it
      HWND first_child = FindWindowExW(parent, nullptr, nullptr, nullptr);
      if (first_child && first_child != hwnd) {
        // Place before first child (which should be at bottom of Z-order)
        result = SetWindowPos(hwnd, first_child, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        std::cout << "[WindowManager] Placed before first child: " << first_child << std::endl;
      } else {
        // Fallback: Use HWND_BOTTOM
        result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      }
    }
  } else {
    // Not a child window - use standard SetWindowPos
    result = SetWindowPos(hwnd, shelldll, 0, 0, 0, 0,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
  
  if (result) {
    std::cout << "[WindowManager] ✓ Z-order set successfully: Icons on top, WebView below" << std::endl;
    
    // v2.1.10+ Fix: Force window update after Z-order change
    UpdateWindow(hwnd);
    RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    
    // v2.1.10+ CRITICAL: Verify Z-order is correct by checking window position relative to SHELLDLL_DefView
    // For child windows, check sibling order
    if (is_child && parent) {
      HWND current = hwnd;
      bool found_shelldll_after = false;
      // Check if SHELLDLL_DefView comes after our window in the sibling chain
      while ((current = GetWindow(current, GW_HWNDNEXT)) != nullptr) {
        if (current == shelldll) {
          found_shelldll_after = true;
          break;
        }
      }
      if (found_shelldll_after) {
        std::cout << "[WindowManager] ✓ Z-order verified: Window is correctly behind SHELLDLL_DefView (sibling check)" << std::endl;
      } else {
        std::cout << "[WindowManager] ⚠️  Z-order verification: SHELLDLL_DefView not found after window in sibling chain" << std::endl;
      }
    } else {
      // For non-child windows, use standard check
      HWND window_after = GetWindow(hwnd, GW_HWNDNEXT);
      if (window_after == shelldll) {
        std::cout << "[WindowManager] ✓ Z-order verified: Window is correctly behind SHELLDLL_DefView" << std::endl;
      } else {
        std::cout << "[WindowManager] ⚠️  Z-order verification: Window after is " << window_after << " (expected: " << shelldll << ")" << std::endl;
      }
    }
    
    // Verify window is still visible after Z-order change
    BOOL still_visible = IsWindowVisible(hwnd);
    if (!still_visible) {
      std::cout << "[WindowManager] WARNING: Window became invisible after Z-order change, forcing show..." << std::endl;
      ShowWindow(hwnd, SW_SHOW);
      UpdateWindow(hwnd);
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    } else {
      std::cout << "[WindowManager] ✓ Window is visible after Z-order change" << std::endl;
    }
    
    return true;
  } else {
    DWORD error = GetLastError();
    std::cout << "[WindowManager] ERROR: Failed to set Z-order behind SHELLDLL_DefView, error: " << error << std::endl;
    std::cout << "[WindowManager] Attempting HWND_BOTTOM fallback..." << std::endl;
    
    // Try fallback: Place at bottom of Z-order
    result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    if (result) {
      std::cout << "[WindowManager] Z-order set using HWND_BOTTOM fallback" << std::endl;
      // Force window update after fallback Z-order change
      UpdateWindow(hwnd);
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
      
      // Verify visibility
      BOOL still_visible = IsWindowVisible(hwnd);
      if (!still_visible) {
        std::cout << "[WindowManager] WARNING: Window invisible after fallback, forcing show..." << std::endl;
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
      }
    } else {
      std::cout << "[WindowManager] ERROR: HWND_BOTTOM fallback also failed!" << std::endl;
    }
    
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

bool WindowManager::DiagnoseWindowVisibility(HWND hwnd, HWND worker_w) {
  if (!hwnd || !IsWindow(hwnd)) {
    std::cout << "[WindowManager] [Diagnosis] Window handle is invalid" << std::endl;
    return false;
  }
  
  std::cout << "[WindowManager] ========== Window Visibility Diagnosis ==========" << std::endl;
  std::cout << "[WindowManager] [Diagnosis] Window HWND: " << hwnd << std::endl;
  
  // Check 1: Window validity
  BOOL is_window = IsWindow(hwnd);
  std::cout << "[WindowManager] [Diagnosis] IsWindow: " << (is_window ? "YES" : "NO") << std::endl;
  if (!is_window) {
    std::cout << "[WindowManager] [Diagnosis] ❌ Window handle is invalid!" << std::endl;
    return false;
  }
  
  // Check 2: Window visibility
  BOOL is_visible = IsWindowVisible(hwnd);
  std::cout << "[WindowManager] [Diagnosis] IsWindowVisible: " << (is_visible ? "YES" : "NO") << std::endl;
  
  // Check 3: Window styles
  LONG style = GetWindowLongW(hwnd, GWL_STYLE);
  bool has_visible_style = (style & WS_VISIBLE) != 0;
  bool has_child_style = (style & WS_CHILD) != 0;
  std::cout << "[WindowManager] [Diagnosis] Window styles:" << std::endl;
  std::cout << "[WindowManager] [Diagnosis]   - WS_VISIBLE: " << (has_visible_style ? "YES" : "NO") << std::endl;
  std::cout << "[WindowManager] [Diagnosis]   - WS_CHILD: " << (has_child_style ? "YES" : "NO") << std::endl;
  
  // Check 4: Extended styles
  LONG ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
  bool has_transparent = (ex_style & WS_EX_TRANSPARENT) != 0;
  bool has_noactivate = (ex_style & WS_EX_NOACTIVATE) != 0;
  std::cout << "[WindowManager] [Diagnosis] Extended styles:" << std::endl;
  std::cout << "[WindowManager] [Diagnosis]   - WS_EX_TRANSPARENT: " << (has_transparent ? "YES" : "NO") << std::endl;
  std::cout << "[WindowManager] [Diagnosis]   - WS_EX_NOACTIVATE: " << (has_noactivate ? "YES" : "NO") << std::endl;
  
  // Check 5: Window position and size
  RECT rect;
  if (GetWindowRect(hwnd, &rect)) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    std::cout << "[WindowManager] [Diagnosis] Window rect: (" << rect.left << "," << rect.top 
              << ") " << width << "x" << height << std::endl;
    if (width == 0 || height == 0) {
      std::cout << "[WindowManager] [Diagnosis] ⚠️  Window has zero size!" << std::endl;
    }
  } else {
    std::cout << "[WindowManager] [Diagnosis] ⚠️  Failed to get window rect" << std::endl;
  }
  
  // Check 6: Parent window
  HWND parent = GetParent(hwnd);
  std::cout << "[WindowManager] [Diagnosis] Parent window: " << parent << " (expected: " << worker_w << ")" << std::endl;
  if (parent != worker_w) {
    std::cout << "[WindowManager] [Diagnosis] ⚠️  Parent window mismatch!" << std::endl;
  }
  
  if (parent && IsWindow(parent)) {
    BOOL parent_visible = IsWindowVisible(parent);
    std::cout << "[WindowManager] [Diagnosis] Parent visible: " << (parent_visible ? "YES" : "NO") << std::endl;
    if (!parent_visible) {
      std::cout << "[WindowManager] [Diagnosis] ❌ Parent window is not visible!" << std::endl;
    }
  } else {
    std::cout << "[WindowManager] [Diagnosis] ❌ Parent window is invalid!" << std::endl;
  }
  
  // Check 7: Window Z-order (check if window is behind desktop icons)
  HWND shelldll = FindSHELLDLL_DefView(worker_w);
  if (shelldll) {
    std::cout << "[WindowManager] [Diagnosis] SHELLDLL_DefView found: " << shelldll << std::endl;
    // Check if our window is actually behind shelldll
    HWND window_after = GetWindow(hwnd, GW_HWNDNEXT);
    std::cout << "[WindowManager] [Diagnosis] Window after in Z-order: " << window_after << std::endl;
  } else {
    std::cout << "[WindowManager] [Diagnosis] ⚠️  SHELLDLL_DefView not found" << std::endl;
  }
  
  // Summary
  bool should_be_visible = is_window && is_visible && has_visible_style && 
                          parent && IsWindow(parent) && IsWindowVisible(parent) &&
                          (rect.right - rect.left) > 0 && (rect.bottom - rect.top) > 0;
  
  std::cout << "[WindowManager] [Diagnosis] ========== Summary ==========" << std::endl;
  std::cout << "[WindowManager] [Diagnosis] Window should be visible: " << (should_be_visible ? "YES" : "NO") << std::endl;
  
  if (!should_be_visible) {
    std::cout << "[WindowManager] [Diagnosis] ❌ Visibility issues detected!" << std::endl;
  } else {
    std::cout << "[WindowManager] [Diagnosis] ✓ Window appears to be properly configured" << std::endl;
  }
  
  return should_be_visible;
}

}  // namespace anywp_engine

