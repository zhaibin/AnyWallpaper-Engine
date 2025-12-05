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
  
  Logger::Instance().Info("WindowManager", "Creating WebView host window...");

  if (!parent_window) {
    Logger::Instance().Error("WindowManager", "No parent window (WorkerW) available");
    Logger::Instance().Error("WindowManager", "No parent window");
    return nullptr;
  }
  
  Logger::Instance().Info("WindowManager", 
    "Using parent window (WorkerW): " + std::to_string(reinterpret_cast<uintptr_t>(parent_window)));

  // Validate parent window
  if (!IsValidWindowHandle(parent_window)) {
    Logger::Instance().Error("WindowManager", "Parent window (WorkerW) is invalid");
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
    Logger::Instance().Error("WindowManager", "Invalid window dimensions: " + 
      std::to_string(width) + "x" + std::to_string(height));
    Logger::Instance().Error("WindowManager", "Invalid dimensions: " + 
      std::to_string(width) + "x" + std::to_string(height));
    return nullptr;
  }
  
  Logger::Instance().Info("WindowManager", 
    "Creating child window: " + std::to_string(width) + "x" + std::to_string(height) + 
    " at (" + std::to_string(x) + "," + std::to_string(y) + ")");
  Logger::Instance().Info("WindowManager", 
    std::string("Mouse transparent: ") + (enable_mouse_transparent ? "enabled" : "disabled"));

  // Set extended styles based on mouse transparent mode
  DWORD ex_style = WS_EX_NOACTIVATE;  // Always prevent focus stealing to avoid interfering with MouseHook
  if (enable_mouse_transparent) {
    // Transparent mode: prevent focus + mouse pass-through
    ex_style |= WS_EX_TRANSPARENT;
    Logger::Instance().Info("WindowManager", 
      "Extended styles: WS_EX_NOACTIVATE | WS_EX_TRANSPARENT (pass-through mode)");
  } else {
    // Interactive mode: prevent focus (keep WS_EX_NOACTIVATE), but remove WS_EX_TRANSPARENT
    // Why keep WS_EX_NOACTIVATE? Because we use MouseHook to inject events, focus changes interfere with this.
    Logger::Instance().Info("WindowManager", 
      "Extended styles: WS_EX_NOACTIVATE (interactive mode - MouseHook injects events)");
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
    Logger::Instance().Error("WindowManager", "Failed to create window, error: " + std::to_string(error));
    Logger::Instance().Error("WindowManager", "CreateWindow failed: error " + std::to_string(error));
    return nullptr;
  }

  // Validate created window
  if (!IsValidWindowHandle(hwnd)) {
    Logger::Instance().Error("WindowManager", "Created window is invalid");
    Logger::Instance().Error("WindowManager", "Created window is invalid");
    DestroyWindow(hwnd);
    return nullptr;
  }

  Logger::Instance().Info("WindowManager", 
    "WebView host window created successfully: " + std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
  
  // Track window for cleanup
  ResourceTracker::Instance().TrackWindow(hwnd);
  
  // v2.1.10+ Fix: Verify window visibility immediately after creation
  Logger::Instance().Info("WindowManager", "Verifying window visibility...");
  BOOL is_visible = IsWindowVisible(hwnd);
  BOOL is_window = IsWindow(hwnd);
  HWND actual_parent = GetParent(hwnd);
  
  Logger::Instance().Info("WindowManager", "Window validation:");
  Logger::Instance().Info("WindowManager", 
    std::string("  - IsWindow: ") + (is_window ? "YES" : "NO"));
  Logger::Instance().Info("WindowManager", 
    std::string("  - IsWindowVisible: ") + (is_visible ? "YES" : "NO"));
  Logger::Instance().Info("WindowManager", 
    "  - Parent window: " + std::to_string(reinterpret_cast<uintptr_t>(actual_parent)) + 
    " (expected: " + std::to_string(reinterpret_cast<uintptr_t>(parent_window)) + ")");
  
  // If window is not visible, force show it
  if (!is_visible) {
    Logger::Instance().Warning("WindowManager", "Window is not visible, forcing ShowWindow...");
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    
    // Verify again
    is_visible = IsWindowVisible(hwnd);
    Logger::Instance().Info("WindowManager", 
      std::string("After ShowWindow, IsWindowVisible: ") + (is_visible ? "YES" : "NO"));
  }
  
  // Verify parent window is valid and visible
  if (actual_parent && actual_parent != parent_window) {
    Logger::Instance().Warning("WindowManager", "Parent window mismatch!");
  }
  
  if (actual_parent && IsWindow(actual_parent)) {
    BOOL parent_visible = IsWindowVisible(actual_parent);
    Logger::Instance().Info("WindowManager", 
      std::string("Parent window visible: ") + (parent_visible ? "YES" : "NO"));
    if (!parent_visible) {
      Logger::Instance().Warning("WindowManager", "Parent window (WorkerW) is not visible!");
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
        Logger::Instance().Info("WindowManager", 
          "SHELLDLL_DefView found in Progman instead of WorkerW");
      }
    }
  }
  
  // v2.1.10+ Windows 11 Fix: When using Progman as parent, ensure window is properly positioned
  if (is_progman_parent && shelldll) {
    Logger::Instance().Info("WindowManager", 
      "Windows 11 mode: Using Progman as parent, SHELLDLL_DefView found");
    Logger::Instance().Info("WindowManager", 
      "CRITICAL: Window MUST be below desktop icons (SHELLDLL_DefView)");
  }
  
  if (!shelldll) {
    Logger::Instance().Warning("WindowManager", 
      "Could not find SHELLDLL_DefView, trying HWND_BOTTOM fallback");
    // Fallback: Just place window at bottom of Z-order
    BOOL result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    if (result) {
      Logger::Instance().Info("WindowManager", "Z-order set using HWND_BOTTOM fallback");
      // v2.1.10+ Fix: Force window update after Z-order change
      UpdateWindow(hwnd);
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
      
      // Verify window is still visible after Z-order change
      BOOL still_visible = IsWindowVisible(hwnd);
      if (!still_visible) {
        Logger::Instance().Warning("WindowManager", 
          "Window became invisible after Z-order change, forcing show...");
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
      }
      
      return true;
    }
    return false;
  }
  
  // v2.4.1+ CRITICAL: Ensure WorkerW itself is at the bottom of Z-order
  // This is essential for Windows 11 Raised Desktop mode to ensure WorkerW doesn't cover icons
  EnsureWorkerWZOrder(worker_w);

  // v2.1.10+ CRITICAL: Set Z-order: WebView behind SHELLDLL_DefView (desktop icons)
  // This ensures desktop icons are always visible on top of the wallpaper
  Logger::Instance().Info("WindowManager", 
    "Setting Z-order: Window behind SHELLDLL_DefView (desktop icons)");
  Logger::Instance().Info("WindowManager", 
    "SHELLDLL_DefView HWND: " + std::to_string(reinterpret_cast<uintptr_t>(shelldll)));
  
  // v2.1.10+ Fix: Check if window is a child window
  LONG style = GetWindowLongW(hwnd, GWL_STYLE);
  bool is_child = (style & WS_CHILD) != 0;
  HWND parent = GetParent(hwnd);
  
  Logger::Instance().Info("WindowManager", 
    std::string("Window is child: ") + (is_child ? "YES" : "NO"));
  if (is_child) {
    Logger::Instance().Info("WindowManager", 
      "Parent window: " + std::to_string(reinterpret_cast<uintptr_t>(parent)));
  }
  
  BOOL result = FALSE;

  // v2.1.10+ Fix: For child windows, we need to set Z-order within the parent's child window chain
  // If both windows are children of the same parent, use SetWindowPos with shelldll
  // Otherwise, use HWND_BOTTOM to place at bottom of parent's child chain
  if (is_child && parent) {
    HWND shelldll_parent = GetParent(shelldll);
    Logger::Instance().Info("WindowManager", 
      "SHELLDLL_DefView parent: " + std::to_string(reinterpret_cast<uintptr_t>(shelldll_parent)));

    if (shelldll_parent == parent) {
      // Both are children of the same parent - can use SetWindowPos with shelldll
      Logger::Instance().Info("WindowManager", 
        "Both windows are siblings, using SetWindowPos with SHELLDLL_DefView");
      result = SetWindowPos(hwnd, shelldll, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

      // v2.1.10+ CRITICAL FIX: If SetWindowPos succeeded but window is at bottom,
      // this indicates the Z-order setting didn't work as expected
      if (result) {
        Logger::Instance().Info("WindowManager", "SetWindowPos succeeded, checking if window is at bottom...");

        // Check if our window is at the bottom of Z-order (indicating failure)
        HWND window_after = GetWindow(hwnd, GW_HWNDNEXT);
        if (window_after == nullptr) {
          Logger::Instance().Warning("WindowManager", 
            "Window is at Z-order bottom despite successful SetWindowPos!");
          Logger::Instance().Warning("WindowManager", 
            "This suggests Z-order setting failed. Trying alternative approach...");

          // Alternative approach: Find SHELLDLL_DefView's position in sibling chain
          // and place our window right after it
          Logger::Instance().Info("WindowManager", 
            "Finding SHELLDLL_DefView position in sibling chain...");

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
            Logger::Instance().Info("WindowManager", 
              "Found window after SHELLDLL_DefView: " + std::to_string(reinterpret_cast<uintptr_t>(shelldll_next)));
            Logger::Instance().Info("WindowManager", "Placing our window before this window...");
            result = SetWindowPos(hwnd, shelldll_next, 0, 0, 0, 0,
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            Logger::Instance().Info("WindowManager", 
              std::string("Alternative Z-order setting result: ") + (result ? "SUCCESS" : "FAILED"));
          } else {
            Logger::Instance().Info("WindowManager", 
              "SHELLDLL_DefView is the last window, placing at bottom...");
            result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
          }
        }
      }
    } else {
      // Different parents - need to place at bottom of parent's child chain
      Logger::Instance().Info("WindowManager", 
        "Windows have different parents, using HWND_BOTTOM in parent's child chain");
      // First, try to find the first child of parent to place before it
      HWND first_child = FindWindowExW(parent, nullptr, nullptr, nullptr);
      if (first_child && first_child != hwnd) {
        // Place before first child (which should be at bottom of Z-order)
        result = SetWindowPos(hwnd, first_child, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        Logger::Instance().Info("WindowManager", 
          "Placed before first child: " + std::to_string(reinterpret_cast<uintptr_t>(first_child)));
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
    Logger::Instance().Info("WindowManager", "[OK] Z-order set successfully: Icons on top, WebView below");
    
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
        Logger::Instance().Info("WindowManager", 
          "[OK] Z-order verified: Window is correctly behind SHELLDLL_DefView (sibling check)");
      } else {
        Logger::Instance().Warning("WindowManager", 
          "Z-order verification: SHELLDLL_DefView not found after window in sibling chain");
      }
    } else {
      // For non-child windows, use standard check
      HWND window_after = GetWindow(hwnd, GW_HWNDNEXT);
      if (window_after == shelldll) {
        Logger::Instance().Info("WindowManager", 
          "[OK] Z-order verified: Window is correctly behind SHELLDLL_DefView");
      } else {
        Logger::Instance().Warning("WindowManager", 
          "Z-order verification: Window after is " + std::to_string(reinterpret_cast<uintptr_t>(window_after)) + 
          " (expected: " + std::to_string(reinterpret_cast<uintptr_t>(shelldll)) + ")");
      }
    }
    
    // Verify window is still visible after Z-order change
    BOOL still_visible = IsWindowVisible(hwnd);
    if (!still_visible) {
      Logger::Instance().Warning("WindowManager", 
        "Window became invisible after Z-order change, forcing show...");
      ShowWindow(hwnd, SW_SHOW);
      UpdateWindow(hwnd);
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    } else {
      Logger::Instance().Info("WindowManager", "[OK] Window is visible after Z-order change");
    }
    
    return true;
  } else {
    DWORD error = GetLastError();
    Logger::Instance().Error("WindowManager", 
      "Failed to set Z-order behind SHELLDLL_DefView, error: " + std::to_string(error));
    Logger::Instance().Info("WindowManager", "Attempting HWND_BOTTOM fallback...");
    
    // Try fallback: Place at bottom of Z-order
    result = SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    if (result) {
      Logger::Instance().Info("WindowManager", "Z-order set using HWND_BOTTOM fallback");
      // Force window update after fallback Z-order change
      UpdateWindow(hwnd);
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
      
      // Verify visibility
      BOOL still_visible = IsWindowVisible(hwnd);
      if (!still_visible) {
        Logger::Instance().Warning("WindowManager", 
          "Window invisible after fallback, forcing show...");
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
      }
    } else {
      Logger::Instance().Error("WindowManager", "HWND_BOTTOM fallback also failed!");
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
    
    Logger::Instance().Info("WindowManager", 
      "Using monitor: " + std::string(monitor->device_name) + 
      " [" + std::to_string(out_width) + "x" + std::to_string(out_height) + "]" +
      " at virtual desktop position (" + std::to_string(out_x) + "," + std::to_string(out_y) + ")");
    
    return true;
  } else {
    // Legacy: Get work area (desktop minus taskbar)
    RECT workArea = {0};
    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0)) {
      Logger::Instance().Error("WindowManager", 
        "Failed to get work area, using screen dimensions");
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
    Logger::Instance().Info("WindowManager", "Initializing window as desktop wallpaper...");
    Logger::Instance().Info("WindowManager", 
      "Window handle: " + std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
    Logger::Instance().Info("WindowManager", 
      std::string("Transparent mode: ") + (enable_transparent ? "enabled" : "disabled"));

    // Step 1: Send 0x052C message to Progman to create second WorkerW
    Logger::Instance().Info("WindowManager", "Step 1: Sending 0x052C to Progman...");
    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman) {
      Logger::Instance().Error("WindowManager", "Failed to find Progman window");
      return false;
    }
    Logger::Instance().Info("WindowManager", 
      "Progman found: " + std::to_string(reinterpret_cast<uintptr_t>(progman)));

    // This message triggers Windows to create the second WorkerW
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    Logger::Instance().Info("WindowManager", "Message sent successfully");

    // Step 2: Find the second WorkerW window
    Logger::Instance().Info("WindowManager", "Step 2: Finding second WorkerW...");
    HWND workerw = FindSecondWorkerW();
    if (!workerw) {
      Logger::Instance().Error("WindowManager", "Failed to find second WorkerW window");
      return false;
    }
    Logger::Instance().Info("WindowManager", 
      "Second WorkerW found: " + std::to_string(reinterpret_cast<uintptr_t>(workerw)));

    // Step 3: Set window as child of WorkerW
    Logger::Instance().Info("WindowManager", "Step 3: Embedding window into WorkerW...");
    HWND old_parent = SetParent(hwnd, workerw);
    if (!old_parent && GetLastError() != 0) {
      DWORD error = GetLastError();
      Logger::Instance().Error("WindowManager", 
        "SetParent failed: error " + std::to_string(error));
      return false;
    }
    Logger::Instance().Info("WindowManager", 
      "Window embedded successfully (old parent: " + std::to_string(reinterpret_cast<uintptr_t>(old_parent)) + ")");

    // Step 4: Set window styles
    Logger::Instance().Info("WindowManager", "Step 4: Setting window styles...");
    
    // Base styles: WS_CHILD + WS_VISIBLE + WS_CLIPSIBLINGS + WS_CLIPCHILDREN
    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    style |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    SetWindowLongW(hwnd, GWL_STYLE, style);
    Logger::Instance().Info("WindowManager", 
      "Base styles set: WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN");

    // Extended styles: Always keep WS_EX_NOACTIVATE, conditionally add WS_EX_TRANSPARENT
    LONG ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
    // Remove WS_EX_TRANSPARENT first
    ex_style &= ~WS_EX_TRANSPARENT;
    // Always set WS_EX_NOACTIVATE (prevent focus stealing, avoid interfering with MouseHook)
    ex_style |= WS_EX_NOACTIVATE;
    
    if (enable_transparent) {
      // Transparent mode: prevent focus + mouse pass-through
      ex_style |= WS_EX_TRANSPARENT;
      Logger::Instance().Info("WindowManager", 
        "Extended styles: WS_EX_NOACTIVATE | WS_EX_TRANSPARENT (pass-through mode)");
    } else {
      // Interactive mode: prevent focus (keep WS_EX_NOACTIVATE), but remove WS_EX_TRANSPARENT
      Logger::Instance().Info("WindowManager", 
        "Extended styles: WS_EX_NOACTIVATE (interactive mode - MouseHook injects events)");
    }
    SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style);

    // Step 5: Position window to cover entire screen
    Logger::Instance().Info("WindowManager", "Step 5: Positioning window...");
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    Logger::Instance().Info("WindowManager", 
      "Screen size: " + std::to_string(screen_width) + "x" + std::to_string(screen_height));

    BOOL pos_result = SetWindowPos(
      hwnd,
      nullptr,
      0, 0,
      screen_width, screen_height,
      SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW
    );

    if (!pos_result) {
      DWORD error = GetLastError();
      Logger::Instance().Warning("WindowManager", 
        "SetWindowPos failed with error " + std::to_string(error));
    } else {
      Logger::Instance().Info("WindowManager", "Window positioned successfully");
    }

    // Force window update
    UpdateWindow(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    
    Logger::Instance().Info("WindowManager", "[OK] Wallpaper initialization complete!");
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
      Logger::Instance().Info("WindowManager", "SetInteractive(true): Removing WS_EX_TRANSPARENT");
    } else {
      // Add WS_EX_TRANSPARENT - mouse passes through
      ex_style |= WS_EX_TRANSPARENT;
      Logger::Instance().Info("WindowManager", "SetInteractive(false): Adding WS_EX_TRANSPARENT");
    }

    // Always keep WS_EX_NOACTIVATE to prevent focus stealing
    ex_style |= WS_EX_NOACTIVATE;

    SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style);
    
    // Force window to update its style
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    Logger::Instance().Info("WindowManager", 
      std::string("Interactive mode: ") + (interactive ? "ENABLED" : "DISABLED"));
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
  
  Logger::Instance().Info("WindowManager", "Enumerating WorkerW windows...");
  
  // Enumerate all WorkerW windows
  int count = 0;
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    count++;
    Logger::Instance().Info("WindowManager", 
      "Found WorkerW #" + std::to_string(count) + ": " + std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
    
    // Check if this WorkerW contains SHELLDLL_DefView
    HWND shelldll = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shelldll) {
      Logger::Instance().Info("WindowManager", "This WorkerW contains SHELLDLL_DefView (desktop icons)");
      
      // The WorkerW we want is the NEXT one (sibling)
      workerw = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      if (workerw) {
        Logger::Instance().Info("WindowManager", 
          "Found second WorkerW (wallpaper layer): " + std::to_string(reinterpret_cast<uintptr_t>(workerw)));
        return workerw;
      }
    }
  }
  
  Logger::Instance().Info("WindowManager", "Total WorkerW windows found: " + std::to_string(count));
  Logger::Instance().Warning("WindowManager", "Could not find second WorkerW");
  
  return nullptr;
}

