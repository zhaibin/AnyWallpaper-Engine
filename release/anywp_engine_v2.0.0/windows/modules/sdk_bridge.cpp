#include "sdk_bridge.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <codecvt>
#include <locale>

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
  std::cout << "[AnyWP] [SDKBridge] Injecting SDK into current page..." << std::endl;
  webview_->ExecuteScript(
    wsdk_script.c_str(),
    Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
      [](HRESULT result, LPCWSTR resultObjectAsJson) -> HRESULT {
        if (SUCCEEDED(result)) {
          std::cout << "[AnyWP] [SDKBridge] SDK executed successfully on current page" << std::endl;
        } else {
          std::cout << "[AnyWP] [SDKBridge] ERROR: Failed to execute SDK: " << std::hex << result << std::endl;
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
  
  // Determine message type
  std::string type = GetMessageType(message);
  
  if (type.empty()) {
    std::cout << "[AnyWP] [SDKBridge] Unknown message type (showing raw): " << message << std::endl;
    return;
  }
  
  // Find and invoke handler
  auto it = handlers_.find(type);
  if (it != handlers_.end()) {
    it->second(message);
  } else {
    std::cout << "[AnyWP] [SDKBridge] No handler registered for type: " << type << std::endl;
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
  
  // Try to load SDK from file
  // Priority 1: SDK file in windows/ directory (development mode)
  // Priority 2: SDK file in data/flutter_assets/ directory (release mode)
  
  std::vector<std::string> sdk_paths = {
    "windows\\anywp_sdk.js",           // Development: relative to project root
    "..\\anywp_sdk.js",                // Alternative: relative to executable
    "data\\flutter_assets\\windows\\anywp_sdk.js",  // Release: in assets
  };
  
  for (const auto& sdk_path : sdk_paths) {
    std::ifstream sdk_file(sdk_path);
    if (sdk_file.is_open()) {
      std::string sdk_content((std::istreambuf_iterator<char>(sdk_file)),
                              std::istreambuf_iterator<char>());
      sdk_file.close();
      
      if (!sdk_content.empty()) {
        std::cout << "[AnyWP] [SDKBridge] SDK loaded from: " << sdk_path 
                  << " (size: " << sdk_content.length() << " bytes)" << std::endl;
        
        // Cache the SDK script for future use
        cached_sdk_script_ = sdk_content;
        sdk_script_loaded_ = true;
        
        return sdk_content;
      }
    }
  }
  
  // Fallback: Return error shim if SDK file not found
  std::cout << "[AnyWP] [SDKBridge] WARNING: SDK file not found, using error shim" << std::endl;
  std::cout << "[AnyWP] [SDKBridge] Tried paths:" << std::endl;
  for (const auto& path : sdk_paths) {
    std::cout << "[AnyWP] [SDKBridge]   - " << path << std::endl;
  }
  
  return R"(
console.log('[AnyWP] Note: Full SDK should be loaded via <script src="../windows/anywp_sdk.js">');
if (!window.AnyWP) {
  console.error('[AnyWP] ERROR: SDK not loaded! Add <script src="../windows/anywp_sdk.js"></script> to your HTML');
  window.AnyWP = {
    version: '0.0.0-missing',
    error: 'SDK not loaded - add script tag to HTML'
  };
}
)";
}

std::string SDKBridge::GetMessageType(const std::string& message) {
  // Try to extract type field
  std::string type = ExtractJsonValue(message, "type");
  
  if (!type.empty()) {
    return type;
  }
  
  // Fallback: check for known patterns
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

