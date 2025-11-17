#include "sdk_bridge.h"
#include "../sdk_loader.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <codecvt>
#include <locale>
#include <windows.h>

namespace anywp_engine {

// Static member initialization
std::string SDKBridge::cached_sdk_script_ = "";
bool SDKBridge::sdk_script_loaded_ = false;

SDKBridge::SDKBridge() {
}

SDKBridge::~SDKBridge() {
}

// ========== WebView Management ==========

void SDKBridge::SetWebView(Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
  webview_ = webview;
  std::cout << "[AnyWP] [SDKBridge] WebView set" << std::endl;
}

Microsoft::WRL::ComPtr<ICoreWebView2> SDKBridge::GetWebView() const {
  return webview_;
}

// ========== SDK Injection ==========

void SDKBridge::InjectSDK() {
  if (!webview_) {
    std::cout << "[AnyWP] [SDKBridge] ERROR: WebView not set" << std::endl;
    return;
  }
  
  std::cout << "[AnyWP] [SDKBridge] Injecting AnyWallpaper SDK..." << std::endl;
  
  // Load SDK script
  std::string sdk_script = LoadSDKScript();
  std::wstring wsdk_script(sdk_script.begin(), sdk_script.end());
  
  std::cout << "[AnyWP] [SDKBridge] SDK script size: " << sdk_script.length() << " bytes" << std::endl;
  
  // Inject on every navigation (for future navigations)
  webview_->AddScriptToExecuteOnDocumentCreated(
    wsdk_script.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
      [](HRESULT result, LPCWSTR id) -> HRESULT {
        if (SUCCEEDED(result)) {
          std::wcout << L"[AnyWP] [SDKBridge] SDK registered for future pages, ID: " << id << std::endl;
        } else {
          std::cout << "[AnyWP] [SDKBridge] ERROR: Failed to register SDK: " << std::hex << result << std::endl;
        }
        return S_OK;
      }).Get());
  
  // IMPORTANT: Also inject immediately for current page
  // Note: This may fail if page hasn't loaded yet, but AddScriptToExecuteOnDocumentCreated
  // will ensure SDK is injected when page is created
  std::cout << "[AnyWP] [SDKBridge] Attempting to inject SDK into current page..." << std::endl;
  webview_->ExecuteScript(
    wsdk_script.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
      [](HRESULT result, LPCWSTR resultObjectAsJson) -> HRESULT {
        if (SUCCEEDED(result)) {
          std::cout << "[AnyWP] [SDKBridge] SDK executed successfully on current page" << std::endl;
        } else {
          std::cout << "[AnyWP] [SDKBridge] WARNING: Failed to execute SDK on current page (may not be loaded yet): " 
                    << std::hex << result << std::endl;
          std::cout << "[AnyWP] [SDKBridge] SDK will be injected via AddScriptToExecuteOnDocumentCreated when page loads" << std::endl;
        }
        return S_OK;
      }).Get());
  
  // Add verification script to check if SDK is loaded
  std::wstring verify_script = L"setTimeout(function() {"
    L"  if (window.AnyWP && window.AnyWP.version) {"
    L"    console.log('[AnyWP] SDK verification: SDK loaded successfully, version:', window.AnyWP.version);"
    L"    if (window.chrome && window.chrome.webview && window.chrome.webview.postMessage) {"
    L"      window.chrome.webview.postMessage(JSON.stringify({type:'sdkReady', version: window.AnyWP.version}));"
    L"    }"
    L"  } else {"
    L"    console.error('[AnyWP] SDK verification: SDK NOT loaded! window.AnyWP:', window.AnyWP);"
    L"    if (window.chrome && window.chrome.webview && window.chrome.webview.postMessage) {"
    L"      window.chrome.webview.postMessage(JSON.stringify({type:'sdkError', error:'SDK not found'}));"
    L"    }"
    L"  }"
    L"}, 1000);";
  
  webview_->AddScriptToExecuteOnDocumentCreated(
    verify_script.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
      [](HRESULT result, LPCWSTR id) -> HRESULT {
        if (SUCCEEDED(result)) {
          std::wcout << L"[AnyWP] [SDKBridge] SDK verification script registered, ID: " << id << std::endl;
        } else {
          std::cout << "[AnyWP] [SDKBridge] WARNING: Failed to register verification script: " << std::hex << result << std::endl;
        }
        return S_OK;
      }).Get());
}

