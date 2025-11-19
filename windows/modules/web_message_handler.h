#ifndef FLUTTER_PLUGIN_WEB_MESSAGE_HANDLER_H_
#define FLUTTER_PLUGIN_WEB_MESSAGE_HANDLER_H_

#include <string>
#include <functional>
#include <memory>
#include <map>

namespace anywp_engine {

// Forward declarations
class StatePersistence;
struct WallpaperInstance;

/**
 * @brief WebView2 消息处理器
 * 
 * 统一处理所有从 WebView2 接收的消息，包括：
 * - iframe 数据同步
 * - URL 打开请求
 * - 就绪通知
 * - 日志消息
 * - 状态持久化请求
 * 
 * @since v2.5.0
 */
class WebMessageHandler {
 public:
  /**
   * @brief 消息处理回调类型定义
   * 
   * 参数：
   * - message: 原始消息内容
   * - instance: 相关的壁纸实例（可能为 nullptr）
   */
  using MessageCallback = std::function<void(const std::string& message, WallpaperInstance* instance)>;
  
  /**
   * @brief URL 打开回调类型
   * 
   * 参数：
   * - url: 要打开的 URL
   */
  using UrlOpenCallback = std::function<void(const std::string& url)>;
  
  /**
   * @brief 就绪通知回调类型
   */
  using ReadyCallback = std::function<void()>;
  
  /**
   * @brief 日志回调类型
   * 
   * 参数：
   * - level: 日志级别（debug/info/warning/error）
   * - message: 日志内容
   */
  using LogCallback = std::function<void(const std::string& level, const std::string& message)>;

  WebMessageHandler();
  ~WebMessageHandler();

  // 禁止拷贝和赋值
  WebMessageHandler(const WebMessageHandler&) = delete;
  WebMessageHandler& operator=(const WebMessageHandler&) = delete;

  /**
   * @brief 初始化消息处理器
   * 
   * @param state_persistence 状态持久化模块指针
   */
  void Initialize(StatePersistence* state_persistence);

  /**
   * @brief 处理接收到的 Web 消息（主入口）
   * 
   * 根据消息类型路由到对应的处理器
   * 
   * @param message JSON 格式的消息内容
   * @param instance 发送消息的壁纸实例
   * @return true 如果消息被成功处理
   */
  bool HandleMessage(const std::string& message, WallpaperInstance* instance = nullptr);

  /**
   * @brief 设置 iframe 数据回调
   */
  void SetIframeDataCallback(MessageCallback callback);

  /**
   * @brief 设置 URL 打开回调
   */
  void SetUrlOpenCallback(UrlOpenCallback callback);

  /**
   * @brief 设置就绪通知回调
   */
  void SetReadyCallback(ReadyCallback callback);

  /**
   * @brief 设置日志回调
   */
  void SetLogCallback(LogCallback callback);

 private:
  // 消息类型处理器
  bool HandleIframeDataMessage(const std::string& message, WallpaperInstance* instance);
  bool HandleOpenUrlMessage(const std::string& message);
  bool HandleReadyMessage(const std::string& message);
  bool HandleLogMessage(const std::string& message);
  bool HandleConsoleLogMessage(const std::string& message);
  bool HandleSaveStateMessage(const std::string& message);
  bool HandleLoadStateMessage(const std::string& message);
  bool HandleClearStateMessage(const std::string& message);

  // 辅助方法
  std::string ExtractMessageType(const std::string& message);
  std::string ExtractMessageData(const std::string& message);

  // 回调函数
  MessageCallback iframe_data_callback_;
  UrlOpenCallback url_open_callback_;
  ReadyCallback ready_callback_;
  LogCallback log_callback_;

  // 状态持久化模块（不拥有所有权）
  StatePersistence* state_persistence_ = nullptr;

  // 统计信息
  std::map<std::string, size_t> message_counters_;  // 每种消息类型的处理计数
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_WEB_MESSAGE_HANDLER_H_