bool WindowManager::DiagnoseWindowVisibility(HWND hwnd, HWND worker_w) {
  if (!hwnd || !IsWindow(hwnd)) {
    Logger::Instance().Error("WindowManager", "[Diagnosis] Window handle is invalid");
    return false;
  }
  
  Logger::Instance().Debug("WindowManager", "========== Window Visibility Diagnosis ==========");
  Logger::Instance().Debug("WindowManager", 
    "[Diagnosis] Window HWND: " + std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
  
  // Check 1: Window validity
  BOOL is_window = IsWindow(hwnd);
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis] IsWindow: ") + (is_window ? "YES" : "NO"));
  if (!is_window) {
    Logger::Instance().Error("WindowManager", "[Diagnosis] [FAIL] Window handle is invalid!");
    return false;
  }
  
  // Check 2: Window visibility
  BOOL is_visible = IsWindowVisible(hwnd);
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis] IsWindowVisible: ") + (is_visible ? "YES" : "NO"));
  
  // Check 3: Window styles
  LONG style = GetWindowLongW(hwnd, GWL_STYLE);
  bool has_visible_style = (style & WS_VISIBLE) != 0;
  bool has_child_style = (style & WS_CHILD) != 0;
  Logger::Instance().Debug("WindowManager", "[Diagnosis] Window styles:");
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis]   - WS_VISIBLE: ") + (has_visible_style ? "YES" : "NO"));
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis]   - WS_CHILD: ") + (has_child_style ? "YES" : "NO"));
  
  // Check 4: Extended styles
  LONG ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
  bool has_transparent = (ex_style & WS_EX_TRANSPARENT) != 0;
  bool has_noactivate = (ex_style & WS_EX_NOACTIVATE) != 0;
  Logger::Instance().Debug("WindowManager", "[Diagnosis] Extended styles:");
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis]   - WS_EX_TRANSPARENT: ") + (has_transparent ? "YES" : "NO"));
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis]   - WS_EX_NOACTIVATE: ") + (has_noactivate ? "YES" : "NO"));
  
  // Check 5: Window position and size
  RECT rect;
  if (GetWindowRect(hwnd, &rect)) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    Logger::Instance().Debug("WindowManager", 
      "[Diagnosis] Window rect: (" + std::to_string(rect.left) + "," + std::to_string(rect.top) + 
      ") " + std::to_string(width) + "x" + std::to_string(height));
    if (width == 0 || height == 0) {
      Logger::Instance().Warning("WindowManager", "[Diagnosis] Window has zero size!");
    }
  } else {
    Logger::Instance().Warning("WindowManager", "[Diagnosis] Failed to get window rect");
  }
  
  // Check 6: Parent window
  HWND parent = GetParent(hwnd);
  Logger::Instance().Debug("WindowManager", 
    "[Diagnosis] Parent window: " + std::to_string(reinterpret_cast<uintptr_t>(parent)) + 
    " (expected: " + std::to_string(reinterpret_cast<uintptr_t>(worker_w)) + ")");
  if (parent != worker_w) {
    Logger::Instance().Warning("WindowManager", "[Diagnosis] Parent window mismatch!");
  }
  
  if (parent && IsWindow(parent)) {
    BOOL parent_visible = IsWindowVisible(parent);
    Logger::Instance().Debug("WindowManager", 
      std::string("[Diagnosis] Parent visible: ") + (parent_visible ? "YES" : "NO"));
    if (!parent_visible) {
      Logger::Instance().Error("WindowManager", "[Diagnosis] [FAIL] Parent window is not visible!");
    }
  } else {
    Logger::Instance().Error("WindowManager", "[Diagnosis] [FAIL] Parent window is invalid!");
  }
  
  // Check 7: Window Z-order (check if window is behind desktop icons)
  HWND shelldll = FindSHELLDLL_DefView(worker_w);
  if (shelldll) {
    Logger::Instance().Debug("WindowManager", 
      "[Diagnosis] SHELLDLL_DefView found: " + std::to_string(reinterpret_cast<uintptr_t>(shelldll)));
    // Check if our window is actually behind shelldll
    HWND window_after = GetWindow(hwnd, GW_HWNDNEXT);
    Logger::Instance().Debug("WindowManager", 
      "[Diagnosis] Window after in Z-order: " + std::to_string(reinterpret_cast<uintptr_t>(window_after)));
  } else {
    Logger::Instance().Warning("WindowManager", "[Diagnosis] SHELLDLL_DefView not found");
  }
  
  // Summary
  bool should_be_visible = is_window && is_visible && has_visible_style && 
                          parent && IsWindow(parent) && IsWindowVisible(parent) &&
                          (rect.right - rect.left) > 0 && (rect.bottom - rect.top) > 0;
  
  Logger::Instance().Debug("WindowManager", "========== Summary ==========");
  Logger::Instance().Debug("WindowManager", 
    std::string("[Diagnosis] Window should be visible: ") + (should_be_visible ? "YES" : "NO"));
  
  if (!should_be_visible) {
    Logger::Instance().Error("WindowManager", "[Diagnosis] [FAIL] Visibility issues detected!");
  } else {
    Logger::Instance().Debug("WindowManager", "[Diagnosis] [OK] Window appears to be properly configured");
  }
  
  return should_be_visible;
}

void WindowManager::EnsureWorkerWZOrder(HWND worker_w) {
  if (!worker_w || !IsWindow(worker_w)) return;

  // Check if we are in Windows 11 Raised Desktop mode
  HWND progman = FindWindowW(L"Progman", nullptr);
  if (!progman) return;

  // Only proceed if worker_w is a child of Progman (Win11 Raised Desktop)
  if (GetParent(worker_w) != progman) return;

  // Check if WorkerW is already at the bottom of Progman's child list
  HWND last_child = GetWindow(progman, GW_CHILD);
  if (last_child) {
    last_child = GetWindow(last_child, GW_HWNDLAST);
  }

  if (last_child != worker_w) {
    Logger::Instance().Info("WindowManager", "WorkerW is not at the bottom, fixing Z-order...");
    
    // Move WorkerW to bottom (behind everything else in Progman, including SHELLDLL_DefView)
    SetWindowPos(worker_w, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                 
    Logger::Instance().Info("WindowManager", "WorkerW moved to HWND_BOTTOM");
  }
}

}  // namespace anywp_engine

