#include "mouse_hook_manager.h"
#include "../anywp_engine_plugin.h"
#include "../utils/logger.h"

namespace anywp_engine {

MouseHookManager* MouseHookManager::instance_ = nullptr;

MouseHookManager::MouseHookManager()
    : hook_(nullptr),
      paused_(false),
      is_mouse_down_(false) {
  instance_ = this;
}

MouseHookManager::~MouseHookManager() {
  Uninstall();
  instance_ = nullptr;
}

bool MouseHookManager::Install() {
  if (hook_) {
    return true;  // Already installed
  }
  
  try {
    Logger::Instance().Info("MouseHook", "Installing low-level mouse hook...");
    
    hook_ = SetWindowsHookExW(
      WH_MOUSE_LL,
      LowLevelMouseProc,
      GetModuleHandle(nullptr),
      0
    );
    
    if (hook_) {
      Logger::Instance().Info("MouseHook", "Hook installed successfully");
      return true;
    } else {
      DWORD error = GetLastError();
      Logger::Instance().Error("MouseHook", "Failed to install hook: " + std::to_string(error));
      return false;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("MouseHook", "Exception in Install: " + std::string(e.what()));
    return false;
  } catch (...) {
    Logger::Instance().Error("MouseHook", "Unknown exception in Install");
    return false;
  }
}

void MouseHookManager::Uninstall() {
  if (hook_) {
    Logger::Instance().Info("MouseHook", "Uninstalling mouse hook...");
    UnhookWindowsHookEx(hook_);
    hook_ = nullptr;
  }
}

bool MouseHookManager::IsInstalled() const {
  return hook_ != nullptr;
}

void MouseHookManager::SetClickCallback(ClickCallback callback) {
  click_callback_ = callback;
}

void MouseHookManager::SetIframeCallback(IframeCallback callback) {
  iframe_callback_ = callback;
}

void MouseHookManager::SetInstanceCallback(InstanceCallback callback) {
  instance_callback_ = callback;
}

void MouseHookManager::SetHwndCheckCallback(HwndCheckCallback callback) {
  hwnd_check_callback_ = callback;
}

void MouseHookManager::SetPaused(bool paused) {
  paused_ = paused;
}

bool MouseHookManager::IsPaused() const {
  return paused_;
}


LRESULT CALLBACK MouseHookManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  // v2.3.2+: Removed debug counters to reduce log noise
  
  if (nCode < 0 || !instance_) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  // Skip if paused (performance optimization)
  if (instance_->paused_) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
  POINT pt = info->pt;
  
  // Check if click position is occluded by a top-level application window
  HWND window_at_point = WindowFromPoint(pt);
  
  // Get window class name
  wchar_t className[256] = {0};
  if (window_at_point) {
    GetClassNameW(window_at_point, className, 256);
  }
  
  // Check if this is a top-level application window (not desktop layer)
  bool is_app_window = false;
  if (window_at_point) {
    // Get the root owner window
    HWND root_window = GetAncestor(window_at_point, GA_ROOT);
    
    // Check if it's a visible top-level window
    if (root_window && IsWindowVisible(root_window)) {
      LONG style = GetWindowLongW(root_window, GWL_STYLE);
      
      // Get root window class name
      wchar_t rootClassName[256] = {0};
      GetClassNameW(root_window, rootClassName, 256);
      
      // v2.0.10+ CRITICAL FIX: Detect desktop AND desktop icon list
      // SysListView32 is the icon container, we need to forward these events too
      bool is_desktop_window = (
        wcscmp(rootClassName, L"Progman") == 0 ||
        wcscmp(rootClassName, L"WorkerW") == 0 ||
        wcscmp(rootClassName, L"Shell_TrayWnd") == 0 ||  // Taskbar
        wcsstr(rootClassName, L"Xaml") != nullptr ||  // System UI
        wcscmp(className, L"SysListView32") == 0  // Desktop icons (child of Progman)
      );
      
      // Check if it's OUR wallpaper window (via HWND callback)
      bool is_our_window = false;
      if (instance_->hwnd_check_callback_) {
        // Check both the window at point and its root
        bool check_window = instance_->hwnd_check_callback_(window_at_point);
        bool check_root = instance_->hwnd_check_callback_(root_window);
        is_our_window = check_window || check_root;
      }
      
      // Fallback: Also check if it's our Chrome WebView2 window
      if (!is_our_window && (wcsstr(rootClassName, L"Chrome") != nullptr || wcsstr(className, L"Chrome") != nullptr)) {
        if (instance_->instance_callback_) {
          WallpaperInstance* inst = instance_->instance_callback_(pt.x, pt.y);
          is_our_window = (inst != nullptr);
        }
      }
      
      if (!is_desktop_window && !is_our_window) {
        // Check if it's a normal app window (has caption or is popup)
        bool has_window_style = (style & WS_CAPTION) || (style & WS_POPUP);
        
        if (has_window_style) {
          is_app_window = true;
        }
      }
    }
  }
  
  // v2.0.4+ Set mouse button state BEFORE checking is_app_window
  // Otherwise mousedown on app window will return early and never set is_mouse_down_
  
  // Determine event type and update mouse button state FIRST
  const char* event_type = nullptr;
  
  if (wParam == WM_LBUTTONDOWN) {
    event_type = "mousedown";
    // v2.0.4+ Track mouse button down state - MUST set this before any early returns
    instance_->is_mouse_down_ = true;
  } else if (wParam == WM_LBUTTONUP) {
    event_type = "mouseup";
    // v2.0.4+ Clear mouse button down state
    instance_->is_mouse_down_ = false;
  } else if (wParam == WM_MOUSEMOVE) {
    event_type = "mousemove";
  }
  
  // v2.0.4+ NOW check is_app_window (after mouse button state is set)
  // Don't block events when mouse button is pressed
  if (is_app_window && !instance_->is_mouse_down_) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  // Get target wallpaper instance (via callback)
  WallpaperInstance* target_instance = nullptr;
  if (instance_->instance_callback_) {
    target_instance = instance_->instance_callback_(pt.x, pt.y);
  }
  
  // Check if click is on an iframe ad (via callback)
  if (wParam == WM_LBUTTONUP && target_instance && instance_->iframe_callback_) {
    IframeInfo* iframe = instance_->iframe_callback_(pt.x, pt.y, target_instance);
    
    if (iframe && !iframe->click_url.empty()) {
      Logger::Instance().Info("MouseHook", 
        "Click on iframe: " + iframe->id + " at (" + std::to_string(pt.x) + "," + std::to_string(pt.y) + ")");
      Logger::Instance().Info("MouseHook", "Opening URL: " + iframe->click_url);
      
      // Open the ad URL directly
      std::wstring url_wide(iframe->click_url.begin(), iframe->click_url.end());
      ShellExecuteW(nullptr, L"open", url_wide.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      
      // Don't forward to WebView
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
  }
  
  // Forward to WebView via callback
  if (event_type && instance_->click_callback_) {
    instance_->click_callback_(pt.x, pt.y, event_type);
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

}  // namespace anywp_engine
