#include "keyboard_hook_manager.h"
#include <iostream>
#include "../utils/logger.h"

namespace anywp_engine {

KeyboardHookManager* KeyboardHookManager::instance_ = nullptr;

KeyboardHookManager::KeyboardHookManager()
    : hook_(nullptr),
      paused_(false) {
  instance_ = this;
}

KeyboardHookManager::~KeyboardHookManager() {
  Uninstall();
  instance_ = nullptr;
}

bool KeyboardHookManager::Install() {
  if (hook_) {
    return true;  // Already installed
  }
  
  try {
    Logger::Instance().Info("KeyboardHook", "Installing low-level keyboard hook...");
    
    hook_ = SetWindowsHookExW(
      WH_KEYBOARD_LL,
      LowLevelKeyboardProc,
      GetModuleHandle(nullptr),
      0
    );
    
    if (hook_) {
      Logger::Instance().Info("KeyboardHook", "Hook installed successfully");
      return true;
    } else {
      DWORD error = GetLastError();
      Logger::Instance().Error("KeyboardHook", "Failed to install hook: " + std::to_string(error));
      return false;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("KeyboardHook", std::string("Exception in Install: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("KeyboardHook", "Unknown exception in Install");
    return false;
  }
}

void KeyboardHookManager::Uninstall() {
  if (hook_) {
    Logger::Instance().Info("KeyboardHook", "Uninstalling keyboard hook...");
    UnhookWindowsHookEx(hook_);
    hook_ = nullptr;
  }
}

bool KeyboardHookManager::IsInstalled() const {
  return hook_ != nullptr;
}

void KeyboardHookManager::SetKeyboardCallback(KeyboardCallback callback) {
  keyboard_callback_ = callback;
}

void KeyboardHookManager::SetPaused(bool paused) {
  paused_ = paused;
}

bool KeyboardHookManager::IsPaused() const {
  return paused_;
}

LRESULT CALLBACK KeyboardHookManager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode < 0 || !instance_) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  // Skip if paused (performance optimization)
  if (instance_->paused_) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  KBDLLHOOKSTRUCT* info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
  
  // Determine event type
  const char* event_type = nullptr;
  
  if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
    event_type = "keydown";
  } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
    event_type = "keyup";
  }
  
  // Get modifier key states
  bool alt_down = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
  bool ctrl_down = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
  bool shift_down = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
  
  // Forward to callback if registered
  if (event_type && instance_->keyboard_callback_) {
    bool extended = (info->flags & LLKHF_EXTENDED) != 0;
    
    // CRITICAL: Wrap callback in try-catch to prevent application crashes
    // Keyboard hook runs in system thread - unhandled exceptions will crash the app
    try {
      instance_->keyboard_callback_(
        event_type,
        info->vkCode,
        info->scanCode,
        extended,
        alt_down,
        ctrl_down,
        shift_down
      );
    } catch (const std::exception& e) {
      // Log error but continue - don't let callback exceptions crash the hook
      Logger::Instance().Error("KeyboardHook", std::string("Exception in keyboard callback: ") + e.what());
    } catch (...) {
      Logger::Instance().Error("KeyboardHook", "Unknown exception in keyboard callback");
    }
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

}  // namespace anywp_engine

