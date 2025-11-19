#include "web_message_handler.h"
#include "../utils/logger.h"
#include "../utils/state_persistence.h"
#include "anywp_engine_plugin.h"
#include <nlohmann/json.hpp>

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
    auto json_obj = nlohmann::json::parse(message);
    if (!json_obj.contains("url")) {
      Logger::Instance().Error("WebMessageHandler", "openUrl message missing 'url' field");
      return false;
    }

    std::string url = json_obj["url"].get<std::string>();
    Logger::Instance().Info("WebMessageHandler", "Opening URL: " + url);

    if (url_open_callback_) {
      url_open_callback_(url);
      return true;
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleOpenUrlMessage: ") + e.what());
  }
  
  return false;
}

bool WebMessageHandler::HandleReadyMessage(const std::string& message) {
  Logger::Instance().Info("WebMessageHandler", "WebView content is ready");
  
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
    auto json_obj = nlohmann::json::parse(message);
    std::string log_level = json_obj.value("level", "info");
    std::string log_message = json_obj.value("message", "");

    if (log_callback_) {
      log_callback_(log_level, log_message);
    } else {
      // 默认行为：输出到 Logger
      Logger::Instance().Info("WebContent", "[" + log_level + "] " + log_message);
    }
    
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleLogMessage: ") + e.what());
    return false;
  }
}

bool WebMessageHandler::HandleConsoleLogMessage(const std::string& message) {
  try {
    auto json_obj = nlohmann::json::parse(message);
    std::string console_message = json_obj.value("message", "");
    
    Logger::Instance().Info("WebConsole", console_message);
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleConsoleLogMessage: ") + e.what());
    return false;
  }
}

bool WebMessageHandler::HandleSaveStateMessage(const std::string& message) {
  if (!state_persistence_) {
    Logger::Instance().Error("WebMessageHandler", "StatePersistence not initialized");
    return false;
  }

  try {
    auto json_obj = nlohmann::json::parse(message);
    std::string key = json_obj.value("key", "");
    std::string value = json_obj.value("value", "");

    if (key.empty()) {
      Logger::Instance().Error("WebMessageHandler", "saveState message missing 'key' field");
      return false;
    }

    bool success = state_persistence_->SaveState(key, value);
    if (success) {
      Logger::Instance().Debug("WebMessageHandler", "State saved: " + key);
    } else {
      Logger::Instance().Error("WebMessageHandler", "Failed to save state: " + key);
    }
    
    return success;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleSaveStateMessage: ") + e.what());
    return false;
  }
}

bool WebMessageHandler::HandleLoadStateMessage(const std::string& message) {
  if (!state_persistence_) {
    Logger::Instance().Error("WebMessageHandler", "StatePersistence not initialized");
    return false;
  }

  try {
    auto json_obj = nlohmann::json::parse(message);
    std::string key = json_obj.value("key", "");

    if (key.empty()) {
      Logger::Instance().Error("WebMessageHandler", "loadState message missing 'key' field");
      return false;
    }

    std::string value = state_persistence_->LoadState(key);
    Logger::Instance().Debug("WebMessageHandler", 
      "State loaded: " + key + " (length: " + std::to_string(value.length()) + ")");
    
    // TODO: 需要将结果返回给 WebView2
    // 可能需要添加回调或通过 ExecuteScript 返回
    
    return true;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleLoadStateMessage: ") + e.what());
    return false;
  }
}

bool WebMessageHandler::HandleClearStateMessage(const std::string& message) {
  if (!state_persistence_) {
    Logger::Instance().Error("WebMessageHandler", "StatePersistence not initialized");
    return false;
  }

  try {
    bool success = state_persistence_->ClearState();
    if (success) {
      Logger::Instance().Info("WebMessageHandler", "All state cleared");
    } else {
      Logger::Instance().Error("WebMessageHandler", "Failed to clear state");
    }
    
    return success;
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in HandleClearStateMessage: ") + e.what());
    return false;
  }
}

std::string WebMessageHandler::ExtractMessageType(const std::string& message) {
  try {
    auto json_obj = nlohmann::json::parse(message);
    if (json_obj.contains("type")) {
      return json_obj["type"].get<std::string>();
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in ExtractMessageType: ") + e.what());
  }
  
  return "";
}

std::string WebMessageHandler::ExtractMessageData(const std::string& message) {
  try {
    auto json_obj = nlohmann::json::parse(message);
    if (json_obj.contains("data")) {
      return json_obj["data"].dump();
    }
  } catch (const std::exception& e) {
    Logger::Instance().Error("WebMessageHandler", 
      std::string("Exception in ExtractMessageData: ") + e.what());
  }
  
  return "";
}

}  // namespace anywp_engine

