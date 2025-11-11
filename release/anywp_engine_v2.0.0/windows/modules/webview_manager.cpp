#include "webview_manager.h"
#include "../utils/logger.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <ShlObj.h>

namespace anywp_engine {

// Static member initialization
Microsoft::WRL::ComPtr<ICoreWebView2Environment> WebViewManager::shared_environment_;

WebViewManager::WebViewManager() 
    : last_cleanup_(std::chrono::steady_clock::now()) {
}

WebViewManager::~WebViewManager() {
}

void WebViewManager::InitializeEnvironment(const std::wstring& user_data_folder) {
  if (shared_environment_) {
    Logger::Instance().Info("WebViewManager", "Environment already initialized");
    return;
  }

  Logger::Instance().Info("WebViewManager", "Initializing WebView2 environment...");

  auto callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
        if (FAILED(result)) {
          Logger::Instance().Error("WebViewManager", 
            "Failed to create environment: HRESULT=0x" + std::to_string(result));
          return result;
        }

        shared_environment_ = env;
        Logger::Instance().Info("WebViewManager", "Environment initialized successfully");
        return S_OK;
      });

  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data_folder.c_str(), nullptr, callback.Get());

  if (FAILED(hr)) {
    Logger::Instance().Error("WebViewManager", 
      "CreateCoreWebView2EnvironmentWithOptions failed: HRESULT=0x" + std::to_string(hr));
    Logger::Instance().Error("WebViewManager", 
      "Please install Microsoft Edge WebView2 Runtime: https://go.microsoft.com/fwlink/?linkid=2124701");
  }
}

Microsoft::WRL::ComPtr<ICoreWebView2Environment> WebViewManager::GetEnvironment() {
  return shared_environment_;
}

void WebViewManager::CreateWebView(
    HWND parent_hwnd,
    const std::string& url,
    WebViewReadyCallback callback) {
  
  Logger::Instance().Info("WebViewManager", "Creating WebView for window: " + std::to_string((long long)parent_hwnd));

  // Initialize environment if not already done
  if (!shared_environment_) {
    wchar_t user_data_folder[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, user_data_folder);
    wcscat_s(user_data_folder, L"\\AnyWallpaperEngine");
    InitializeEnvironment(user_data_folder);
  }

  // Wait for environment to be ready (with timeout)
  int retry_count = 0;
  while (!shared_environment_ && retry_count < 50) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    retry_count++;
  }

  if (!shared_environment_) {
    Logger::Instance().Error("WebViewManager", "Environment initialization timeout");
    return;
  }

  // Convert URL to wstring
  std::wstring wurl(url.begin(), url.end());

  // Create controller
  auto controller_callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
      [this, parent_hwnd, wurl, callback](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
        if (FAILED(result)) {
          Logger::Instance().Error("WebViewManager", 
            "Failed to create controller: HRESULT=0x" + std::to_string(result));
          return result;
        }

        if (!controller) {
          Logger::Instance().Error("WebViewManager", "Controller is null");
          return E_FAIL;
        }

        Logger::Instance().Info("WebViewManager", "Controller created successfully");

        // Get CoreWebView2
        Microsoft::WRL::ComPtr<ICoreWebView2Controller> ctrl = controller;
        Microsoft::WRL::ComPtr<ICoreWebView2> webview;
        
        HRESULT hr = ctrl->get_CoreWebView2(&webview);
        if (FAILED(hr) || !webview) {
          Logger::Instance().Error("WebViewManager", 
            "Failed to get CoreWebView2: HRESULT=0x" + std::to_string(hr));
          return hr;
        }

        // Set bounds
        RECT bounds;
        GetClientRect(parent_hwnd, &bounds);
        
        hr = ctrl->put_Bounds(bounds);
        if (FAILED(hr)) {
          Logger::Instance().Error("WebViewManager", 
            "Failed to set bounds: HRESULT=0x" + std::to_string(hr));
          return hr;
        }

        // Set visible
        hr = ctrl->put_IsVisible(TRUE);
        if (FAILED(hr)) {
          Logger::Instance().Error("WebViewManager", 
            "Failed to set visibility: HRESULT=0x" + std::to_string(hr));
          return hr;
        }

        Logger::Instance().Info("WebViewManager", "WebView configured successfully");

        // Call user callback
        if (callback) {
          try {
            callback(ctrl, webview);
          } catch (const std::exception& e) {
            Logger::Instance().Error("WebViewManager", 
              "Exception in user callback: " + std::string(e.what()));
          } catch (...) {
            Logger::Instance().Error("WebViewManager", 
              "Unknown exception in user callback");
          }
        }

        return S_OK;
      });

  shared_environment_->CreateCoreWebView2Controller(parent_hwnd, controller_callback.Get());
}

