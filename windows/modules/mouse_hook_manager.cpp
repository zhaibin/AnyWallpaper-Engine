#include "mouse_hook_manager.h"
#include <iostream>

namespace anywp_engine {

MouseHookManager* MouseHookManager::instance_ = nullptr;

MouseHookManager::MouseHookManager()
    : hook_(nullptr), paused_(false) {
  instance_ = this;
}

MouseHookManager::~MouseHookManager() {
  Uninstall();
  instance_ = nullptr;
}

bool MouseHookManager::Install() {
  if (hook_) {
    std::cout << "[MouseHookManager] Hook already installed" << std::endl;
    return true;
  }
  
  hook_ = SetWindowsHookExW(
    WH_MOUSE_LL,
    LowLevelMouseProc,
    GetModuleHandleW(nullptr),
    0
  );
  
  if (hook_) {
    std::cout << "[MouseHookManager] Hook installed successfully" << std::endl;
    return true;
  } else {
    std::cout << "[MouseHookManager] ERROR: Failed to install hook" << std::endl;
    return false;
  }
}

void MouseHookManager::Uninstall() {
  if (hook_) {
    UnhookWindowsHookEx(hook_);
    hook_ = nullptr;
    std::cout << "[MouseHookManager] Hook removed" << std::endl;
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
  if (webview) {
    std::wstring script = L"if (window.AnyWP && window.AnyWP._dragState) { "
                          L"window.AnyWP._dragState = null; }";
    webview->ExecuteScript(script.c_str(), nullptr);
  }
}

LRESULT CALLBACK MouseHookManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0 && instance_ && !instance_->paused_) {
    MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    POINT pt = info->pt;
    
    // Forward click to callback
    if (instance_->click_callback_) {
      const char* event_type = nullptr;
      if (wParam == WM_LBUTTONDOWN) event_type = "mousedown";
      else if (wParam == WM_LBUTTONUP) event_type = "mouseup";
      else if (wParam == WM_MOUSEMOVE) event_type = "mousemove";
      
      if (event_type) {
        instance_->click_callback_(pt.x, pt.y, event_type);
      }
    }
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

}  // namespace anywp_engine

