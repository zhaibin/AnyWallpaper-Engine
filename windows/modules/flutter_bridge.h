#ifndef FLUTTER_PLUGIN_ANYWP_ENGINE_FLUTTER_BRIDGE_H_
#define FLUTTER_PLUGIN_ANYWP_ENGINE_FLUTTER_BRIDGE_H_

#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

namespace anywp_engine {

// Forward declaration
class AnyWPEnginePlugin;

// Flutter method bridge - handles all Flutter method channel communication
class FlutterBridge {
public:
  // Method handler callback type
  using MethodHandler = std::function<void(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)>;

  explicit FlutterBridge(AnyWPEnginePlugin* plugin);
  ~FlutterBridge();

  // Disable copy
  FlutterBridge(const FlutterBridge&) = delete;
  FlutterBridge& operator=(const FlutterBridge&) = delete;

  // Main method call dispatcher
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Register all method handlers
  void RegisterAllHandlers();

private:
  // Register a single handler
  void RegisterHandler(const std::string& method_name, MethodHandler handler);

  // ========================================
  // Wallpaper Control Methods
  // ========================================
  
  void HandleInitializeWallpaper(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleStopWallpaper(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleNavigateToUrl(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Multi-Monitor Methods
  // ========================================
  
  void HandleGetMonitors(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleInitializeWallpaperOnMonitor(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleStopWallpaperOnMonitor(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleNavigateToUrlOnMonitor(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Power Management Methods
  // ========================================
  
  void HandlePauseWallpaper(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleResumeWallpaper(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleSetAutoPowerSaving(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleGetPowerState(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Memory Management Methods
  // ========================================
  
  void HandleGetMemoryUsage(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleOptimizeMemory(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Configuration Methods
  // ========================================
  
  void HandleSetIdleTimeout(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleSetMemoryThreshold(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleSetCleanupInterval(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleGetConfiguration(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // State Persistence Methods
  // ========================================
  
  void HandleSaveState(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleLoadState(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleClearState(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleSetApplicationName(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  void HandleGetStoragePath(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Utility Methods
  // ========================================
  
  void HandleGetVersion(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Message Communication Methods
  // ========================================
  
  void HandleSendMessage(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // ========================================
  // Helper Methods
  // ========================================
  
  // Validate required argument exists
  bool ValidateArgument(
      const flutter::EncodableMap* args,
      const std::string& key,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result);
  
  // Get string argument with validation
  bool GetStringArgument(
      const flutter::EncodableMap* args,
      const std::string& key,
      std::string& out_value,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result);
  
  // Get int argument with validation
  bool GetIntArgument(
      const flutter::EncodableMap* args,
      const std::string& key,
      int& out_value,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>& result);
  
  // Get bool argument with default value
  bool GetBoolArgument(
      const flutter::EncodableMap* args,
      const std::string& key,
      bool default_value = false);

  // Plugin reference (not owned)
  AnyWPEnginePlugin* plugin_;
  
  // Method handler registry
  std::unordered_map<std::string, MethodHandler> handlers_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_ANYWP_ENGINE_FLUTTER_BRIDGE_H_

