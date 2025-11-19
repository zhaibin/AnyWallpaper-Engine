#include "web_message_handler.h"
#include "../utils/logger.h"
#include "../utils/state_persistence.h"
#include <windows.h>
#include <shellapi.h>
#include <sstream>

namespace anywp_engine {

WebMessageHandler::WebMessageHandler() {
  Logger::Instance().Info("WebMessageHandler", "Module initialized");
}

WebMessageHandler::~WebMessageHandler() {
  Logger::Instance().Info("WebMessageHandler", "Module destroyed");
}

void WebMessageHandler::Initialize(StatePersistence* state_persistence) {
  state_persistence_ = state_persistence;
  message_counters_.clear();
  Logger::Instance().Info("WebMessageHandler", "Module configured");
}

bool WebMessageHandler::HandleMessage(const std::string& message, WallpaperInstance* instance) {
  if (message.empty()) {
    Logger::Instance().Warning("WebMessageHandler", "Received empty message");
    return false;
  }

  try {
    // 提取消息类型
    std::string message_type = ExtractMessageType(message);
    
    if (message_type.empty()) {
      Logger::Instance().Warning("WebMessageHandler", "Cannot extract message type");
      return false;
    }

    // 更新统计
    message_counters_[message_type]++;

    // 路由到对应的处理器
    if (message_type == "iframeData") {
      return HandleIframeDataMessage(message, instance);
    } else if (message_type == "openUrl") {
      return HandleOpenUrlMessage(message);
    } else if (message_type == "ready") {
      return HandleReadyMessage(message);
    } else if (message_type == "log") {
      return HandleLogMessage(message);
    } else if (message_type == "consoleLog") {
      return HandleConsoleLogMessage(message);
    } else if (message_type == "saveState") {
      return HandleSaveStateMessage(message);
    } else if (message_type == "loadState") {
      return HandleLoadStateMessage(message);
    } else if (message_type == "clearState") {
      return HandleClearStateMessage(message);
    } else {
      Logger::Instance().Warning("WebMessageHandler", 
        "Unknown message type: " + message_type);
      return false;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleMessage: ") + e.what());
    return false;
  }
}

void WebMessageHandler::SetIframeDataCallback(MessageCallback callback) {
  iframe_data_callback_ = callback;
}

void WebMessageHandler::SetUrlOpenCallback(UrlOpenCallback callback) {
  url_open_callback_ = callback;
}

void WebMessageHandler::SetReadyCallback(ReadyCallback callback) {
  ready_callback_ = callback;
}

void WebMessageHandler::SetLogCallback(LogCallback callback) {
  log_callback_ = callback;
}

void WebMessageHandler::SetScriptExecutionCallback(ScriptExecutionCallback callback) {
  script_execution_callback_ = callback;
  Logger::Instance().Info("WebMessageHandler", "Script execution callback registered");
}

// ========== Private Methods ==========

bool WebMessageHandler::HandleIframeDataMessage(const std::string& message, WallpaperInstance* instance) {
  Logger::Instance().Debug("WebMessageHandler", "Handling iframe data message");
  
  if (iframe_data_callback_) {
    try {
      iframe_data_callback_(message, instance);
      return true;
    } catch (const std::exception& e) {
      Logger::Instance().Error("WebMessageHandler", 
        std::string("Exception in iframe data callback: ") + e.what());
      return false;
    }
  }
  
  Logger::Instance().Warning("WebMessageHandler", "No iframe data callback registered");
  return false;
}

bool WebMessageHandler::HandleOpenUrlMessage(const std::string& message) {
  Logger::Instance().Debug("WebMessageHandler", "Handling open URL message");
  
  try {
    // Extract URL from JSON (简单解析，避免依赖 nlohmann::json)
    size_t url_start = message.find("\"url\":\"") + 7;
    size_t url_end = message.find("\"", url_start);
    
    if (url_start != std::string::npos && url_end != std::string::npos) {
      std::string url = message.substr(url_start, url_end - url_start);
      Logger::Instance().Info("WebMessageHandler", "Opening URL: " + url);

      if (url_open_callback_) {
        url_open_callback_(url);
        return true;
      } else {
        // 默认行为：使用 ShellExecute 打开
        std::wstring wurl(url.begin(), url.end());
        ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return true;
      }
    } else {
      Logger::Instance().Error("WebMessageHandler", "openUrl message missing 'url' field");
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleOpenUrlMessage: ") + e.what());
  }
  
  return false;
}

bool WebMessageHandler::HandleReadyMessage(const std::string& message) {
  // Extract name
  size_t name_start = message.find("\"name\":\"") + 8;
  size_t name_end = message.find("\"", name_start);
  
  if (name_start != std::string::npos && name_end != std::string::npos) {
    std::string name = message.substr(name_start, name_end - name_start);
    Logger::Instance().Info("WebMessageHandler", "Wallpaper ready: " + name);
  } else {
    Logger::Instance().Info("WebMessageHandler", "WebView content is ready");
  }
  
  if (ready_callback_) {
    try {
      ready_callback_();
      return true;
    } catch (const std::exception& e) {
      Logger::Instance().Error("WebMessageHandler", 
        std::string("Exception in ready callback: ") + e.what());
      return false;
    }
  }
  
  return true;  // 就绪消息即使没有回调也算成功
}

bool WebMessageHandler::HandleLogMessage(const std::string& message) {
  try {
    // Extract log message
    size_t msg_start = message.find("\"message\":\"") + 11;
    size_t msg_end = message.find("\"", msg_start);
    
    if (msg_start != std::string::npos && msg_end != std::string::npos) {
      std::string log_msg = message.substr(msg_start, msg_end - msg_start);
      
      if (log_callback_) {
        log_callback_("info", log_msg);
      } else {
        // 默认行为：输出到 Logger
        Logger::Instance().Info("WebLog", log_msg);
      }
      
      return true;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleLogMessage: ") + e.what());
  }
  
  return false;
}

bool WebMessageHandler::HandleConsoleLogMessage(const std::string& message) {
  try {
    // Enhanced console.log forwarding with level support
    size_t msg_start = message.find("\"message\":\"") + 11;
    size_t msg_end = message.rfind("\"");
    
    if (msg_start != std::string::npos && msg_end != std::string::npos && msg_end > msg_start) {
      std::string log_msg = message.substr(msg_start, msg_end - msg_start);
      bool is_error = message.find("\"level\":\"error\"") != std::string::npos;
      bool is_warn = message.find("\"level\":\"warn\"") != std::string::npos;
      
      if (is_error) {
        Logger::Instance().Error("JavaScript", log_msg);
      } else if (is_warn) {
        Logger::Instance().Warning("JavaScript", log_msg);
      } else {
        Logger::Instance().Debug("JavaScript", log_msg);
      }
      
      return true;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleConsoleLogMessage: ") + e.what());
  }
  
  return false;
}

bool WebMessageHandler::HandleSaveStateMessage(const std::string& message) {
  if (!state_persistence_) {
    Logger::Instance().Error("WebMessageHandler", "StatePersistence not initialized");
    return false;
  }

  try {
    // Extract key and value from JSON
    size_t key_start = message.find("\"key\":\"") + 7;
    size_t key_end = message.find("\"", key_start);
    size_t value_start = message.find("\"value\":\"") + 9;
    // Find the closing brace, then work backwards to find last quote
    size_t end_brace = message.rfind("}");
    size_t value_end = message.rfind("\"", end_brace);
    
    if (key_start != std::string::npos && key_end != std::string::npos &&
        value_start != std::string::npos && value_end != std::string::npos && value_end > value_start) {
      std::string key = message.substr(key_start, key_end - key_start);
      std::string value = message.substr(value_start, value_end - value_start);
      
      bool success = state_persistence_->SaveState(key, value);
      Logger::Instance().Info("WebMessageHandler", 
        "[State] Saved via WebMessage: " + key + " = " + value);
      
      // Send success notification back to WebView
      std::ostringstream detail_json;
      detail_json << "{\"type\": \"stateSaved\", \"key\": \"" << key 
                  << "\", \"success\": " << (success ? "true" : "false") << "}";
      SendResponseToWebView("AnyWP:stateSaved", detail_json.str());
      
      return success;
    } else {
      Logger::Instance().Error("WebMessageHandler", "Failed to parse saveState message");
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleSaveStateMessage: ") + e.what());
  }
  
  return false;
}

bool WebMessageHandler::HandleLoadStateMessage(const std::string& message) {
  if (!state_persistence_) {
    Logger::Instance().Error("WebMessageHandler", "StatePersistence not initialized");
    return false;
  }

  try {
    // Extract key
    size_t key_start = message.find("\"key\":\"") + 7;
    size_t key_end = message.find("\"", key_start);
    
    if (key_start != std::string::npos && key_end != std::string::npos) {
      std::string key = message.substr(key_start, key_end - key_start);
      std::string value = state_persistence_->LoadState(key);
      
      Logger::Instance().Info("WebMessageHandler", 
        "[State] Loaded via WebMessage: " + key + " = " + value);
      
      // Send result back to WebView
      std::ostringstream detail_json;
      detail_json << "{\"type\": \"stateLoaded\", \"key\": \"" << key 
                  << "\", \"value\": \"" << value << "\"}";
      SendResponseToWebView("AnyWP:stateLoaded", detail_json.str());
      
      return true;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleLoadStateMessage: ") + e.what());
  }
  
  return false;
}

bool WebMessageHandler::HandleClearStateMessage(const std::string& message) {
  if (!state_persistence_) {
    Logger::Instance().Error("WebMessageHandler", "StatePersistence not initialized");
    return false;
  }

  try {
    bool success = state_persistence_->ClearState();
    Logger::Instance().Info("WebMessageHandler", "[State] Cleared all state via WebMessage");
    
    // Send success notification back to WebView
    std::ostringstream detail_json;
    detail_json << "{\"type\": \"stateCleared\", \"success\": " 
                << (success ? "true" : "false") << "}";
    SendResponseToWebView("AnyWP:stateCleared", detail_json.str());
    
    return success;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleClearStateMessage: ") + e.what());
    return false;
  }
}

std::string WebMessageHandler::ExtractMessageType(const std::string& message) {
  try {
    // Extract type field from JSON (简单解析)
    size_t type_start = message.find("\"type\":\"") + 8;
    size_t type_end = message.find("\"", type_start);
    
    if (type_start != std::string::npos && type_end != std::string::npos) {
      return message.substr(type_start, type_end - type_start);
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in ExtractMessageType: ") + e.what());
  }
  
  return "";
}

std::string WebMessageHandler::ExtractMessageData(const std::string& message) {
  try {
    size_t data_start = message.find("\"data\":");
    if (data_start != std::string::npos) {
      return message.substr(data_start + 7);  // Skip '"data":'
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in ExtractMessageData: ") + e.what());
  }
  
  return "";
}

void WebMessageHandler::SendResponseToWebView(
    const std::string& event_name, 
    const std::string& detail_json) {
  
  if (!script_execution_callback_) {
    Logger::Instance().Warning("WebMessageHandler", 
      "No script execution callback registered, cannot send response");
    return;
  }

  try {
    // Build JavaScript to dispatch custom event
    std::ostringstream js;
    js << "window.dispatchEvent(new CustomEvent('" << event_name << "', {"
       << "detail: " << detail_json
       << "}));";
    
    std::string js_code = js.str();
    std::wstring wjs_code(js_code.begin(), js_code.end());
    
    script_execution_callback_(wjs_code);
    Logger::Instance().Debug("WebMessageHandler", 
      "Sent response event: " + event_name);
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in SendResponseToWebView: ") + e.what());
  }
}

}  // namespace anywp_engine

