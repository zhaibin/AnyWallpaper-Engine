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

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_STATE_PERSISTENCE_H_