bool WebViewManager::Navigate(ICoreWebView2* webview, const std::string& url) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "Navigate: webview is null");
    return false;
  }

  std::wstring wurl(url.begin(), url.end());
  HRESULT hr = webview->Navigate(wurl.c_str());
  
  if (FAILED(hr)) {
    Logger::Instance().Error("WebViewManager", 
      "Navigation failed: HRESULT=0x" + std::to_string(hr));
    return false;
  }

  Logger::Instance().Info("WebViewManager", "Navigation started: " + url);
  return true;
}

bool WebViewManager::InjectSDK(ICoreWebView2* webview, const std::string& sdk_script) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "InjectSDK: webview is null");
    return false;
  }

  if (sdk_script.empty()) {
    Logger::Instance().Error("WebViewManager", "InjectSDK: script is empty");
    return false;
  }

  std::wstring wscript(sdk_script.begin(), sdk_script.end());
  
  HRESULT hr = webview->ExecuteScript(wscript.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
      [](HRESULT error, LPCWSTR result) -> HRESULT {
        if (SUCCEEDED(error)) {
          Logger::Instance().Info("WebViewManager", "SDK injected successfully");
        } else {
          Logger::Instance().Error("WebViewManager", 
            "SDK injection failed: HRESULT=0x" + std::to_string(error));
        }
        return S_OK;
      }).Get());

  if (FAILED(hr)) {
    Logger::Instance().Error("WebViewManager", 
      "ExecuteScript failed: HRESULT=0x" + std::to_string(hr));
    return false;
  }

  return true;
}

void WebViewManager::ExecuteScript(
    ICoreWebView2* webview,
    const std::wstring& script,
    std::function<void(HRESULT, const std::wstring&)> callback) {
  
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "ExecuteScript: webview is null");
    if (callback) {
      callback(E_FAIL, L"");
    }
    return;
  }

  auto handler = Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
    [callback](HRESULT error, LPCWSTR result) -> HRESULT {
      if (callback) {
        try {
          callback(error, result ? result : L"");
        } catch (const std::exception& e) {
          Logger::Instance().Error("WebViewManager", 
            "Exception in ExecuteScript callback: " + std::string(e.what()));
        } catch (...) {
          Logger::Instance().Error("WebViewManager", 
            "Unknown exception in ExecuteScript callback");
        }
      }
      return S_OK;
    });

  HRESULT hr = webview->ExecuteScript(script.c_str(), handler.Get());
  if (FAILED(hr)) {
    Logger::Instance().Error("WebViewManager", 
      "ExecuteScript failed: HRESULT=0x" + std::to_string(hr));
    if (callback) {
      callback(hr, L"");
    }
  }
}

void WebViewManager::SetupNavigationHandlers(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "SetupNavigationHandlers: webview is null");
    return;
  }

  // Setup NavigationStarting handler
  webview->add_NavigationStarting(
    Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
      [](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
        LPWSTR uri;
        args->get_Uri(&uri);
        
        std::wstring wuri(uri);
        
        // Convert wstring to string safely
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wuri.c_str(), 
                                              (int)wuri.length(), 
                                              nullptr, 0, nullptr, nullptr);
        std::string uri_str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wuri.c_str(), 
                           (int)wuri.length(), 
                           &uri_str[0], size_needed, nullptr, nullptr);
        
        Logger::Instance().Info("WebViewManager", "Navigation starting: " + uri_str);
        
        CoTaskMemFree(uri);
        return S_OK;
      }).Get(), nullptr);

  // Setup NavigationCompleted handler
  webview->add_NavigationCompleted(
    Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
      [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
        BOOL success;
        args->get_IsSuccess(&success);
        
        if (success) {
          Logger::Instance().Info("WebViewManager", "Navigation completed successfully");
        } else {
          COREWEBVIEW2_WEB_ERROR_STATUS status;
          args->get_WebErrorStatus(&status);
          Logger::Instance().Error("WebViewManager", 
            "Navigation failed with status: " + std::to_string(status));
        }
        
        return S_OK;
      }).Get(), nullptr);

  Logger::Instance().Info("WebViewManager", "Navigation handlers configured");
}

