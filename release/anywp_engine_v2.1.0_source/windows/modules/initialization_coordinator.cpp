#include "initialization_coordinator.h"
#include "../utils/logger.h"
#include "../utils/desktop_wallpaper_helper.h"
#include "../utils/url_validator.h"
#include "window_manager.h"
#include "webview_manager.h"
#include <iostream>

namespace anywp_engine {

InitializationCoordinator::InitializationCoordinator()
    : window_manager_(nullptr),
      webview_manager_(nullptr),
      url_validator_(nullptr),
      retry_count_(0) {
  Logger::Instance().Info("InitCoordinator", "Initialization coordinator created");
}

InitializationCoordinator::~InitializationCoordinator() {
  Logger::Instance().Info("InitCoordinator", "Initialization coordinator destroyed");
}

void InitializationCoordinator::SetWindowManager(WindowManager* window_manager) {
  window_manager_ = window_manager;
  Logger::Instance().Info("InitCoordinator", "WindowManager dependency set");
}

void InitializationCoordinator::SetWebViewManager(WebViewManager* webview_manager) {
  webview_manager_ = webview_manager;
  Logger::Instance().Info("InitCoordinator", "WebViewManager dependency set");
}

void InitializationCoordinator::SetURLValidator(URLValidator* url_validator) {
  url_validator_ = url_validator;
  Logger::Instance().Info("InitCoordinator", "URLValidator dependency set");
}

InitializationCoordinator::InitResult 
InitializationCoordinator::Initialize(const InitConfig& config) {
  InitResult result = {false, nullptr, nullptr, ""};
  
  // Log initialization start
  LogInitializationStart(config);
  
  // Step 1: Validate URL
  if (!ValidateURL(config.url)) {
    result.error_message = "URL validation failed: " + config.url;
    LogInitializationFailure(result);
    return result;
  }
  
  // Step 2: Find WorkerW window
  if (!FindWallpaperLayer(result.worker_w_window)) {
    result.error_message = "Failed to find WorkerW window";
    LogInitializationFailure(result);
    return result;
  }
  
  // Step 3: Create host window
  if (!CreateHostWindow(config, result.worker_w_window, result.host_window)) {
    result.error_message = "Failed to create host window";
    LogInitializationFailure(result);
    return result;
  }
  
  // Step 4: Setup window Z-order
  if (!SetupWindowZOrder(result.host_window, result.worker_w_window)) {
    result.error_message = "Failed to setup window Z-order";
    LogInitializationFailure(result);
    return result;
  }
  
  // Step 5: Configure mouse hook
  ConfigureMouseHook(config.enable_mouse_transparent);
  
  // Success
  result.success = true;
  retry_count_ = 0;  // Reset retry counter on success
  LogInitializationSuccess(result);
  
  return result;
}

bool InitializationCoordinator::ValidateURL(const std::string& url) {
  if (!url_validator_) {
    Logger::Instance().Error("InitCoordinator", "URLValidator not set");
    return false;
  }
  
  bool is_valid = url_validator_->IsAllowed(url);
  
  if (!is_valid) {
    Logger::Instance().Warning("InitCoordinator", 
      "URL validation failed: " + url);
    LogError("URL validation failed: " + url);
  } else {
    Logger::Instance().Info("InitCoordinator", 
      "URL validated successfully: " + url);
  }
  
  return is_valid;
}

bool InitializationCoordinator::FindWallpaperLayer(HWND& out_worker_w) {
  Logger::Instance().Info("InitCoordinator", "Finding WorkerW window...");
  
  // Use DesktopWallpaperHelper to find WorkerW
  if (!DesktopWallpaperHelper::Instance().FindWorkerW()) {
    Logger::Instance().Error("InitCoordinator", "Failed to find WorkerW window");
    LogError("Failed to find WorkerW window");
    return false;
  }
  
  out_worker_w = DesktopWallpaperHelper::Instance().GetWallpaperParent();
  if (!out_worker_w) {
    Logger::Instance().Error("InitCoordinator", "Failed to get wallpaper parent window");
    LogError("Failed to get wallpaper parent window");
    return false;
  }
  
  // Validate window handle
  if (!ValidateWindowHandle(out_worker_w)) {
    Logger::Instance().Error("InitCoordinator", "Wallpaper layer window is invalid");
    LogError("Invalid wallpaper layer window");
    return false;
  }
  
  Logger::Instance().Info("InitCoordinator", 
    "WorkerW window found: " + std::to_string(reinterpret_cast<uintptr_t>(out_worker_w)));
  
  return true;
}

bool InitializationCoordinator::CreateHostWindow(
    const InitConfig& config, HWND worker_w, HWND& out_hwnd) {
  
  if (!window_manager_) {
    Logger::Instance().Error("InitCoordinator", "WindowManager not set");
    return false;
  }
  
  Logger::Instance().Info("InitCoordinator", "Creating host window...");
  
  // Create window using WindowManager
  out_hwnd = window_manager_->CreateWebViewHostWindow(
      config.enable_mouse_transparent, 
      config.monitor, 
      worker_w);
  
  if (!out_hwnd) {
    Logger::Instance().Error("InitCoordinator", "Failed to create WebView host window");
    LogError("Failed to create WebView host window");
    return false;
  }
  
  Logger::Instance().Info("InitCoordinator", 
    "Host window created: " + std::to_string(reinterpret_cast<uintptr_t>(out_hwnd)));
  
  // Log monitor information if available
  if (config.monitor) {
    Logger::Instance().Info("InitCoordinator", 
      "Monitor: " + config.monitor->device_name + 
      " [" + std::to_string(config.monitor->width) + "x" + 
      std::to_string(config.monitor->height) + "]");
  }
  
  return true;
}

bool InitializationCoordinator::SetupWindowZOrder(HWND host_window, HWND worker_w) {
  if (!window_manager_) {
    Logger::Instance().Error("InitCoordinator", "WindowManager not set");
    return false;
  }
  
  Logger::Instance().Info("InitCoordinator", "Setting up window Z-order...");
  
  bool success = window_manager_->SetWallpaperZOrder(host_window, worker_w);
  
  if (!success) {
    Logger::Instance().Warning("InitCoordinator", "Failed to setup window Z-order");
    // Not a fatal error, continue
  } else {
    Logger::Instance().Info("InitCoordinator", "Window Z-order configured");
  }
  
  return true;  // Always return true (non-fatal)
}

void InitializationCoordinator::ConfigureMouseHook(bool enable_mouse_transparent) {
  Logger::Instance().Info("InitCoordinator", 
    "Configuring mouse hook for mode: " + 
    std::string(enable_mouse_transparent ? "TRANSPARENT" : "INTERACTIVE"));
  
  // Log explanation
  if (enable_mouse_transparent) {
    std::cout << "[AnyWP] [InitCoordinator] Mode: TRANSPARENT (pass-through)" << std::endl;
    std::cout << "[AnyWP] [InitCoordinator] MouseHook will forward events (WebView cannot receive focus)" << std::endl;
  } else {
    std::cout << "[AnyWP] [InitCoordinator] Mode: INTERACTIVE (clickable)" << std::endl;
    std::cout << "[AnyWP] [InitCoordinator] MouseHook will inject events into WebView via JavaScript" << std::endl;
  }
  
  // Note: Actual mouse hook setup is handled by MouseHookManager in the main plugin
  // This method just logs the configuration intent
}

void InitializationCoordinator::LogError(const std::string& error) {
  Logger::Instance().Error("InitCoordinator", error);
  
  // Also log to stdout for backward compatibility
  std::cout << "[AnyWP] [InitCoordinator] ERROR: " << error << std::endl;
}

bool InitializationCoordinator::ValidateWindowHandle(HWND hwnd) {
  return (hwnd != nullptr && IsWindow(hwnd));
}

void InitializationCoordinator::LogInitializationStart(const InitConfig& config) {
  Logger::Instance().Info("InitCoordinator", 
    "Starting initialization - URL: " + config.url + 
    ", Transparent: " + (config.enable_mouse_transparent ? "true" : "false") +
    ", Monitor: " + std::to_string(config.monitor_index));
  
  std::cout << "[AnyWP] [InitCoordinator] ========== Initialization Start ==========" << std::endl;
  std::cout << "[AnyWP] [InitCoordinator] URL: " << config.url << std::endl;
  std::cout << "[AnyWP] [InitCoordinator] Transparent: " << (config.enable_mouse_transparent ? "true" : "false") << std::endl;
  
  if (config.monitor) {
    std::cout << "[AnyWP] [InitCoordinator] Target Monitor: " << config.monitor->device_name 
              << " [" << config.monitor->width << "x" << config.monitor->height << "]" << std::endl;
  } else {
    std::cout << "[AnyWP] [InitCoordinator] Mode: Single-monitor (legacy)" << std::endl;
  }
}

void InitializationCoordinator::LogInitializationSuccess(const InitResult& result) {
  Logger::Instance().Info("InitCoordinator", "Initialization completed successfully");
  
  std::cout << "[AnyWP] [InitCoordinator] ========== Initialization Success ==========" << std::endl;
  std::cout << "[AnyWP] [InitCoordinator] Host Window: " << result.host_window << std::endl;
  std::cout << "[AnyWP] [InitCoordinator] WorkerW Window: " << result.worker_w_window << std::endl;
}

void InitializationCoordinator::LogInitializationFailure(const InitResult& result) {
  Logger::Instance().Error("InitCoordinator", "Initialization failed: " + result.error_message);
  
  std::cout << "[AnyWP] [InitCoordinator] ========== Initialization Failed ==========" << std::endl;
  std::cout << "[AnyWP] [InitCoordinator] Error: " << result.error_message << std::endl;
}

}  // namespace anywp_engine

