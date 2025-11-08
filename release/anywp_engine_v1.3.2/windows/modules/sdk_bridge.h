#ifndef ANYWP_ENGINE_SDK_BRIDGE_H_
#define ANYWP_ENGINE_SDK_BRIDGE_H_

#include <WebView2.h>
#include <wrl.h>
#include <string>
#include <functional>
#include <map>

namespace anywp_engine {

/**
 * SDKBridge - JavaScript SDK injection and message handling
 * 
 * Features:
 * - Inject JavaScript SDK into WebView
 * - Setup bidirectional message bridge
 * - Handle messages from web content
 * - Execute scripts in WebView
 * - Type-safe message handlers
 * 
 * Message Types:
 * - IFRAME_DATA: iframe click regions
 * - OPEN_URL: open external URL
 * - READY: wallpaper initialization complete
 * - LOG: console.log forwarding
 * - saveState/loadState/clearState: state persistence
 */
class SDKBridge {
public:
  // Message handler callback type
  using MessageHandler = std::function<void(const std::string& message)>;

  SDKBridge();
  ~SDKBridge();

  // WebView management
  void SetWebView(Microsoft::WRL::ComPtr<ICoreWebView2> webview);
  Microsoft::WRL::ComPtr<ICoreWebView2> GetWebView() const;

  // SDK injection
  void InjectSDK();
  void SetupMessageBridge();

  // Message handling
  void RegisterHandler(const std::string& message_type, MessageHandler handler);
  void UnregisterHandler(const std::string& message_type);
  void HandleMessage(const std::string& message);

  // Script execution
  bool ExecuteScript(const std::wstring& script);
  bool ExecuteScript(const std::string& script);

  // Utility
  std::string ExtractJsonValue(const std::string& json, const std::string& key);

private:
  std::string LoadSDKScript();
  std::string GetMessageType(const std::string& message);

  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  std::map<std::string, MessageHandler> handlers_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_SDK_BRIDGE_H_

