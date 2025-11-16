#include "input_validator.h"
#include "logger.h"
#include <algorithm>
#include <cctype>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace anywp_engine {

// Static member initialization
const std::vector<std::string> InputValidator::dangerous_protocols_ = {
    "javascript:",
    "data:",
    "vbscript:",
    // "file:" - Removed: Allow local file testing for development
    "about:",
    "res:",
    "ms-appx:",
    "ms-appx-web:"
};

const std::vector<std::string> InputValidator::allowed_protocols_ = {
    "http:",
    "https:",
    "file:",      // Allow local file URLs for development/testing
    "about:blank"
};

// ========================================
// URL Validation
// ========================================

bool InputValidator::IsValidUrl(const std::string& url) {
  if (url.empty() || url.length() > 2048) {
    return false;
  }
  
  // Check for dangerous protocols
  if (IsDangerousUrl(url)) {
    return false;
  }
  
  // Basic URL format check
  // Must contain :// or be about:blank
  if (url.find("://") == std::string::npos && url != "about:blank") {
    return false;
  }
  
  return true;
}

bool InputValidator::IsValidHttpsUrl(const std::string& url) {
  return StartsWithProtocol(url, "https:");
}

bool InputValidator::IsAllowedUrl(const std::string& url) {
  if (!IsValidUrl(url)) {
    return false;
  }
  
  // Check if URL starts with allowed protocol
  for (const auto& protocol : allowed_protocols_) {
    if (StartsWithProtocol(url, protocol)) {
      return true;
    }
  }
  
  return false;
}

bool InputValidator::IsDangerousUrl(const std::string& url) {
  std::string lower_url = url;
  std::transform(lower_url.begin(), lower_url.end(), lower_url.begin(),
      [](unsigned char c) { return std::tolower(c); });
  
  for (const auto& protocol : dangerous_protocols_) {
    if (StartsWithProtocol(lower_url, protocol)) {
      Logger::Instance().Warn("InputValidator", 
        "Dangerous protocol detected: " + protocol);
      return true;
    }
  }
  
  return false;
}

// ========================================
// Path Validation
// ========================================

bool InputValidator::IsValidPath(const std::wstring& path) {
  if (path.empty() || path.length() > MAX_PATH) {
    return false;
  }
  
  // Check for invalid characters
  const wchar_t* invalid_chars = L"<>|\"*?";
  if (path.find_first_of(invalid_chars) != std::wstring::npos) {
    return false;
  }
  
  return true;
}

bool InputValidator::IsSafePath(const std::wstring& path) {
  if (!IsValidPath(path)) {
    return false;
  }
  
  // Check for path traversal attempts
  if (path.find(L"..") != std::wstring::npos) {
    Logger::Instance().Warn("InputValidator", 
      "Path traversal attempt detected");
    return false;
  }
  
  return true;
}

bool InputValidator::IsAbsolutePath(const std::wstring& path) {
  if (path.length() < 3) {
    return false;
  }
  
  // Check for Windows absolute path (e.g., C:\)
  if (path[1] == L':' && (path[2] == L'\\' || path[2] == L'/')) {
    return true;
  }
  
  // Check for UNC path (\\server\share)
  if (path[0] == L'\\' && path[1] == L'\\') {
    return true;
  }
  
  return false;
}

std::wstring InputValidator::NormalizePath(const std::wstring& path) {
  wchar_t normalized[MAX_PATH];
  if (!PathCanonicalizeW(normalized, path.c_str())) {
    return path;
  }
  return normalized;
}

// ========================================
// Window Handle Validation
// ========================================

bool InputValidator::IsValidWindowHandle(HWND hwnd) {
  return (hwnd != nullptr && IsWindow(hwnd));
}

bool InputValidator::IsOwnedWindow(HWND hwnd) {
  if (!IsValidWindowHandle(hwnd)) {
    return false;
  }
  
  DWORD window_pid = 0;
  GetWindowThreadProcessId(hwnd, &window_pid);
  
  return (window_pid == GetCurrentProcessId());
}

bool InputValidator::IsValidParentWindow(HWND hwnd) {
  if (!IsValidWindowHandle(hwnd)) {
    return false;
  }
  
  // Check window info
  WINDOWINFO info = {0};
  info.cbSize = sizeof(WINDOWINFO);
  if (!GetWindowInfo(hwnd, &info)) {
    return false;
  }
  
  return true;
}

// ========================================
// JSON Validation
// ========================================

bool InputValidator::IsValidJson(const std::string& json) {
  if (json.empty()) {
    return false;
  }
  
  // Basic JSON validation - must start with { or [
  char first = json[0];
  char last = json[json.length() - 1];
  
  if ((first == '{' && last == '}') || (first == '[' && last == ']')) {
    // Very basic check - proper parsing would require a JSON parser
    return true;
  }
  
  return false;
}

bool InputValidator::IsJsonSizeReasonable(const std::string& json, size_t max_size) {
  return json.length() <= max_size;
}

std::string InputValidator::SanitizeJsonString(const std::string& input) {
  std::string output;
  output.reserve(input.length() + 10);
  
  for (char c : input) {
    switch (c) {
      case '"': output += "\\\""; break;
      case '\\': output += "\\\\"; break;
      case '\b': output += "\\b"; break;
      case '\f': output += "\\f"; break;
      case '\n': output += "\\n"; break;
      case '\r': output += "\\r"; break;
      case '\t': output += "\\t"; break;
      default:
        if (c >= 0 && c <= 0x1F) {
          // Control character - escape as \uXXXX
          char buf[8];
          sprintf_s(buf, "\\u%04x", (unsigned char)c);
          output += buf;
        } else {
          output += c;
        }
        break;
    }
  }
  
  return output;
}

// ========================================
// Numeric Validation
// ========================================

bool InputValidator::IsValidMonitorIndex(int index, int max_monitors) {
  return (index >= 0 && index < max_monitors);
}

bool InputValidator::IsValidSize(int size, int min_size, int max_size) {
  return (size >= min_size && size <= max_size);
}

bool InputValidator::IsValidCoordinate(int coord, int min_coord, int max_coord) {
  return (coord >= min_coord && coord <= max_coord);
}

// ========================================
// String Validation
// ========================================

bool InputValidator::IsEmptyOrWhitespace(const std::string& str) {
  return str.empty() || 
         std::all_of(str.begin(), str.end(), 
             [](unsigned char c) { return std::isspace(c); });
}

bool InputValidator::IsPrintableAscii(const std::string& str) {
  return std::all_of(str.begin(), str.end(),
      [](unsigned char c) { return c >= 32 && c <= 126; });
}

bool InputValidator::IsLengthValid(const std::string& str, size_t max_length) {
  return str.length() <= max_length;
}

std::string InputValidator::Trim(const std::string& str) {
  auto start = str.begin();
  while (start != str.end() && std::isspace(*start)) {
    start++;
  }
  
  auto end = str.end();
  do {
    end--;
  } while (std::distance(start, end) > 0 && std::isspace(*end));
  
  return std::string(start, end + 1);
}

// ========================================
// Private Helpers
// ========================================

bool InputValidator::StartsWithProtocol(const std::string& url, const std::string& protocol) {
  if (url.length() < protocol.length()) {
    return false;
  }
  
  return url.compare(0, protocol.length(), protocol) == 0;
}

}  // namespace anywp_engine

