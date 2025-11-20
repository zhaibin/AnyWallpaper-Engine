#include "iframe_detector.h"
#include "../utils/logger.h"

#include <sstream>

namespace anywp_engine {

IframeDetector::IframeDetector() {
}

IframeDetector::~IframeDetector() {
}

// ========== Public Methods ==========

void IframeDetector::UpdateIframes(const std::string& json_data) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  Logger::Instance().Debug("IframeDetector", "Parsing iframe data...");
  Logger::Instance().Debug("IframeDetector", "Raw JSON: " + json_data);
  
  // Parse and update
  std::vector<IframeInfo> new_iframes;
  if (ParseIframeJson(json_data, new_iframes)) {
    iframes_ = std::move(new_iframes);
    Logger::Instance().Info("IframeDetector", "Total iframes: " + std::to_string(iframes_.size()));
  } else {
    Logger::Instance().Error("IframeDetector", "Failed to parse iframe data");
  }
}

IframeInfo* IframeDetector::GetIframeAtPoint(int x, int y) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  Logger::Instance().Debug("IframeDetector", 
    "GetIframeAtPoint: checking (" + std::to_string(x) + "," + std::to_string(y) + ") against " + std::to_string(iframes_.size()) + " iframes");
  
  for (auto& iframe : iframes_) {
    if (!iframe.visible) {
      Logger::Instance().Debug("IframeDetector", "  " + iframe.id + " - HIDDEN");
      continue;
    }
    
    int right = iframe.left + iframe.width;
    int bottom = iframe.top + iframe.height;
    
    Logger::Instance().Debug("IframeDetector", 
      "  " + iframe.id + ": [" + std::to_string(iframe.left) + "," + std::to_string(iframe.top) + 
      "] ~ [" + std::to_string(right) + "," + std::to_string(bottom) + "]");
    
    if (x >= iframe.left && x < right &&
        y >= iframe.top && y < bottom) {
      Logger::Instance().Debug("IframeDetector", "  MATCH!");
      return &iframe;
    }
  }
  
  Logger::Instance().Debug("IframeDetector", "  No match found");
  return nullptr;
}

const std::vector<IframeInfo>& IframeDetector::GetIframes() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return iframes_;
}

void IframeDetector::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  iframes_.clear();
  Logger::Instance().Info("IframeDetector", "Cleared all iframes");
}

size_t IframeDetector::GetCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return iframes_.size();
}

// ========== Private Helpers ==========

