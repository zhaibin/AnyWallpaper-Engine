#include "webview_configurator.h"
#include "../utils/logger.h"
#include <iostream>
#include <sstream>

namespace anywp_engine {

WebViewConfigurator::WebViewConfigurator() {
  Logger::Instance().Info("WebViewConfig", "WebView configurator created");
  
  // Default configuration
  current_options_.enable_console_logging = true;
  current_options_.enable_permission_filtering = true;
  current_options_.enable_security_handlers = true;
  current_options_.inject_sdk = true;
}

WebViewConfigurator::~WebViewConfigurator() {
  Logger::Instance().Info("WebViewConfig", "WebView configurator destroyed");
}

bool WebViewConfigurator::ConfigureWebView(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    const ConfigOptions& options) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  current_options_ = options;
  Logger::Instance().Info("WebViewConfig", "Starting WebView configuration...");
  
  try {
    // Setup permissions
    if (options.enable_permission_filtering) {
      if (!SetupPermissions(webview)) {
        Logger::Instance().Warning("WebViewConfig", "Failed to setup permissions");
      }
    }
    
    // Setup security handlers
    if (options.enable_security_handlers) {
      // Note: Security handlers require URL validator callback
      // This will be set up separately by the caller
      Logger::Instance().Info("WebViewConfig", "Security handlers will be configured separately");
    }
    
    // Setup console listener
    if (options.enable_console_logging) {
      if (!SetupConsoleListener(webview, "AnyWP")) {
        Logger::Instance().Warning("WebViewConfig", "Failed to setup console listener");
      }
    }
    
    Logger::Instance().Info("WebViewConfig", "WebView configuration completed");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebViewConfig", 
      std::string("Exception during configuration: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("WebViewConfig", "Unknown exception during configuration");
    return false;
  }
}

bool WebViewConfigurator::SetupPermissions(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  Logger::Instance().Info("WebViewConfig", "Setting up permission handlers...");
  
  try {
    HRESULT hr = webview->add_PermissionRequested(
      Microsoft::WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
        [this](ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
          try {
            COREWEBVIEW2_PERMISSION_KIND kind;
            args->get_PermissionKind(&kind);
            
            // Deny dangerous permissions by default
            switch (kind) {
              case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
              case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
              case COREWEBVIEW2_PERMISSION_KIND_GEOLOCATION:
              case COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
                args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
                LogPermissionDenied(kind);
                break;
              default:
                args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
            }
            
            return S_OK;
          } catch (...) {
            Logger::Instance().Error("WebViewConfig", "Exception in permission callback");
            return E_FAIL;
          }
        }).Get(), nullptr);
    
    if (FAILED(hr)) {
      Logger::Instance().Error("WebViewConfig", 
        "Failed to add permission handler: HRESULT=" + std::to_string(hr));
      return false;
    }
    
    Logger::Instance().Info("WebViewConfig", "Permission handlers configured");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebViewConfig", 
      std::string("Exception in SetupPermissions: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("WebViewConfig", "Unknown exception in SetupPermissions");
    return false;
  }
}

bool WebViewConfigurator::SetupSecurityHandlers(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    std::function<bool(const std::string&)> url_validator) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  if (!url_validator) {
    Logger::Instance().Error("WebViewConfig", "URL validator callback not provided");
    return false;
  }
  
  Logger::Instance().Info("WebViewConfig", "Setting up security handlers...");
  
  try {
    HRESULT hr = webview->add_NavigationStarting(
      Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
        [this, url_validator](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
          try {
            LPWSTR uri;
            args->get_Uri(&uri);
            
            if (!uri) {
              return E_FAIL;
            }
            
            // Convert wide string to UTF-8 (proper conversion)
            std::wstring wuri(uri);
            
            // Use WideCharToMultiByte for proper UTF-8 conversion
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wuri.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string url(size_needed - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wuri.c_str(), -1, &url[0], size_needed, nullptr, nullptr);
            
            // Validate URL
            if (!url_validator(url)) {
              args->put_Cancel(TRUE);
              LogNavigationBlocked(url);
            } else {
              Logger::Instance().Info("WebViewConfig", "Navigation allowed: " + url);
            }
            
            CoTaskMemFree(uri);
            return S_OK;
            
          } catch (...) {
            Logger::Instance().Error("WebViewConfig", "Exception in navigation callback");
            return E_FAIL;
          }
        }).Get(), nullptr);
    
    if (FAILED(hr)) {
      Logger::Instance().Error("WebViewConfig", 
        "Failed to add navigation handler: HRESULT=" + std::to_string(hr));
      return false;
    }
    
    Logger::Instance().Info("WebViewConfig", "Security handlers configured");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebViewConfig", 
      std::string("Exception in SetupSecurityHandlers: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("WebViewConfig", "Unknown exception in SetupSecurityHandlers");
    return false;
  }
}

bool WebViewConfigurator::SetupNavigationHandlers(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    std::function<void(bool success)> on_navigation_completed) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  if (!on_navigation_completed) {
    Logger::Instance().Error("WebViewConfig", "Navigation callback not provided");
    return false;
  }
  
  Logger::Instance().Info("WebViewConfig", "Setting up navigation handlers...");
  
  try {
    HRESULT hr = webview->add_NavigationCompleted(
      Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
        [this, on_navigation_completed](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
          try {
            BOOL success;
            args->get_IsSuccess(&success);
            
            if (!success) {
              COREWEBVIEW2_WEB_ERROR_STATUS status;
              args->get_WebErrorStatus(&status);
              Logger::Instance().Error("WebViewConfig", 
                "Navigation failed with status: " + std::to_string(status));
            } else {
              Logger::Instance().Info("WebViewConfig", "Navigation completed successfully");
            }
            
            on_navigation_completed(success != FALSE);
            return S_OK;
            
          } catch (...) {
            Logger::Instance().Error("WebViewConfig", "Exception in navigation completed callback");
            return E_FAIL;
          }
        }).Get(), nullptr);
    
    if (FAILED(hr)) {
      Logger::Instance().Error("WebViewConfig", 
        "Failed to add navigation completed handler: HRESULT=" + std::to_string(hr));
      return false;
    }
    
    Logger::Instance().Info("WebViewConfig", "Navigation handlers configured");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebViewConfig", 
      std::string("Exception in SetupNavigationHandlers: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("WebViewConfig", "Unknown exception in SetupNavigationHandlers");
    return false;
  }
}

