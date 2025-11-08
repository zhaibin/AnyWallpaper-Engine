#include "mouse_hook_manager.h"
#include <iostream>
#include "../anywp_engine_plugin.h"

namespace anywp_engine {

MouseHookManager* MouseHookManager::instance_ = nullptr;

MouseHookManager::MouseHookManager()
    : hook_(nullptr),
      paused_(false) {
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
  
  std::cout << "[AnyWP] [MouseHook] Installing low-level mouse hook..." << std::endl;
  
  hook_ = SetWindowsHookExW(
    WH_MOUSE_LL,
    LowLevelMouseProc,
    GetModuleHandle(nullptr),
    0
  );
  
  if (hook_) {
    std::cout << "[AnyWP] [MouseHook] Hook installed successfully" << std::endl;
    return true;
  } else {
    std::cout << "[AnyWP] [MouseHook] Failed to install hook: " << GetLastError() << std::endl;
    return false;
  }
}

void MouseHookManager::Uninstall() {
  if (hook_) {
    std::cout << "[AnyWP] [MouseHook] Uninstalling mouse hook..." << std::endl;
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

void MouseHookManager::SetPaused(bool paused) {
  paused_ = paused;
}

bool MouseHookManager::IsPaused() const {
  return paused_;
}

void MouseHookManager::CancelActiveDrag(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  if (!webview) {
    return;
  }
  
  std::wstring cancel_script = LR"(
    (function() {
      if (window.AnyWP && window.AnyWP.cancelActiveDrag) {
        window.AnyWP.cancelActiveDrag();
      }
    })();
  )";
  
  webview->ExecuteScript(cancel_script.c_str(), nullptr);
}

LRESULT CALLBACK MouseHookManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
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
  
  // Get window class name for debugging
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
      
      // Skip desktop-related windows
      bool is_desktop_window = (
        wcscmp(rootClassName, L"Progman") == 0 ||
        wcscmp(rootClassName, L"WorkerW") == 0 ||
        wcscmp(rootClassName, L"Shell_TrayWnd") == 0 ||  // Taskbar
        wcsstr(rootClassName, L"Xaml") != nullptr  // System UI
      );
      
      // Check if it's OUR WebView2 window (via callback)
      bool is_our_webview = false;
      if (wcsstr(rootClassName, L"Chrome") != nullptr || wcsstr(className, L"Chrome") != nullptr) {
        // Use instance callback to check if this is our window
        if (instance_->instance_callback_) {
          WallpaperInstance* inst = instance_->instance_callback_(pt.x, pt.y);
          is_our_webview = (inst != nullptr);
        }
      }
      
      if (!is_desktop_window && !is_our_webview) {
        // Check if it's a normal app window (has caption or is popup)
        bool has_window_style = (style & WS_CAPTION) || (style & WS_POPUP);
        
        if (has_window_style) {
          is_app_window = true;
        }
      }
    }
  }
  
  // If occluded by app window, don't forward
  if (is_app_window) {
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
      std::cout << "[AnyWP] [MouseHook] Click on iframe: " << iframe->id 
                << " at (" << pt.x << "," << pt.y << ")" << std::endl;
      std::cout << "[AnyWP] [MouseHook] Opening URL: " << iframe->click_url << std::endl;
      
      // Open the ad URL directly
      std::wstring url_wide(iframe->click_url.begin(), iframe->click_url.end());
      ShellExecuteW(nullptr, L"open", url_wide.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      
      // Don't forward to WebView
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
  }
  
  // Determine event type
  const char* event_type = nullptr;
  
  if (wParam == WM_LBUTTONDOWN) {
    event_type = "mousedown";
  } else if (wParam == WM_LBUTTONUP) {
    event_type = "mouseup";
  } else if (wParam == WM_MOUSEMOVE) {
    event_type = "mousemove";
  }
  
  // Forward to WebView via callback
  if (event_type && instance_->click_callback_) {
    instance_->click_callback_(pt.x, pt.y, event_type);
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

}  // namespace anywp_engine
