#include "sdk_bridge.h"

#include <iostream>
#include <codecvt>
#include <locale>

namespace anywp_engine {

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
        
        // Convert wide string to UTF-8
        std::wstring wmessage(message);
        std::string msg;
        for (wchar_t c : wmessage) {
          if (c < 128) msg.push_back(static_cast<char>(c));
        }
        
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
  // NOTE: SDK is loaded via <script src> in HTML files
  // C++ injection is kept for compatibility but not required
  std::cout << "[AnyWP] [SDKBridge] SDK should be loaded via <script src> in HTML" << std::endl;
  
  // Return minimal compatibility shim
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

