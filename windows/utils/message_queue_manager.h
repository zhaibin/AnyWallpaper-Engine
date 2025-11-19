#ifndef FLUTTER_PLUGIN_MESSAGE_QUEUE_MANAGER_H_
#define FLUTTER_PLUGIN_MESSAGE_QUEUE_MANAGER_H_

#include <string>
#include <queue>
#include <mutex>
#include <vector>

namespace anywp_engine {

/**
 * @brief 消息队列管理器（工具类）
 * 
 * 管理从 C++ 到 Dart 的消息队列：
 * - JavaScript 消息队列
 * - 电源状态变化队列
 * - 线程安全的队列操作
 * 
 * @since v2.5.0
 */
class MessageQueueManager {
 public:
  /**
   * @brief 电源状态变化结构
   */
  struct PowerStateChange {
    std::string old_state;
    std::string new_state;
  };

  MessageQueueManager();
  ~MessageQueueManager();

  // 禁止拷贝和赋值
  MessageQueueManager(const MessageQueueManager&) = delete;
  MessageQueueManager& operator=(const MessageQueueManager&) = delete;

  // JavaScript 消息队列
  void EnqueueMessage(const std::string& message);
  std::vector<std::string> GetPendingMessages();
  size_t GetMessageCount() const;

  // 电源状态变化队列
  void EnqueuePowerStateChange(const std::string& old_state, const std::string& new_state);
  std::vector<PowerStateChange> GetPendingPowerStateChanges();
  size_t GetPowerStateChangeCount() const;

  // 清空队列
  void ClearMessages();
  void ClearPowerStateChanges();
  void ClearAll();

 private:
  std::queue<std::string> message_queue_;
  std::mutex message_mutex_;

  std::queue<PowerStateChange> power_state_queue_;
  std::mutex power_state_mutex_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_MESSAGE_QUEUE_MANAGER_H_

