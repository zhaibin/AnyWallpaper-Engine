#include "state_persistence.h"
#include "logger.h"  // v1.4.1+ Phase B: Use Logger instead of std::cout

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <direct.h>

#include <windows.h>
#include <shlobj.h>
#include <combaseapi.h>

namespace anywp_engine {

StatePersistence::StatePersistence() 
    : application_name_("Default") {
}

StatePersistence::~StatePersistence() {
}

// ========== Application Identity Management ==========

void StatePersistence::SetApplicationName(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (name.empty()) {
    std::cout << "[AnyWP] [State] WARNING: Empty application name, using 'Default'" << std::endl;
    application_name_ = "Default";
    return;
  }
  
  // Sanitize application name (remove invalid filename characters)
  std::string sanitized_name;
  for (char c : name) {
    if (isalnum(c) || c == '_' || c == '-') {
      sanitized_name += c;
    } else if (c == ' ') {
      sanitized_name += '_';
    }
  }
  
  if (sanitized_name.empty()) {
    std::cout << "[AnyWP] [State] WARNING: Invalid application name, using 'Default'" << std::endl;
    application_name_ = "Default";
    return;
  }
  
  // Clear cache when switching applications
  if (application_name_ != sanitized_name && !state_cache_.empty()) {
    std::cout << "[AnyWP] [State] Switching from '" << application_name_ 
              << "' to '" << sanitized_name << "', clearing cache" << std::endl;
    state_cache_.clear();
  }
  
  application_name_ = sanitized_name;
  std::cout << "[AnyWP] [State] Application name set to: " << application_name_ << std::endl;
}

std::string StatePersistence::GetApplicationName() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return application_name_;
}

std::string StatePersistence::GetStoragePath() const {
  return GetAppDataPath();
}

// ========== State Operations ==========

bool StatePersistence::SaveState(const std::string& key, const std::string& value) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    // Update in-memory cache
    state_cache_[key] = value;
    
    // Save to file
    bool success = SaveStateFile(state_cache_);
    
    if (success) {
      std::cout << "[AnyWP] [State] Saved to file (" << application_name_ 
                << "): " << key << " = " << value << std::endl;
    } else {
      std::cout << "[AnyWP] [State] ERROR: Failed to save state to file" << std::endl;
    }
    
    return success;
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [State] ERROR: Exception in SaveState: " << e.what() << std::endl;
    return false;
  }
}

std::string StatePersistence::LoadState(const std::string& key) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Check cache first
  auto it = state_cache_.find(key);
  if (it != state_cache_.end()) {
    std::cout << "[AnyWP] [State] Loaded from cache (" << application_name_ 
              << "): " << key << " = " << it->second << std::endl;
    return it->second;
  }
  
  // Load all state from file if cache is empty
  if (state_cache_.empty()) {
    try {
      state_cache_ = LoadStateFile();
    } catch (const std::exception& e) {
      std::cout << "[AnyWP] [State] ERROR: Exception in LoadStateFile: " << e.what() << std::endl;
    }
  }
  
  // Check cache again after loading
  it = state_cache_.find(key);
  if (it != state_cache_.end()) {
    std::cout << "[AnyWP] [State] Loaded from file (" << application_name_ 
              << "): " << key << " = " << it->second << std::endl;
    return it->second;
  }
  
  std::cout << "[AnyWP] [State] Key not found (" << application_name_ << "): " << key << std::endl;
  return "";
}

bool StatePersistence::ClearState() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  try {
    // Clear in-memory cache
    state_cache_.clear();
    
    // Delete state file
    std::string app_data = GetAppDataPath();
    if (app_data.empty()) {
      std::cout << "[AnyWP] [State] ERROR: Failed to get app data path" << std::endl;
      return false;
    }
    
    std::string state_file = app_data + "\\state.json";
    
    // Delete file if exists
    if (DeleteFileA(state_file.c_str()) || GetLastError() == ERROR_FILE_NOT_FOUND) {
      std::cout << "[AnyWP] [State] Cleared all state (" << application_name_ 
                << ") (deleted file: " << state_file << ")" << std::endl;
      return true;
    } else {
      std::cout << "[AnyWP] [State] ERROR: Failed to delete state file: " << GetLastError() << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [State] ERROR: Exception in ClearState: " << e.what() << std::endl;
    return false;
  }
}

