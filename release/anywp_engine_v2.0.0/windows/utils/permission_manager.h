#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_PERMISSION_MANAGER_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_PERMISSION_MANAGER_H_

#include <windows.h>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <functional>

namespace anywp_engine {

// Permission types for different operations
enum class Permission {
  // Navigation permissions
  NAVIGATE_HTTP,        // Allow HTTP navigation
  NAVIGATE_HTTPS,       // Allow HTTPS navigation
  NAVIGATE_FILE,        // Allow file:// URLs
  NAVIGATE_LOCALHOST,   // Allow localhost URLs
  
  // Resource permissions
  EXECUTE_JAVASCRIPT,   // Allow JavaScript execution
  LOAD_EXTERNAL_CONTENT,// Allow loading external content (images, scripts)
  ACCESS_STORAGE,       // Allow state persistence
  ACCESS_CLIPBOARD,     // Allow clipboard access
  
  // System permissions
  MODIFY_SETTINGS,      // Allow changing plugin settings
  CREATE_WINDOWS,       // Allow creating new windows
  HOOK_MOUSE,           // Allow mouse hook
  MONITOR_DISPLAY,      // Allow display change monitoring
  
  // Special permissions
  UNRESTRICTED_ACCESS   // Full access (use with caution)
};

// Permission check result
struct PermissionCheckResult {
  bool granted = false;
  std::string reason;  // Reason for denial
  
  PermissionCheckResult() = default;
  PermissionCheckResult(bool g, const std::string& r = "") 
      : granted(g), reason(r) {}
};

// Permission policy configuration
struct PermissionPolicy {
  std::set<Permission> allowed_permissions;
  std::set<std::string> url_whitelist;      // Allowed URL patterns
  std::set<std::string> url_blacklist;      // Blocked URL patterns
  bool require_https = false;               // Require HTTPS for external URLs
  bool allow_local_files = true;            // Allow file:// URLs
  size_t max_storage_size = 100 * 1024 * 1024;  // 100MB default
  
  // Create a default policy with basic permissions
  static PermissionPolicy CreateDefault();
  
  // Create a restrictive policy
  static PermissionPolicy CreateRestrictive();
  
  // Create a permissive policy
  static PermissionPolicy CreatePermissive();
};

// Permission manager for fine-grained access control
class PermissionManager {
public:
  static PermissionManager& Instance();

  // Delete copy constructor and assignment operator
  PermissionManager(const PermissionManager&) = delete;
  PermissionManager& operator=(const PermissionManager&) = delete;

  // ========================================
  // Policy Management
  // ========================================

  // Set permission policy
  void SetPolicy(const PermissionPolicy& policy);
  
  // Get current policy
  PermissionPolicy GetPolicy() const;
  
  // Reset to default policy
  void ResetToDefault();

  // ========================================
  // Permission Checks
  // ========================================

  // Check if a permission is granted
  PermissionCheckResult CheckPermission(Permission permission) const;
  
  // Check if URL is allowed
  PermissionCheckResult CheckUrlAccess(const std::string& url) const;
  
  // Check if JavaScript execution is allowed
  PermissionCheckResult CheckJavaScriptExecution(const std::string& script) const;
  
  // Check if storage operation is allowed
  PermissionCheckResult CheckStorageAccess(size_t requested_size) const;

  // ========================================
  // Whitelist/Blacklist Management
  // ========================================

  // Add URL pattern to whitelist
  void AddUrlToWhitelist(const std::string& pattern);
  
  // Remove URL pattern from whitelist
  void RemoveUrlFromWhitelist(const std::string& pattern);
  
  // Add URL pattern to blacklist
  void AddUrlToBlacklist(const std::string& pattern);
  
  // Remove URL pattern from blacklist
  void RemoveUrlFromBlacklist(const std::string& pattern);
  
  // Clear whitelist
  void ClearWhitelist();
  
  // Clear blacklist
  void ClearBlacklist();

  // ========================================
  // Permission Granting/Revoking
  // ========================================

  // Grant a permission
  void GrantPermission(Permission permission);
  
  // Revoke a permission
  void RevokePermission(Permission permission);
  
  // Grant all permissions
  void GrantAllPermissions();
  
  // Revoke all permissions
  void RevokeAllPermissions();

  // ========================================
  // Auditing
  // ========================================

  // Log permission check (for auditing)
  void LogPermissionCheck(Permission permission, bool granted, 
                          const std::string& context = "") const;
  
  // Get permission check statistics
  struct PermissionStats {
    size_t total_checks = 0;
    size_t granted_checks = 0;
    size_t denied_checks = 0;
  };
  
  PermissionStats GetStatistics() const;
  
  // Reset statistics
  void ResetStatistics();

  // ========================================
  // Utilities
  // ========================================

  // Convert permission to string
  static std::string PermissionToString(Permission permission);
  
  // Check if URL matches pattern (supports wildcards)
  static bool MatchesPattern(const std::string& url, const std::string& pattern);

private:
  PermissionManager();
  ~PermissionManager() = default;

  mutable std::mutex mutex_;
  PermissionPolicy policy_;
  mutable PermissionStats stats_;

  // Helper: Check if URL is in blacklist
  bool IsUrlBlacklisted(const std::string& url) const;
  
  // Helper: Check if URL is in whitelist
  bool IsUrlWhitelisted(const std::string& url) const;
  
  // Helper: Extract domain from URL
  static std::string ExtractDomain(const std::string& url);
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_PERMISSION_MANAGER_H_

