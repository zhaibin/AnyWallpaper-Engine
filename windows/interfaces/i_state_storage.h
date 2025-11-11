#ifndef ANYWP_ENGINE_I_STATE_STORAGE_H_
#define ANYWP_ENGINE_I_STATE_STORAGE_H_

#include <string>
#include <vector>

namespace anywp_engine {

/**
 * @brief IStateStorage - Abstract interface for state persistence
 * 
 * Purpose: Decouples state storage operations from concrete implementation,
 *          enabling different storage backends (file, registry, cloud, etc.)
 * 
 * Thread-safe: Implementation-dependent
 */
class IStateStorage {
public:
  virtual ~IStateStorage() = default;
  
  /**
   * Save state value
   * 
   * @param key State key
   * @param value State value
   * @return true if saved successfully
   */
  virtual bool SaveState(const std::string& key, const std::string& value) = 0;
  
  /**
   * Load state value
   * 
   * @param key State key
   * @param out_value Output value
   * @return true if loaded successfully
   */
  virtual bool LoadState(const std::string& key, std::string& out_value) = 0;
  
  /**
   * Clear specific state
   * 
   * @param key State key
   * @return true if cleared successfully
   */
  virtual bool ClearState(const std::string& key) = 0;
  
  /**
   * Clear all state for current application
   * 
   * @return true if cleared successfully
   */
  virtual bool ClearAllState() = 0;
  
  /**
   * Check if state exists
   * 
   * @param key State key
   * @return true if state exists
   */
  virtual bool HasState(const std::string& key) = 0;
  
  /**
   * Get all state keys
   * 
   * @return Vector of all keys
   */
  virtual std::vector<std::string> GetAllKeys() = 0;
  
  /**
   * Set application name (for isolation)
   * 
   * @param app_name Application name
   * @return true if set successfully
   */
  virtual bool SetApplicationName(const std::string& app_name) = 0;
  
  /**
   * Get storage directory path
   * 
   * @return Storage directory path
   */
  virtual std::string GetStoragePath() const = 0;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_I_STATE_STORAGE_H_