bool IframeDetector::ParseIframeJson(const std::string& json_data, std::vector<IframeInfo>& iframes) {
  iframes.clear();
  
  // Simple JSON parsing for iframe array
  // Format: {"type":"IFRAME_DATA","iframes":[{...},{...}]}
  size_t iframes_start = json_data.find("\"iframes\":[");
  if (iframes_start == std::string::npos) {
    Logger::Instance().Debug("IframeDetector", "No iframes array found");
    return false;
  }
  
  size_t array_end = json_data.find("]", iframes_start);
  if (array_end == std::string::npos) {
    Logger::Instance().Debug("IframeDetector", "No array end found");
    return false;
  }
  
  // Find each iframe object in the array
  size_t pos = iframes_start + 11;  // Start after "iframes":[
  
  while (pos < array_end) {
    // Find next iframe object start
    pos = json_data.find("{", pos);
    if (pos == std::string::npos || pos >= array_end) break;
    
    // Find the end of this iframe object (matching closing brace)
    int brace_count = 1;
    size_t obj_end = pos + 1;
    while (obj_end < array_end && brace_count > 0) {
      if (json_data[obj_end] == '{') brace_count++;
      else if (json_data[obj_end] == '}') brace_count--;
      obj_end++;
    }
    
    if (brace_count != 0) {
      Logger::Instance().Error("IframeDetector", "Unmatched braces at pos " + std::to_string(pos));
      break;
    }
    
    // Extract iframe data within [pos, obj_end)
    std::string obj_data = json_data.substr(pos, obj_end - pos);
    Logger::Instance().Debug("IframeDetector", "Object data: " + obj_data);
    
    IframeInfo iframe;
    
    // Extract fields using helper
    iframe.id = ExtractJsonValue(obj_data, "id");
    iframe.src = ExtractJsonValue(obj_data, "src");
    iframe.click_url = ExtractJsonValue(obj_data, "clickUrl");
    
    // Extract bounds
    size_t bounds_start = obj_data.find("\"bounds\":{");
    if (bounds_start != std::string::npos) {
      // Extract left
      size_t left_start = obj_data.find("\"left\":", bounds_start);
      if (left_start != std::string::npos) {
        left_start += 7;
        iframe.left = std::stoi(obj_data.substr(left_start, 10));
      }
      
      // Extract top
      size_t top_start = obj_data.find("\"top\":", bounds_start);
      if (top_start != std::string::npos) {
        top_start += 6;
        iframe.top = std::stoi(obj_data.substr(top_start, 10));
      }
      
      // Extract width
      size_t width_start = obj_data.find("\"width\":", bounds_start);
      if (width_start != std::string::npos) {
        width_start += 8;
        iframe.width = std::stoi(obj_data.substr(width_start, 10));
      }
      
      // Extract height
      size_t height_start = obj_data.find("\"height\":", bounds_start);
      if (height_start != std::string::npos) {
        height_start += 9;
        iframe.height = std::stoi(obj_data.substr(height_start, 10));
      }
    }
    
    // Extract visible
    size_t visible_start = obj_data.find("\"visible\":");
    if (visible_start != std::string::npos) {
      visible_start += 10;
      iframe.visible = (obj_data.substr(visible_start, 4) == "true");
    } else {
      iframe.visible = true;  // Default to visible
    }
    
    // Add to list
    iframes.push_back(iframe);
    
    Logger::Instance().Debug("IframeDetector", 
      "Added iframe #" + std::to_string(iframes.size()) + ": id=" + iframe.id + 
      " pos=(" + std::to_string(iframe.left) + "," + std::to_string(iframe.top) + ")" +
      " size=" + std::to_string(iframe.width) + "x" + std::to_string(iframe.height) +
      " url=" + iframe.click_url);
    
    // Move to next object
    pos = obj_end;
  }
  
  return !iframes.empty();
}

