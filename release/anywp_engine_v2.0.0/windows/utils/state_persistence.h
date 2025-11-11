#ifndef ANYWP_ENGINE_STATE_PERSISTENCE_H_
#define ANYWP_ENGINE_STATE_PERSISTENCE_H_

#include <string>
#include <map>
#include <mutex>

namespace anywp_engine {

/**
 * StatePersistence - Application-level state storage with isolation
 * 
 * Features:
 * - Key-value storage persisted to JSON file
 * - Application-level isolation (each app has separate storage)
 * - Thread-safe operations
 * - Automatic directory creation
 * 
 * Storage Path: %LOCALAPPDATA%\AnyWPEngine\[AppName]\state.json
 */
class StatePersistence {
public:
  StatePersistence();
  ~StatePersistence();

  // Application identity management
  void SetApplicationName(const std::string& name);
  std::string GetApplicationName() const;
  std::string GetStoragePath() const;

  // State operations
  bool SaveState(const std::string& key, const std::string& value);
  std::string LoadState(const std::string& key);
  bool ClearState();
  
  // Batch operations
  std::map<std::string, std::string> LoadAllStates();
  bool SaveAllStates(const std::map<std::string, std::string>& states);

private:
  // Internal helpers
  std::string GetAppDataPath() const;
  bool EnsureDirectoryExists(const std::string& path);
  
  // JSON helpers
  std::string EscapeJson(const std::string& str);
  std::string UnescapeJson(const std::string& str);
  
  // File I/O
  std::map<std::string, std::string> LoadStateFile();
  bool SaveStateFile(const std::map<std::string, std::string>& state);

  // State management
  std::string application_name_;
  std::map<std::string, std::string> state_cache_;
  mutable std::mutex mutex_;
};

// ========== v1.4.1+ Phase B: Standalone utility functions ==========
// These functions can be used independently without creating a StatePersistence instance

// Get application-specific storage path
std::string GetAppDataPathForApp(const std::string& app_name = "Default");

// Ensure directory exists (creates if needed)
bool EnsureDirectoryExistsUtil(const std::string& path);

// JSON string escaping/unescaping
std::string EscapeJsonString(const std::string& str);
std::string UnescapeJsonString(const std::string& str);

// Load/save entire state file
std::map<std::string, std::string> LoadStateFileForApp(const std::string& app_name);
bool SaveStateFileForApp(const std::map<std::string, std::string>& state, const std::string& app_name);

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_STATE_PERSISTENCE_H_

