#include "iframe_detector.h"

#include <iostream>
#include <sstream>

namespace anywp_engine {

IframeDetector::IframeDetector() {
}

IframeDetector::~IframeDetector() {
}

// ========== Public Methods ==========

void IframeDetector::UpdateIframes(const std::string& json_data) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::cout << "[AnyWP] [iframe] Parsing iframe data..." << std::endl;
  std::cout << "[AnyWP] [iframe] Raw JSON: " << json_data << std::endl;
  
  // Parse and update
  std::vector<IframeInfo> new_iframes;
  if (ParseIframeJson(json_data, new_iframes)) {
    iframes_ = std::move(new_iframes);
    std::cout << "[AnyWP] [iframe] Total iframes: " << iframes_.size() << std::endl;
  } else {
    std::cout << "[AnyWP] [iframe] ERROR: Failed to parse iframe data" << std::endl;
  }
}

IframeInfo* IframeDetector::GetIframeAtPoint(int x, int y) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::cout << "[AnyWP] [iframe] GetIframeAtPoint: checking (" << x << "," << y << ") against " 
            << iframes_.size() << " iframes" << std::endl;
  
  for (auto& iframe : iframes_) {
    if (!iframe.visible) {
      std::cout << "[AnyWP] [iframe]   " << iframe.id << " - HIDDEN" << std::endl;
      continue;
    }
    
    int right = iframe.left + iframe.width;
    int bottom = iframe.top + iframe.height;
    
    std::cout << "[AnyWP] [iframe]   " << iframe.id << ": [" << iframe.left << "," << iframe.top 
              << "] ~ [" << right << "," << bottom << "]" << std::endl;
    
    if (x >= iframe.left && x < right &&
        y >= iframe.top && y < bottom) {
      std::cout << "[AnyWP] [iframe]   MATCH!" << std::endl;
      return &iframe;
    }
  }
  
  std::cout << "[AnyWP] [iframe]   No match found" << std::endl;
  return nullptr;
}

const std::vector<IframeInfo>& IframeDetector::GetIframes() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return iframes_;
}

void IframeDetector::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  iframes_.clear();
  std::cout << "[AnyWP] [iframe] Cleared all iframes" << std::endl;
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
    std::cout << "[AnyWP] [iframe] No iframes array found" << std::endl;
    return false;
  }
  
  size_t array_end = json_data.find("]", iframes_start);
  if (array_end == std::string::npos) {
    std::cout << "[AnyWP] [iframe] No array end found" << std::endl;
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
      std::cout << "[AnyWP] [iframe] ERROR: Unmatched braces at pos " << pos << std::endl;
      break;
    }
    
    // Extract iframe data within [pos, obj_end)
    std::string obj_data = json_data.substr(pos, obj_end - pos);
    std::cout << "[AnyWP] [iframe] Object data: " << obj_data << std::endl;
    
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
    
    std::cout << "[AnyWP] [iframe] Added iframe #" << iframes.size() << ": id=" << iframe.id 
              << " pos=(" << iframe.left << "," << iframe.top << ")"
              << " size=" << iframe.width << "x" << iframe.height
              << " url=" << iframe.click_url << std::endl;
    
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
  std::cout << "[AnyWP] [iframe] UpdateIframeVector: Parsing iframe data..." << std::endl;
  std::cout << "[AnyWP] [iframe] Raw JSON: " << json_data << std::endl;
  
  target_iframes.clear();
  
  // Simple JSON parsing for iframe array
  // Format: {"type":"IFRAME_DATA","iframes":[{...},{...}]}
  size_t iframes_start = json_data.find("\"iframes\":[");
  if (iframes_start == std::string::npos) {
    std::cout << "[AnyWP] [iframe] No iframes array found" << std::endl;
    return false;
  }
  
  size_t array_end = json_data.find("]", iframes_start);
  if (array_end == std::string::npos) {
    std::cout << "[AnyWP] [iframe] No array end found" << std::endl;
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
      std::cout << "[AnyWP] [iframe] ERROR: Unmatched braces at pos " << pos << std::endl;
      break;
    }
    
    // Extract iframe data within [pos, obj_end)
    std::string obj_data = json_data.substr(pos, obj_end - pos);
    std::cout << "[AnyWP] [iframe] Object data: " << obj_data << std::endl;
    
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
    
    std::cout << "[AnyWP] [iframe] Added iframe #" << target_iframes.size() << ": id=" << iframe.id 
              << " pos=(" << iframe.left << "," << iframe.top << ")"
              << " size=" << iframe.width << "x" << iframe.height
              << " url=" << iframe.click_url << std::endl;
    
    // Move to next object
    pos = obj_end;
  }
  
  std::cout << "[AnyWP] [iframe] Total iframes: " << target_iframes.size() << std::endl;
  return !target_iframes.empty();
}

IframeInfo* IframeDetector::GetIframeAtPointInVector(int x, int y, std::vector<IframeInfo>& iframes) {
  std::cout << "[AnyWP] [iframe] GetIframeAtPointInVector: checking (" << x << "," << y << ") against " 
            << iframes.size() << " iframes" << std::endl;
  
  for (auto& iframe : iframes) {
    if (!iframe.visible) {
      std::cout << "[AnyWP] [iframe]   " << iframe.id << " - HIDDEN" << std::endl;
      continue;
    }
    
    int right = iframe.left + iframe.width;
    int bottom = iframe.top + iframe.height;
    
    std::cout << "[AnyWP] [iframe]   " << iframe.id << ": [" << iframe.left << "," << iframe.top 
              << "] ~ [" << right << "," << bottom << "]" << std::endl;
    
    if (x >= iframe.left && x < right &&
        y >= iframe.top && y < bottom) {
      std::cout << "[AnyWP] [iframe]   MATCH!" << std::endl;
      return &iframe;
    }
  }
  
  std::cout << "[AnyWP] [iframe]   No match found" << std::endl;
  return nullptr;
}

}  // namespace anywp_engine

