#include "script_injection_manager.h"
#include "../utils/logger.h"
#include <fstream>
#include <sstream>

namespace anywp_engine {

ScriptInjectionManager::ScriptInjectionManager() {
  Logger::Instance().Info("ScriptInjectionManager", "Module initialized");
}

ScriptInjectionManager::~ScriptInjectionManager() {
  Logger::Instance().Info("ScriptInjectionManager", "Module destroyed");
}

void ScriptInjectionManager::Initialize(const std::string& sdk_path) {
  sdk_path_ = sdk_path.empty() ? "anywp_sdk.js" : sdk_path;
  cache_valid_ = false;
  Logger::Instance().Info("ScriptInjectionManager", "SDK path: " + sdk_path_);
}

bool ScriptInjectionManager::InjectSDK(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("ScriptInjectionManager", "WebView is null");
    return false;
  }

  try {
    std::string sdk_script = LoadSDKScript();
    if (sdk_script.empty()) {
      Logger::Instance().Error("ScriptInjectionManager", "SDK script is empty");
      return false;
    }

    std::wstring wide_script(sdk_script.begin(), sdk_script.end());
    HRESULT hr = webview->AddScriptToExecuteOnDocumentCreated(wide_script.c_str(), nullptr);
    
    if (FAILED(hr)) {
      Logger::Instance().Error("ScriptInjectionManager", 
        "Failed to inject SDK, HRESULT: " + std::to_string(hr));
      return false;
    }

    Logger::Instance().Info("ScriptInjectionManager", "SDK injected successfully");
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("ScriptInjectionManager", 
      std::string("Exception in InjectSDK: ") + e.what());
    return false;
  }
}

bool ScriptInjectionManager::SetupMessageBridge(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("ScriptInjectionManager", "WebView is null");
    return false;
  }

  // TODO: 设置 WebMessage 接收器
  Logger::Instance().Info("ScriptInjectionManager", "Message bridge setup (stub)");
  return true;
}

std::string ScriptInjectionManager::LoadSDKScript() {
  if (cache_valid_) {
    Logger::Instance().Debug("ScriptInjectionManager", "Using cached SDK script");
    return cached_sdk_script_;
  }

  try {
    std::ifstream file(sdk_path_);
    if (!file.is_open()) {
      Logger::Instance().Error("ScriptInjectionManager", "Cannot open SDK file: " + sdk_path_);
      return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    cached_sdk_script_ = buffer.str();
    cache_valid_ = true;

    Logger::Instance().Info("ScriptInjectionManager", 
      "SDK script loaded (" + std::to_string(cached_sdk_script_.length()) + " bytes)");
    
    return cached_sdk_script_;
  } catch (const std::exception& e) {
    Logger::Instance().Error("ScriptInjectionManager", 
      std::string("Exception in LoadSDKScript: ") + e.what());
    return "";
  }
}

void ScriptInjectionManager::ClearCache() {
  cached_sdk_script_.clear();
  cache_valid_ = false;
  Logger::Instance().Info("ScriptInjectionManager", "Cache cleared");
}

}  // namespace anywp_engine