bool WebViewConfigurator::SetupConsoleListener(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    const std::string& filter_pattern) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  Logger::Instance().Info("WebViewConfig", "Setting up console listener...");
  
  try {
    // Enable DevTools Protocol Runtime domain
    webview->CallDevToolsProtocolMethod(L"Runtime.enable", L"{}", nullptr);
    
    // Get event receiver for console API calls
    Microsoft::WRL::ComPtr<ICoreWebView2DevToolsProtocolEventReceiver> console_receiver;
    HRESULT hr = webview->GetDevToolsProtocolEventReceiver(L"Runtime.consoleAPICalled", &console_receiver);
    
    if (FAILED(hr) || !console_receiver) {
      Logger::Instance().Warning("WebViewConfig", "Failed to get console event receiver");
      return false;
    }
    
    hr = console_receiver->add_DevToolsProtocolEventReceived(
      Microsoft::WRL::Callback<ICoreWebView2DevToolsProtocolEventReceivedEventHandler>(
        [this, filter_pattern](ICoreWebView2* sender, ICoreWebView2DevToolsProtocolEventReceivedEventArgs* args) -> HRESULT {
          try {
            LPWSTR json_ptr = nullptr;
            args->get_ParameterObjectAsJson(&json_ptr);
            
            if (json_ptr) {
              std::wstring json_str(json_ptr);
              CoTaskMemFree(json_ptr);
              
              // Convert to UTF-8
              int size_needed = WideCharToMultiByte(CP_UTF8, 0, json_str.c_str(), -1, NULL, 0, NULL, NULL);
              std::string json_utf8(size_needed, 0);
              WideCharToMultiByte(CP_UTF8, 0, json_str.c_str(), -1, &json_utf8[0], size_needed, NULL, NULL);
              
              // Apply filter if provided
              if (filter_pattern.empty() || json_utf8.find(filter_pattern) != std::string::npos) {
                LogConsoleMessage(json_utf8);
              }
            }
            
            return S_OK;
            
          } catch (...) {
            Logger::Instance().Error("WebViewConfig", "Exception in console callback");
            return E_FAIL;
          }
        }).Get(), nullptr);
    
    if (FAILED(hr)) {
      Logger::Instance().Warning("WebViewConfig", "Failed to add console event handler");
      return false;
    }
    
    Logger::Instance().Info("WebViewConfig", "Console listener configured");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebViewConfig", 
      std::string("Exception in SetupConsoleListener: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("WebViewConfig", "Unknown exception in SetupConsoleListener");
    return false;
  }
}

bool WebViewConfigurator::InjectSDK(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    const std::string& sdk_script) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  if (sdk_script.empty()) {
    Logger::Instance().Error("WebViewConfig", "SDK script is empty");
    return false;
  }
  
  Logger::Instance().Info("WebViewConfig", 
    "Injecting SDK script (" + std::to_string(sdk_script.length()) + " bytes)...");
  
  // Convert to wide string
  std::wstring wscript(sdk_script.begin(), sdk_script.end());
  
  return ExecuteScript(webview, wscript);
}

bool WebViewConfigurator::ExecuteScript(
    Microsoft::WRL::ComPtr<ICoreWebView2> webview,
    const std::wstring& script) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewConfig", "Invalid WebView2 instance");
    return false;
  }
  
  try {
    HRESULT hr = webview->ExecuteScript(script.c_str(), nullptr);
    
    if (FAILED(hr)) {
      Logger::Instance().Error("WebViewConfig", 
        "Failed to execute script: HRESULT=" + std::to_string(hr));
      return false;
    }
    
    Logger::Instance().Info("WebViewConfig", "Script executed successfully");
    return true;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebViewConfig", 
      std::string("Exception in ExecuteScript: ") + e.what());
    return false;
  } catch (...) {
    Logger::Instance().Error("WebViewConfig", "Unknown exception in ExecuteScript");
    return false;
  }
}

void WebViewConfigurator::LogPermissionDenied(COREWEBVIEW2_PERMISSION_KIND kind) {
  std::string permission_name;
  switch (kind) {
    case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
      permission_name = "Microphone";
      break;
    case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
      permission_name = "Camera";
      break;
    case COREWEBVIEW2_PERMISSION_KIND_GEOLOCATION:
      permission_name = "Geolocation";
      break;
    case COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
      permission_name = "Clipboard Read";
      break;
    default:
      permission_name = "Unknown (" + std::to_string(kind) + ")";
  }
  
  Logger::Instance().Warning("WebViewConfig", "Permission denied: " + permission_name);
  std::cout << "[AnyWP] [WebViewConfig] Permission denied: " << permission_name << std::endl;
}

void WebViewConfigurator::LogNavigationBlocked(const std::string& url) {
  Logger::Instance().Warning("WebViewConfig", "Navigation blocked: " + url);
  std::cout << "[AnyWP] [WebViewConfig] Navigation blocked: " << url << std::endl;
}

void WebViewConfigurator::LogConsoleMessage(const std::string& message) {
  Logger::Instance().Info("WebView Console", message);
}

}  // namespace anywp_engine

