#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_INPUT_VALIDATOR_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_INPUT_VALIDATOR_H_

#include <windows.h>
#include <string>
#include <vector>
#include <regex>

namespace anywp_engine {

// Input validation utilities for security and stability
class InputValidator {
public:
  // ========================================
  // URL Validation
  // ========================================
  
  // Check if URL is valid and well-formed
  static bool IsValidUrl(const std::string& url);
  
  // Check if URL uses HTTPS protocol
  static bool IsValidHttpsUrl(const std::string& url);
  
  // Check if URL is allowed (checks protocol and basic format)
  static bool IsAllowedUrl(const std::string& url);
  
  // Check if URL contains potentially dangerous protocols
  static bool IsDangerousUrl(const std::string& url);
  
  // ========================================
  // Path Validation
  // ========================================
  
  // Check if Windows path is valid
  static bool IsValidPath(const std::wstring& path);
  
  // Check if path is safe (no path traversal attempts)
  static bool IsSafePath(const std::wstring& path);
  
  // Check if path is absolute (not relative)
  static bool IsAbsolutePath(const std::wstring& path);
  
  // Normalize path (resolve .., ., etc.)
  static std::wstring NormalizePath(const std::wstring& path);
  
  // ========================================
  // Window Handle Validation
  // ========================================
  
  // Check if HWND is valid
  static bool IsValidWindowHandle(HWND hwnd);
  
  // Check if window belongs to current process
  static bool IsOwnedWindow(HWND hwnd);
  
  // Check if window is suitable as parent window
  static bool IsValidParentWindow(HWND hwnd);
  
  // ========================================
  // JSON Validation
  // ========================================
  
  // Check if string is valid JSON (basic check)
  static bool IsValidJson(const std::string& json);
  
  // Check if JSON size is reasonable
  static bool IsJsonSizeReasonable(const std::string& json, size_t max_size = 1024 * 1024);
  
  // Sanitize JSON string (escape special characters)
  static std::string SanitizeJsonString(const std::string& input);
  
  // ========================================
  // Numeric Validation
  // ========================================
  
  // Check if monitor index is valid
  static bool IsValidMonitorIndex(int index, int max_monitors);
  
  // Check if size is valid (width/height)
  static bool IsValidSize(int size, int min_size = 1, int max_size = 10000);
  
  // Check if coordinate is valid
  static bool IsValidCoordinate(int coord, int min_coord = -10000, int max_coord = 10000);
  
  // ========================================
  // String Validation
  // ========================================
  
  // Check if string is empty or whitespace only
  static bool IsEmptyOrWhitespace(const std::string& str);
  
  // Check if string contains only printable ASCII characters
  static bool IsPrintableAscii(const std::string& str);
  
  // Check if string length is within limits
  static bool IsLengthValid(const std::string& str, size_t max_length);
  
  // Trim whitespace from string
  static std::string Trim(const std::string& str);
  
private:
  // Dangerous URL protocols that should be blocked
  static const std::vector<std::string> dangerous_protocols_;
  
  // Allowed URL protocols
  static const std::vector<std::string> allowed_protocols_;
  
  // Helper: Check if URL starts with protocol
  static bool StartsWithProtocol(const std::string& url, const std::string& protocol);
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_INPUT_VALIDATOR_H_