void SDKBridge::SetupMessageBridge() {
  if (!webview_) {
    std::cout << "[AnyWP] [SDKBridge] ERROR: WebView not set" << std::endl;
    return;
  }
  
  std::cout << "[AnyWP] [SDKBridge] Setting up message bridge..." << std::endl;
  
  webview_->add_WebMessageReceived(
    Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
      [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
        LPWSTR message;
        args->get_WebMessageAsJson(&message);
        
        // Convert wide string to UTF-8 (proper conversion)
        std::wstring wmessage(message);
        
        // Use WideCharToMultiByte for proper UTF-8 conversion
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wmessage.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string msg(size_needed - 1, 0);  // -1 to exclude null terminator
        WideCharToMultiByte(CP_UTF8, 0, wmessage.c_str(), -1, &msg[0], size_needed, nullptr, nullptr);
        
        // Check if this is a pause/resume result message
        if (msg.find("\"type\":\"pauseResult\"") != std::string::npos || 
            msg.find("\"type\":\"resumeResult\"") != std::string::npos) {
          std::cout << "[AnyWP] [SDKBridge] Script Result: " << msg << std::endl;
        }
        
        HandleMessage(msg);
        
        CoTaskMemFree(message);
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [SDKBridge] Message bridge ready" << std::endl;
}

// ========== Message Handling ==========

void SDKBridge::RegisterHandler(const std::string& message_type, MessageHandler handler) {
  handlers_[message_type] = handler;
  std::cout << "[AnyWP] [SDKBridge] Registered handler for: " << message_type << std::endl;
}

void SDKBridge::UnregisterHandler(const std::string& message_type) {
  handlers_.erase(message_type);
  std::cout << "[AnyWP] [SDKBridge] Unregistered handler for: " << message_type << std::endl;
}

void SDKBridge::HandleMessage(const std::string& message) {
  std::cout << "[AnyWP] [SDKBridge] Received message: " << message << std::endl;
  
  // Determine message type first
  std::string type = GetMessageType(message);
  
  // Check for SDK verification messages (handle before other handlers)
  if (type == "sdkReady" || message.find("\"type\":\"sdkReady\"") != std::string::npos) {
    std::cout << "[AnyWP] [SDKBridge] SDK verification: SDK loaded successfully" << std::endl;
    // Extract version if present
    std::string version = ExtractJsonValue(message, "version");
    if (!version.empty()) {
      std::cout << "[AnyWP] [SDKBridge] SDK version: " << version << std::endl;
    }
    return;
  }
  
  if (type == "sdkError" || message.find("\"type\":\"sdkError\"") != std::string::npos) {
    std::cout << "[AnyWP] [SDKBridge] SDK verification: SDK NOT loaded" << std::endl;
    std::string error = ExtractJsonValue(message, "error");
    if (!error.empty()) {
      std::cout << "[AnyWP] [SDKBridge] Error: " << error << std::endl;
    }
    return;
  }
  
  if (type.empty()) {
    std::cout << "[AnyWP] [SDKBridge] Unknown message type (showing raw): " << message << std::endl;
    return;
  }
  
  // v2.1.0+ Bidirectional Communication: Forward ALL messages to Flutter
  // This allows Flutter to handle any message type from JavaScript
  std::cout << "[AnyWP] [SDKBridge] Forwarding message to Flutter (type: " << type << ")" << std::endl;
  ForwardMessageToFlutter(message);
  
  // Also invoke registered handler (if any) for backward compatibility
  auto it = handlers_.find(type);
  if (it != handlers_.end()) {
    std::cout << "[AnyWP] [SDKBridge] Invoking registered handler for type: " << type << std::endl;
    it->second(message);
  } else {
    std::cout << "[AnyWP] [SDKBridge] No registered handler for type: " << type << " (message still forwarded to Flutter)" << std::endl;
  }
}

// ========== Flutter Message Forwarding ==========

void SDKBridge::SetFlutterCallback(std::function<void(const std::string&)> callback) {
  flutter_callback_ = callback;
  std::cout << "[AnyWP] [SDKBridge] Flutter callback registered" << std::endl;
}

void SDKBridge::ForwardMessageToFlutter(const std::string& message) {
  if (!flutter_callback_) {
    std::cout << "[AnyWP] [SDKBridge] WARNING: Flutter callback not set, cannot forward message" << std::endl;
    return;
  }

  std::cout << "[AnyWP] [SDKBridge] Forwarding to Flutter: " << message.substr(0, 100) 
            << (message.length() > 100 ? "..." : "") << std::endl;

  try {
    // Call Flutter callback
    flutter_callback_(message);
    std::cout << "[AnyWP] [SDKBridge] Message forwarded successfully" << std::endl;
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [SDKBridge] ERROR: Exception during message forwarding: " 
              << e.what() << std::endl;
  }
}

// ========== Script Execution ==========

bool SDKBridge::ExecuteScript(const std::wstring& script) {
  if (!webview_) {
    std::cout << "[AnyWP] [SDKBridge] ERROR: WebView not set" << std::endl;
    return false;
  }
  
  webview_->ExecuteScript(script.c_str(), nullptr);
  return true;
}

bool SDKBridge::ExecuteScript(const std::string& script) {
  std::wstring wscript(script.begin(), script.end());
  return ExecuteScript(wscript);
}

// ========== Utility ==========

std::string SDKBridge::ExtractJsonValue(const std::string& json, const std::string& key) {
  std::string search = "\"" + key + "\":\"";
  size_t start = json.find(search);
  if (start == std::string::npos) {
    return "";
  }
  
  start += search.length();
  size_t end = json.find("\"", start);
  if (end == std::string::npos) {
    return "";
  }
  
  return json.substr(start, end - start);
}

// ========== Private Helpers ==========

std::string SDKBridge::LoadSDKScript() {
  // Performance optimization: Use cached SDK if already loaded
  if (sdk_script_loaded_ && !cached_sdk_script_.empty()) {
    std::cout << "[AnyWP] [SDKBridge] Using cached SDK (size: " 
              << cached_sdk_script_.length() << " bytes)" << std::endl;
    return cached_sdk_script_;
  }
  
  // v2.3.0+: Use new SDK loader with embedded resource support
  // Priority 1: Embedded in DLL (production, recommended)
  // Priority 2: File system fallback (development, backward compatibility)
  std::string sdk_content = anywp::sdk::LoadSDKScript();
  
  if (!sdk_content.empty()) {
    // Cache the SDK script for future use
    cached_sdk_script_ = sdk_content;
    sdk_script_loaded_ = true;
    
    std::cout << "[AnyWP] [SDKBridge] SDK loaded successfully (size: " 
              << sdk_content.length() << " bytes)" << std::endl;
    
    return sdk_content;
  }
  
  // Should never reach here if SDK is embedded correctly
  std::cout << "[AnyWP] [SDKBridge] WARNING: SDK not found!" << std::endl;
  std::cout << "[AnyWP] [SDKBridge] Please ensure:" << std::endl;
  std::cout << "[AnyWP] [SDKBridge]   1. SDK is embedded in DLL (v2.3.0+)" << std::endl;
  std::cout << "[AnyWP] [SDKBridge]   2. Or SDK file exists at sdk/dist/anywp_sdk.js" << std::endl;
  
  return "";
}

std::string SDKBridge::GetMessageType(const std::string& message) {
  // Try to extract type field
  std::string type = ExtractJsonValue(message, "type");
  
  if (!type.empty()) {
    return type;
  }
  
  // Fallback: check for known patterns
  if (message.find("\"type\":\"sdkReady\"") != std::string::npos) return "sdkReady";
  if (message.find("\"type\":\"sdkError\"") != std::string::npos) return "sdkError";
  if (message.find("\"type\":\"IFRAME_DATA\"") != std::string::npos) return "IFRAME_DATA";
  if (message.find("\"type\":\"OPEN_URL\"") != std::string::npos) return "OPEN_URL";
  if (message.find("\"type\":\"openURL\"") != std::string::npos) return "openURL";
  if (message.find("\"type\":\"READY\"") != std::string::npos) return "READY";
  if (message.find("\"type\":\"ready\"") != std::string::npos) return "ready";
  if (message.find("\"type\":\"LOG\"") != std::string::npos) return "LOG";
  if (message.find("\"type\":\"console_log\"") != std::string::npos) return "console_log";
  if (message.find("\"type\":\"saveState\"") != std::string::npos) return "saveState";
  if (message.find("\"type\":\"loadState\"") != std::string::npos) return "loadState";
  if (message.find("\"type\":\"clearState\"") != std::string::npos) return "clearState";
  
  return "";
}

}  // namespace anywp_engine

