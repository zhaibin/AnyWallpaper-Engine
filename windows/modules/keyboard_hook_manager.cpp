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
    
    // Get DLL module handle (CRITICAL: must be the DLL containing the hook procedure, not the EXE)
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&LowLevelKeyboardProc),
        &hModule)) {
      DWORD error = GetLastError();
      Logger::Instance().Error("KeyboardHook", 
        "Failed to get DLL module handle: " + std::to_string(error));
      return false;
    }
    
    Logger::Instance().Debug("KeyboardHook", 
      "DLL module handle: " + std::to_string(reinterpret_cast<uintptr_t>(hModule)));
    
    hook_ = SetWindowsHookExW(
      WH_KEYBOARD_LL,
      LowLevelKeyboardProc,
      hModule,  // Use DLL handle, not EXE handle
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
  // DEBUG: Log first few calls to verify hook is being triggered
  static int hook_call_count = 0;
  hook_call_count++;
  if (hook_call_count <= 5) {
    Logger::Instance().Debug("KeyboardHook", 
      "Hook called #" + std::to_string(hook_call_count) + 
      " nCode=" + std::to_string(nCode) + 
      " wParam=" + std::to_string(wParam));
  }
  
  if (nCode < 0 || !instance_) {
    if (hook_call_count <= 5 && nCode < 0) {
      Logger::Instance().Debug("KeyboardHook", "Skipping: nCode < 0");
    }
    if (hook_call_count <= 5 && !instance_) {
      Logger::Instance().Error("KeyboardHook", "FATAL: instance_ is NULL!");
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }
  
  // Skip if paused (performance optimization)
  if (instance_->paused_) {
    if (hook_call_count <= 5) {
      Logger::Instance().Debug("KeyboardHook", "Skipping: paused");
    }
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
    
    if (hook_call_count <= 5) {
      Logger::Instance().Debug("KeyboardHook", 
        "Calling callback: " + std::string(event_type) + 
        " vk=" + std::to_string(info->vkCode));
    }
    
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
  } else if (hook_call_count <= 5) {
    if (!event_type) {
      Logger::Instance().Debug("KeyboardHook", "No event_type (wParam=" + std::to_string(wParam) + ")");
    }
    if (!instance_->keyboard_callback_) {
      Logger::Instance().Error("KeyboardHook", "FATAL: keyboard_callback_ is NULL!");
    }
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

}  // namespace anywp_engine

