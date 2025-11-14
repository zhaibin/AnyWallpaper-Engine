#define NOMINMAX
#include "permission_manager.h"
#include "logger.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace anywp_engine {

// ========================================
// PermissionPolicy Static Methods
// ========================================

PermissionPolicy PermissionPolicy::CreateDefault() {
  PermissionPolicy policy;
  policy.allowed_permissions = {
    Permission::NAVIGATE_HTTP,
    Permission::NAVIGATE_HTTPS,
    Permission::NAVIGATE_FILE,
    Permission::NAVIGATE_LOCALHOST,
    Permission::EXECUTE_JAVASCRIPT,
    Permission::LOAD_EXTERNAL_CONTENT,
    Permission::ACCESS_STORAGE,
    Permission::MODIFY_SETTINGS,
    Permission::CREATE_WINDOWS,
    Permission::HOOK_MOUSE,
    Permission::MONITOR_DISPLAY
  };
  policy.require_https = false;
  policy.allow_local_files = true;
  policy.max_storage_size = 100 * 1024 * 1024;  // 100MB
  return policy;
}

PermissionPolicy PermissionPolicy::CreateRestrictive() {
  PermissionPolicy policy;
  policy.allowed_permissions = {
    Permission::NAVIGATE_HTTPS,  // Only HTTPS
    Permission::EXECUTE_JAVASCRIPT,
    Permission::ACCESS_STORAGE
  };
  policy.require_https = true;
  policy.allow_local_files = false;
  policy.max_storage_size = 10 * 1024 * 1024;  // 10MB
  return policy;
}

PermissionPolicy PermissionPolicy::CreatePermissive() {
  PermissionPolicy policy;
  policy.allowed_permissions = {
    Permission::NAVIGATE_HTTP,
    Permission::NAVIGATE_HTTPS,
    Permission::NAVIGATE_FILE,
    Permission::NAVIGATE_LOCALHOST,
    Permission::EXECUTE_JAVASCRIPT,
    Permission::LOAD_EXTERNAL_CONTENT,
    Permission::ACCESS_STORAGE,
    Permission::ACCESS_CLIPBOARD,
    Permission::MODIFY_SETTINGS,
    Permission::CREATE_WINDOWS,
    Permission::HOOK_MOUSE,
    Permission::MONITOR_DISPLAY,
    Permission::UNRESTRICTED_ACCESS
  };
  policy.require_https = false;
  policy.allow_local_files = true;
  policy.max_storage_size = 500 * 1024 * 1024;  // 500MB
  return policy;
}

// ========================================
// PermissionManager Implementation
// ========================================

PermissionManager::PermissionManager() {
  policy_ = PermissionPolicy::CreateDefault();
}

PermissionManager& PermissionManager::Instance() {
  static PermissionManager instance;
  return instance;
}

void PermissionManager::SetPolicy(const PermissionPolicy& policy) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_ = policy;
  Logger::Instance().Info("PermissionManager", 
    "Policy updated, " + std::to_string(policy.allowed_permissions.size()) + " permissions granted");
}

PermissionPolicy PermissionManager::GetPolicy() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return policy_;
}

void PermissionManager::ResetToDefault() {
  SetPolicy(PermissionPolicy::CreateDefault());
}

PermissionCheckResult PermissionManager::CheckPermission(Permission permission) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  stats_.total_checks++;
  
  // Check if unrestricted access is granted
  if (policy_.allowed_permissions.count(Permission::UNRESTRICTED_ACCESS) > 0) {
    stats_.granted_checks++;
    return PermissionCheckResult(true, "Unrestricted access granted");
  }
  
  // Check specific permission
  bool granted = policy_.allowed_permissions.count(permission) > 0;
  
  if (granted) {
    stats_.granted_checks++;
  } else {
    stats_.denied_checks++;
  }
  
  std::string reason = granted ? "" : 
    "Permission " + PermissionToString(permission) + " not granted";
  
  return PermissionCheckResult(granted, reason);
}