std::map<std::string, std::string> StatePersistence::LoadAllStates() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (state_cache_.empty()) {
    try {
      state_cache_ = LoadStateFile();
    } catch (const std::exception& e) {
      std::cout << "[AnyWP] [State] ERROR: Exception in LoadAllStates: " << e.what() << std::endl;
    }
  }
  
  return state_cache_;
}

bool StatePersistence::SaveAllStates(const std::map<std::string, std::string>& states) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  state_cache_ = states;
  return SaveStateFile(state_cache_);
}

// ========== Internal Helpers ==========

std::string StatePersistence::GetAppDataPath() const {
  wchar_t* path = nullptr;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path);
  
  if (SUCCEEDED(hr)) {
    // Convert wchar_t* to std::string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], size_needed, nullptr, nullptr);
    CoTaskMemFree(path);
    
    // Append AnyWPEngine directory and app-specific subdirectory
    // Path: %LOCALAPPDATA%\AnyWPEngine\[AppName]
    result += "\\AnyWPEngine\\" + application_name_;
    return result;
  }
  
  return "";
}

bool StatePersistence::EnsureDirectoryExists(const std::string& path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    // Directory doesn't exist, create it (including parent directories)
    // First create parent directory
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
      std::string parent = path.substr(0, pos);
      if (!EnsureDirectoryExists(parent)) {
        return false;
      }
    }
    return _mkdir(path.c_str()) == 0;
  }
  return (info.st_mode & S_IFDIR) != 0;
}

// ========== JSON Helpers ==========

std::string StatePersistence::EscapeJson(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  
  for (char c : str) {
    switch (c) {
      case '"':  result += "\\\""; break;
      case '\\': result += "\\\\"; break;
      case '\b': result += "\\b"; break;
      case '\f': result += "\\f"; break;
      case '\n': result += "\\n"; break;
      case '\r': result += "\\r"; break;
      case '\t': result += "\\t"; break;
      default:
        if (c < 0x20) {
          // Control character - encode as \uXXXX
          char buf[7];
          sprintf_s(buf, "\\u%04x", static_cast<unsigned char>(c));
          result += buf;
        } else {
          result += c;
        }
    }
  }
  
  return result;
}

std::string StatePersistence::UnescapeJson(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '\\' && i + 1 < str.size()) {
      switch (str[i + 1]) {
        case '"':  result += '"'; i++; break;
        case '\\': result += '\\'; i++; break;
        case 'b':  result += '\b'; i++; break;
        case 'f':  result += '\f'; i++; break;
        case 'n':  result += '\n'; i++; break;
        case 'r':  result += '\r'; i++; break;
        case 't':  result += '\t'; i++; break;
        case 'u':  // Unicode escape sequence
          if (i + 5 < str.size()) {
            // Simple implementation - just skip it for now
            i += 5;
          }
          break;
        default:
          result += str[i];
      }
    } else {
      result += str[i];
    }
  }
  
  return result;
}

// ========== File I/O ==========