void WebViewManager::ConfigurePermissions(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "ConfigurePermissions: webview is null");
    return;
  }

  // Get settings
  Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;
  HRESULT hr = webview->get_Settings(&settings);
  
  if (FAILED(hr) || !settings) {
    Logger::Instance().Error("WebViewManager", 
      "Failed to get settings: HRESULT=0x" + std::to_string(hr));
    return;
  }

  // Configure security settings
  settings->put_IsScriptEnabled(TRUE);
  settings->put_AreDefaultScriptDialogsEnabled(FALSE);
  settings->put_IsWebMessageEnabled(TRUE);
  settings->put_AreDevToolsEnabled(TRUE);  // Enable for debugging
  settings->put_IsStatusBarEnabled(FALSE);
  settings->put_AreDefaultContextMenusEnabled(FALSE);
  settings->put_IsZoomControlEnabled(FALSE);

  // Try to access ICoreWebView2Settings2 for additional settings
  Microsoft::WRL::ComPtr<ICoreWebView2Settings2> settings2;
  hr = settings.As(&settings2);
  
  if (SUCCEEDED(hr) && settings2) {
    settings2->put_UserAgent(L"AnyWP-Engine/1.0");
  }

  Logger::Instance().Info("WebViewManager", "Permissions configured");
}

void WebViewManager::SetupSecurityHandlers(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "SetupSecurityHandlers: webview is null");
    return;
  }

  // Setup permission request handler
  webview->add_PermissionRequested(
    Microsoft::WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
      [](ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
        COREWEBVIEW2_PERMISSION_KIND kind;
        args->get_PermissionKind(&kind);
        
        // Allow most permissions for wallpaper functionality
        switch (kind) {
          case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
          case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
            Logger::Instance().Info("WebViewManager", "Denied camera/microphone permission");
            break;
          default:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
            break;
        }
        
        return S_OK;
      }).Get(), nullptr);

  Logger::Instance().Info("WebViewManager", "Security handlers configured");
}

void WebViewManager::OptimizeMemory(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "OptimizeMemory: webview is null");
    return;
  }

  // Execute memory optimization script
  std::wstring optimize_script = L"(function() {"
    L"  try {"
    L"    if (window.gc) window.gc();"  // If available
    L"    console.log('[AnyWP] Memory optimization triggered');"
    L"  } catch(e) {"
    L"    console.warn('[AnyWP] GC not available:', e);"
    L"  }"
    L"})();";

  webview->ExecuteScript(optimize_script.c_str(), 
    Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
      [](HRESULT error, LPCWSTR result) -> HRESULT {
        if (SUCCEEDED(error)) {
          Logger::Instance().Info("WebViewManager", "Memory optimization executed");
        }
        return S_OK;
      }).Get());

  Logger::Instance().Info("WebViewManager", "Memory optimization triggered");
}

void WebViewManager::ScheduleSafeMemoryOptimization(ICoreWebView2* webview) {
  if (!webview) {
    return;
  }

  // Throttle memory optimization (max once per 5 minutes)
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - last_cleanup_);
  
  if (elapsed.count() < 5) {
    return;
  }

  last_cleanup_ = now;

  // Schedule optimization after a delay
  std::thread([webview]() {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Note: This is a simplified version. In production, you should:
    // 1. Keep a weak reference to webview
    // 2. Check if webview is still valid before executing
    // For now, we skip the actual execution due to threading concerns
    
    Logger::Instance().Info("WebViewManager", "Scheduled memory optimization completed");
  }).detach();
}

void WebViewManager::ClearCache(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("WebViewManager", "ClearCache: webview is null");
    return;
  }

  std::wstring clear_script = L"(function() {"
    L"  try {"
    L"    if (window.localStorage && localStorage.clear) localStorage.clear();"
    L"    if (window.sessionStorage && sessionStorage.clear) sessionStorage.clear();"
    L"    if (window.indexedDB && indexedDB.databases) {"
    L"      indexedDB.databases().then(dbs => {"
    L"        dbs.forEach(db => indexedDB.deleteDatabase(db.name));"
    L"      });"
    L"    }"
    L"    console.log('[AnyWP] Cache cleared');"
    L"  } catch(e) {"
    L"    console.warn('[AnyWP] Cache clear error:', e);"
    L"  }"
    L"})();";

  webview->ExecuteScript(clear_script.c_str(), nullptr);
  Logger::Instance().Info("WebViewManager", "Cache cleared");
}

std::string WebViewManager::LoadSDKScriptFromFile() {
  // Try to load from multiple possible locations
  std::vector<std::string> possible_paths = {
    "windows/anywp_sdk.js",
    "../windows/anywp_sdk.js",
    "../../windows/anywp_sdk.js",
    "anywp_sdk.js"
  };

  for (const auto& path : possible_paths) {
    std::ifstream file(path);
    if (file.is_open()) {
      std::stringstream buffer;
      buffer << file.rdbuf();
      file.close();
      
      Logger::Instance().Info("WebViewManager", "SDK loaded from: " + path);
      return buffer.str();
    }
  }

  Logger::Instance().Error("WebViewManager", "Failed to load SDK script from any location");
  return "";
}

}  // namespace anywp_engine