PermissionCheckResult PermissionManager::CheckUrlAccess(const std::string& url) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  stats_.total_checks++;
  
  if (url.empty()) {
    stats_.denied_checks++;
    return PermissionCheckResult(false, "Empty URL");
  }
  
  // Check unrestricted access
  if (policy_.allowed_permissions.count(Permission::UNRESTRICTED_ACCESS) > 0) {
    stats_.granted_checks++;
    return PermissionCheckResult(true, "Unrestricted access");
  }
  
  // Check blacklist first
  if (IsUrlBlacklisted(url)) {
    stats_.denied_checks++;
    return PermissionCheckResult(false, "URL is blacklisted");
  }
  
  // Check whitelist if not empty
  if (!policy_.url_whitelist.empty() && !IsUrlWhitelisted(url)) {
    stats_.denied_checks++;
    return PermissionCheckResult(false, "URL not in whitelist");
  }
  
  // Check protocol-specific permissions
  std::string lower_url = url;
  std::transform(lower_url.begin(), lower_url.end(), lower_url.begin(),
      [](unsigned char c) { return std::tolower(c); });
  
  if (lower_url.find("https://") == 0) {
    bool granted = policy_.allowed_permissions.count(Permission::NAVIGATE_HTTPS) > 0;
    if (granted) stats_.granted_checks++;
    else stats_.denied_checks++;
    return PermissionCheckResult(granted, 
      granted ? "" : "HTTPS navigation not permitted");
  }
  
  if (lower_url.find("http://") == 0) {
    // Check HTTPS requirement
    if (policy_.require_https) {
      stats_.denied_checks++;
      return PermissionCheckResult(false, "HTTPS required, HTTP not allowed");
    }
    
    // Check localhost
    if (lower_url.find("http://localhost") == 0 || 
        lower_url.find("http://127.0.0.1") == 0) {
      bool granted = policy_.allowed_permissions.count(Permission::NAVIGATE_LOCALHOST) > 0;
      if (granted) stats_.granted_checks++;
      else stats_.denied_checks++;
      return PermissionCheckResult(granted, 
        granted ? "" : "Localhost navigation not permitted");
    }
    
    bool granted = policy_.allowed_permissions.count(Permission::NAVIGATE_HTTP) > 0;
    if (granted) stats_.granted_checks++;
    else stats_.denied_checks++;
    return PermissionCheckResult(granted, 
      granted ? "" : "HTTP navigation not permitted");
  }
  
  if (lower_url.find("file://") == 0) {
    if (!policy_.allow_local_files) {
      stats_.denied_checks++;
      return PermissionCheckResult(false, "Local file access not allowed by policy");
    }
    
    bool granted = policy_.allowed_permissions.count(Permission::NAVIGATE_FILE) > 0;
    if (granted) stats_.granted_checks++;
    else stats_.denied_checks++;
    return PermissionCheckResult(granted, 
      granted ? "" : "File navigation not permitted");
  }
  
  // Unknown protocol
  stats_.denied_checks++;
  return PermissionCheckResult(false, "Unknown or unsupported protocol");
}

PermissionCheckResult PermissionManager::CheckJavaScriptExecution(
    const std::string& script) const {
  
  auto perm_result = CheckPermission(Permission::EXECUTE_JAVASCRIPT);
  if (!perm_result.granted) {
    return perm_result;
  }
  
  // Additional checks could be added here for script content
  // For now, just check the permission
  return PermissionCheckResult(true);
}

PermissionCheckResult PermissionManager::CheckStorageAccess(size_t requested_size) const {
  auto perm_result = CheckPermission(Permission::ACCESS_STORAGE);
  if (!perm_result.granted) {
    return perm_result;
  }
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (requested_size > policy_.max_storage_size) {
    return PermissionCheckResult(false, 
      "Requested size exceeds maximum storage limit");
  }
  
  return PermissionCheckResult(true);
}

void PermissionManager::AddUrlToWhitelist(const std::string& pattern) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.url_whitelist.insert(pattern);
  Logger::Instance().Info("PermissionManager", "Added to whitelist: " + pattern);
}

void PermissionManager::RemoveUrlFromWhitelist(const std::string& pattern) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.url_whitelist.erase(pattern);
}

void PermissionManager::AddUrlToBlacklist(const std::string& pattern) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.url_blacklist.insert(pattern);
  Logger::Instance().Warn("PermissionManager", "Added to blacklist: " + pattern);
}

void PermissionManager::RemoveUrlFromBlacklist(const std::string& pattern) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.url_blacklist.erase(pattern);
}

void PermissionManager::ClearWhitelist() {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.url_whitelist.clear();
}

void PermissionManager::ClearBlacklist() {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.url_blacklist.clear();
}

void PermissionManager::GrantPermission(Permission permission) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.allowed_permissions.insert(permission);
  Logger::Instance().Info("PermissionManager", 
    "Granted permission: " + PermissionToString(permission));
}

void PermissionManager::RevokePermission(Permission permission) {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.allowed_permissions.erase(permission);
  Logger::Instance().Warn("PermissionManager", 
    "Revoked permission: " + PermissionToString(permission));
}