std::map<std::string, std::string> StatePersistence::LoadStateFile() {
  std::map<std::string, std::string> state;
  
  std::string app_data = GetAppDataPath();
  if (app_data.empty()) {
    std::cout << "[AnyWP] [State] ERROR: Failed to get app data path" << std::endl;
    return state;
  }
  
  std::string state_file = app_data + "\\state.json";
  std::ifstream file(state_file);
  
  if (!file.is_open()) {
    std::cout << "[AnyWP] [State] State file not found (first run): " << state_file << std::endl;
    return state;
  }
  
  // Read entire file
  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();
  
  if (content.empty() || content == "{}") {
    return state;
  }
  
  // Simple JSON parser for key-value pairs
  // Format: {"key1": "value1", "key2": "value2"}
  size_t pos = 0;
  while (pos < content.size()) {
    // Find key start
    size_t key_start = content.find('"', pos);
    if (key_start == std::string::npos) break;
    key_start++;
    
    // Find key end
    size_t key_end = content.find('"', key_start);
    if (key_end == std::string::npos) break;
    
    std::string key = content.substr(key_start, key_end - key_start);
    
    // Find value start
    size_t value_start = content.find('"', key_end + 1);
    if (value_start == std::string::npos) break;
    value_start++;
    
    // Find value end (handle escaped quotes)
    size_t value_end = value_start;
    while (value_end < content.size()) {
      if (content[value_end] == '"' && (value_end == 0 || content[value_end - 1] != '\\')) {
        break;
      }
      value_end++;
    }
    
    if (value_end == std::string::npos) break;
    
    std::string value = content.substr(value_start, value_end - value_start);
    value = UnescapeJson(value);
    
    state[key] = value;
    pos = value_end + 1;
  }
  
  std::cout << "[AnyWP] [State] Loaded " << state.size() << " entries from file" << std::endl;
  return state;
}

bool StatePersistence::SaveStateFile(const std::map<std::string, std::string>& state) {
  std::string app_data = GetAppDataPath();
  if (app_data.empty()) {
    std::cout << "[AnyWP] [State] ERROR: Failed to get app data path" << std::endl;
    return false;
  }
  
  // Ensure directory exists
  if (!EnsureDirectoryExists(app_data)) {
    std::cout << "[AnyWP] [State] ERROR: Failed to create directory: " << app_data << std::endl;
    return false;
  }
  
  std::string state_file = app_data + "\\state.json";
  std::ofstream file(state_file);
  
  if (!file.is_open()) {
    std::cout << "[AnyWP] [State] ERROR: Failed to open file: " << state_file << std::endl;
    return false;
  }
  
  // Write JSON
  file << "{\n";
  
  size_t count = 0;
  for (const auto& pair : state) {
    if (count > 0) file << ",\n";
    file << "  \"" << EscapeJson(pair.first) << "\": \"" << EscapeJson(pair.second) << "\"";
    count++;
  }
  
  file << "\n}\n";
  file.close();
  
  std::cout << "[AnyWP] [State] Saved " << state.size() << " entries to file: " << state_file << std::endl;
  return true;
}

// ========== v1.4.1+ Phase B: Standalone utility functions ==========

std::string GetAppDataPathForApp(const std::string& app_name) {
  wchar_t* path = nullptr;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path);
  
  if (SUCCEEDED(hr)) {
    // Convert wchar_t* to std::string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], size_needed, nullptr, nullptr);
    CoTaskMemFree(path);
    
    // Append AnyWPEngine directory and app-specific subdirectory
    // Path: %LOCALAPPDATA%\AnyWPEngine\[AppName]
    result += "\\AnyWPEngine\\" + app_name;
    return result;
  }
  
  return "";
}

bool EnsureDirectoryExistsUtil(const std::string& path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    // Directory doesn't exist, create it (including parent directories)
    // First create parent directory
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
      std::string parent = path.substr(0, pos);
      if (!EnsureDirectoryExistsUtil(parent)) {
        return false;
      }
    }
    return _mkdir(path.c_str()) == 0;
  }
  return (info.st_mode & S_IFDIR) != 0;
}

std::string EscapeJsonString(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  
  for (char c : str) {
    switch (c) {
      case '"':  result += "\\\""; break;
      case '\\': result += "\\\\"; break;
      case '\b': result += "\\b"; break;
      case '\f': result += "\\f"; break;
      case '\n': result += "\\n"; break;
      case '\r': result += "\\r"; break;
      case '\t': result += "\\t"; break;
      default:
        if (c < 0x20) {
          // Control character - encode as \uXXXX
          char buf[7];
          sprintf_s(buf, "\\u%04x", static_cast<unsigned char>(c));
          result += buf;
        } else {
          result += c;
        }
    }
  }
  
  return result;
}