std::string IframeDetector::ExtractJsonValue(const std::string& json, const std::string& key) {
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

// ========== v1.4.0+ Static Helpers for WallpaperInstance ==========

bool IframeDetector::UpdateIframeVector(const std::string& json_data, std::vector<IframeInfo>& target_iframes) {
  Logger::Instance().Debug("IframeDetector", "UpdateIframeVector: Parsing iframe data...");
  Logger::Instance().Debug("IframeDetector", "Raw JSON: " + json_data);
  
  target_iframes.clear();
  
  // Simple JSON parsing for iframe array
  // Format: {"type":"IFRAME_DATA","iframes":[{...},{...}]}
  size_t iframes_start = json_data.find("\"iframes\":[");
  if (iframes_start == std::string::npos) {
    Logger::Instance().Debug("IframeDetector", "No iframes array found");
    return false;
  }
  
  size_t array_end = json_data.find("]", iframes_start);
  if (array_end == std::string::npos) {
    Logger::Instance().Debug("IframeDetector", "No array end found");
    return false;
  }
  
  // Find each iframe object in the array
  size_t pos = iframes_start + 11;  // Start after "iframes":[
  
  while (pos < array_end) {
    // Find next iframe object start
    pos = json_data.find("{", pos);
    if (pos == std::string::npos || pos >= array_end) break;
    
    // Find the end of this iframe object (matching closing brace)
    int brace_count = 1;
    size_t obj_end = pos + 1;
    while (obj_end < array_end && brace_count > 0) {
      if (json_data[obj_end] == '{') brace_count++;
      else if (json_data[obj_end] == '}') brace_count--;
      obj_end++;
    }
    
    if (brace_count != 0) {
      Logger::Instance().Error("IframeDetector", "Unmatched braces at pos " + std::to_string(pos));
      break;
    }
    
    // Extract iframe data within [pos, obj_end)
    std::string obj_data = json_data.substr(pos, obj_end - pos);
    Logger::Instance().Debug("IframeDetector", "Object data: " + obj_data);
    
    IframeInfo iframe;
    
    // Extract id
    size_t id_start = obj_data.find("\"id\":\"");
    if (id_start != std::string::npos) {
      id_start += 6;
      size_t id_end = obj_data.find("\"", id_start);
      iframe.id = obj_data.substr(id_start, id_end - id_start);
    }
    
    // Extract src
    size_t src_start = obj_data.find("\"src\":\"");
    if (src_start != std::string::npos) {
      src_start += 7;
      size_t src_end = obj_data.find("\"", src_start);
      iframe.src = obj_data.substr(src_start, src_end - src_start);
    }
    
    // Extract clickUrl
    size_t url_start = obj_data.find("\"clickUrl\":\"");
    if (url_start != std::string::npos) {
      url_start += 12;
      size_t url_end = obj_data.find("\"", url_start);
      iframe.click_url = obj_data.substr(url_start, url_end - url_start);
    }
    
    // Extract bounds
    size_t bounds_start = obj_data.find("\"bounds\":{");
    if (bounds_start != std::string::npos) {
      // Extract left
      size_t left_start = obj_data.find("\"left\":", bounds_start);
      if (left_start != std::string::npos) {
        left_start += 7;
        iframe.left = std::stoi(obj_data.substr(left_start, 10));
      }
      
      // Extract top
      size_t top_start = obj_data.find("\"top\":", bounds_start);
      if (top_start != std::string::npos) {
        top_start += 6;
        iframe.top = std::stoi(obj_data.substr(top_start, 10));
      }
      
      // Extract width
      size_t width_start = obj_data.find("\"width\":", bounds_start);
      if (width_start != std::string::npos) {
        width_start += 8;
        iframe.width = std::stoi(obj_data.substr(width_start, 10));
      }
      
      // Extract height
      size_t height_start = obj_data.find("\"height\":", bounds_start);
      if (height_start != std::string::npos) {
        height_start += 9;
        iframe.height = std::stoi(obj_data.substr(height_start, 10));
      }
    }
    
    // Extract visible
    size_t visible_start = obj_data.find("\"visible\":");
    if (visible_start != std::string::npos) {
      visible_start += 10;
      iframe.visible = (obj_data.substr(visible_start, 4) == "true");
    } else {
      iframe.visible = true;  // Default to visible
    }
    
    // Add to list
    target_iframes.push_back(iframe);
    
    Logger::Instance().Debug("IframeDetector", 
      "Added iframe #" + std::to_string(target_iframes.size()) + ": id=" + iframe.id + 
      " pos=(" + std::to_string(iframe.left) + "," + std::to_string(iframe.top) + ")" +
      " size=" + std::to_string(iframe.width) + "x" + std::to_string(iframe.height) +
      " url=" + iframe.click_url);
    
    // Move to next object
    pos = obj_end;
  }
  
  Logger::Instance().Info("IframeDetector", "Total iframes: " + std::to_string(target_iframes.size()));
  return !target_iframes.empty();
}

IframeInfo* IframeDetector::GetIframeAtPointInVector(int x, int y, std::vector<IframeInfo>& iframes) {
  Logger::Instance().Debug("IframeDetector", 
    "GetIframeAtPointInVector: checking (" + std::to_string(x) + "," + std::to_string(y) + ") against " + std::to_string(iframes.size()) + " iframes");
  
  for (auto& iframe : iframes) {
    if (!iframe.visible) {
      Logger::Instance().Debug("IframeDetector", "  " + iframe.id + " - HIDDEN");
      continue;
    }
    
    int right = iframe.left + iframe.width;
    int bottom = iframe.top + iframe.height;
    
    Logger::Instance().Debug("IframeDetector", 
      "  " + iframe.id + ": [" + std::to_string(iframe.left) + "," + std::to_string(iframe.top) + 
      "] ~ [" + std::to_string(right) + "," + std::to_string(bottom) + "]");
    
    if (x >= iframe.left && x < right &&
        y >= iframe.top && y < bottom) {
      Logger::Instance().Debug("IframeDetector", "  MATCH!");
      return &iframe;
    }
  }
  
  Logger::Instance().Debug("IframeDetector", "  No match found");
  return nullptr;
}

}  // namespace anywp_engine