void PermissionManager::GrantAllPermissions() {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.allowed_permissions = {
    Permission::NAVIGATE_HTTP,
    Permission::NAVIGATE_HTTPS,
    Permission::NAVIGATE_FILE,
    Permission::NAVIGATE_LOCALHOST,
    Permission::EXECUTE_JAVASCRIPT,
    Permission::LOAD_EXTERNAL_CONTENT,
    Permission::ACCESS_STORAGE,
    Permission::ACCESS_CLIPBOARD,
    Permission::MODIFY_SETTINGS,
    Permission::CREATE_WINDOWS,
    Permission::HOOK_MOUSE,
    Permission::MONITOR_DISPLAY,
    Permission::UNRESTRICTED_ACCESS
  };
  Logger::Instance().Info("PermissionManager", "Granted all permissions");
}

void PermissionManager::RevokeAllPermissions() {
  std::lock_guard<std::mutex> lock(mutex_);
  policy_.allowed_permissions.clear();
  Logger::Instance().Warn("PermissionManager", "Revoked all permissions");
}

void PermissionManager::LogPermissionCheck(Permission permission, bool granted, 
                                            const std::string& context) const {
  std::string result = granted ? "GRANTED" : "DENIED";
  std::string msg = "Permission check: " + PermissionToString(permission) + 
                    " -> " + result;
  if (!context.empty()) {
    msg += " (context: " + context + ")";
  }
  
  if (granted) {
    Logger::Instance().Debug("PermissionManager", msg);
  } else {
    Logger::Instance().Warn("PermissionManager", msg);
  }
}

PermissionManager::PermissionStats PermissionManager::GetStatistics() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stats_;
}

void PermissionManager::ResetStatistics() {
  std::lock_guard<std::mutex> lock(mutex_);
  stats_ = PermissionStats();
}

std::string PermissionManager::PermissionToString(Permission permission) {
  switch (permission) {
    case Permission::NAVIGATE_HTTP: return "NAVIGATE_HTTP";
    case Permission::NAVIGATE_HTTPS: return "NAVIGATE_HTTPS";
    case Permission::NAVIGATE_FILE: return "NAVIGATE_FILE";
    case Permission::NAVIGATE_LOCALHOST: return "NAVIGATE_LOCALHOST";
    case Permission::EXECUTE_JAVASCRIPT: return "EXECUTE_JAVASCRIPT";
    case Permission::LOAD_EXTERNAL_CONTENT: return "LOAD_EXTERNAL_CONTENT";
    case Permission::ACCESS_STORAGE: return "ACCESS_STORAGE";
    case Permission::ACCESS_CLIPBOARD: return "ACCESS_CLIPBOARD";
    case Permission::MODIFY_SETTINGS: return "MODIFY_SETTINGS";
    case Permission::CREATE_WINDOWS: return "CREATE_WINDOWS";
    case Permission::HOOK_MOUSE: return "HOOK_MOUSE";
    case Permission::MONITOR_DISPLAY: return "MONITOR_DISPLAY";
    case Permission::UNRESTRICTED_ACCESS: return "UNRESTRICTED_ACCESS";
    default: return "UNKNOWN";
  }
}

bool PermissionManager::MatchesPattern(const std::string& url, const std::string& pattern) {
  // Simple wildcard matching: * matches any characters
  // For more complex patterns, could use regex
  
  if (pattern == "*") {
    return true;  // Match everything
  }
  
  // Check if pattern is a simple substring match
  if (pattern.find('*') == std::string::npos) {
    return url.find(pattern) != std::string::npos;
  }
  
  // Handle wildcard at end: "https://example.com/*"
  if (pattern.back() == '*') {
    std::string prefix = pattern.substr(0, pattern.length() - 1);
    return url.compare(0, prefix.length(), prefix) == 0;
  }
  
  // Handle wildcard at start: "*.example.com"
  if (pattern.front() == '*') {
    std::string suffix = pattern.substr(1);
    if (url.length() >= suffix.length()) {
      return url.compare(url.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    return false;
  }
  
  // For complex patterns, just do substring match
  return url.find(pattern) != std::string::npos;
}

bool PermissionManager::IsUrlBlacklisted(const std::string& url) const {
  for (const auto& pattern : policy_.url_blacklist) {
    if (MatchesPattern(url, pattern)) {
      return true;
    }
  }
  return false;
}

bool PermissionManager::IsUrlWhitelisted(const std::string& url) const {
  for (const auto& pattern : policy_.url_whitelist) {
    if (MatchesPattern(url, pattern)) {
      return true;
    }
  }
  return false;
}

std::string PermissionManager::ExtractDomain(const std::string& url) {
  // Simple domain extraction
  size_t start = url.find("://");
  if (start == std::string::npos) {
    return "";
  }
  start += 3;
  
  size_t end = url.find('/', start);
  if (end == std::string::npos) {
    end = url.length();
  }
  
  return url.substr(start, end - start);
}

}  // namespace anywp_engine