std::string UnescapeJsonString(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '\\' && i + 1 < str.size()) {
      switch (str[i + 1]) {
        case '"':  result += '"'; i++; break;
        case '\\': result += '\\'; i++; break;
        case 'b':  result += '\b'; i++; break;
        case 'f':  result += '\f'; i++; break;
        case 'n':  result += '\n'; i++; break;
        case 'r':  result += '\r'; i++; break;
        case 't':  result += '\t'; i++; break;
        case 'u':  // Unicode escape sequence
          if (i + 5 < str.size()) {
            // Simple implementation - just skip it for now
            i += 5;
          }
          break;
        default:
          result += str[i];
      }
    } else {
      result += str[i];
    }
  }
  
  return result;
}

std::map<std::string, std::string> LoadStateFileForApp(const std::string& app_name) {
  std::map<std::string, std::string> state;
  
  std::string app_data = GetAppDataPathForApp(app_name);
  if (app_data.empty()) {
    Logger::Instance().Error("StatePersistence", "Failed to get app data path");
    return state;
  }
  
  std::string state_file = app_data + "\\state.json";
  std::ifstream file(state_file);
  
  if (!file.is_open()) {
    Logger::Instance().Info("StatePersistence", "State file not found (first run): " + state_file);
    return state;
  }
  
  // Read entire file
  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();
  
  if (content.empty() || content == "{}") {
    return state;
  }
  
  // Simple JSON parser for key-value pairs
  // Format: {"key1": "value1", "key2": "value2"}
  size_t pos = 0;
  while (pos < content.size()) {
    // Find key start
    size_t key_start = content.find('"', pos);
    if (key_start == std::string::npos) break;
    key_start++;
    
    // Find key end
    size_t key_end = content.find('"', key_start);
    if (key_end == std::string::npos) break;
    
    std::string key = content.substr(key_start, key_end - key_start);
    
    // Find value start
    size_t value_start = content.find('"', key_end + 1);
    if (value_start == std::string::npos) break;
    value_start++;
    
    // Find value end (handle escaped quotes)
    size_t value_end = value_start;
    while (value_end < content.size()) {
      if (content[value_end] == '"' && (value_end == 0 || content[value_end - 1] != '\\')) {
        break;
      }
      value_end++;
    }
    
    if (value_end == std::string::npos) break;
    
    std::string value = content.substr(value_start, value_end - value_start);
    value = UnescapeJsonString(value);
    
    state[key] = value;
    pos = value_end + 1;
  }
  
  Logger::Instance().Info("StatePersistence", "Loaded " + std::to_string(state.size()) + " entries from file");
  return state;
}

bool SaveStateFileForApp(const std::map<std::string, std::string>& state, const std::string& app_name) {
  std::string app_data = GetAppDataPathForApp(app_name);
  if (app_data.empty()) {
    Logger::Instance().Error("StatePersistence", "Failed to get app data path");
    return false;
  }
  
  // Ensure directory exists
  if (!EnsureDirectoryExistsUtil(app_data)) {
    Logger::Instance().Error("StatePersistence", "Failed to create directory: " + app_data);
    return false;
  }
  
  std::string state_file = app_data + "\\state.json";
  std::ofstream file(state_file);
  
  if (!file.is_open()) {
    Logger::Instance().Error("StatePersistence", "Failed to open file: " + state_file);
    return false;
  }
  
  // Write JSON
  file << "{\n";
  
  size_t count = 0;
  for (const auto& pair : state) {
    if (count > 0) file << ",\n";
    file << "  \"" << EscapeJsonString(pair.first) << "\": \"" << EscapeJsonString(pair.second) << "\"";
    count++;
  }
  
  file << "\n}\n";
  file.close();
  
  Logger::Instance().Info("StatePersistence", "Saved " + std::to_string(state.size()) + " entries to file: " + state_file);
  return true;
}

}  // namespace anywp_engine

