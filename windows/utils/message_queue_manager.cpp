#include "message_queue_manager.h"
#include "logger.h"

namespace anywp_engine {

MessageQueueManager::MessageQueueManager() {
  Logger::Instance().Info("MessageQueueManager", "Utility initialized");
}

MessageQueueManager::~MessageQueueManager() {
  Logger::Instance().Info("MessageQueueManager", "Utility destroyed");
}

void MessageQueueManager::EnqueueMessage(const std::string& message) {
  std::lock_guard<std::mutex> lock(message_mutex_);
  message_queue_.push(message);
  Logger::Instance().Debug("MessageQueueManager", 
    "Message enqueued (queue size: " + std::to_string(message_queue_.size()) + ")");
}

std::vector<std::string> MessageQueueManager::GetPendingMessages() {
  std::lock_guard<std::mutex> lock(message_mutex_);
  
  std::vector<std::string> messages;
  while (!message_queue_.empty()) {
    messages.push_back(message_queue_.front());
    message_queue_.pop();
  }

  if (!messages.empty()) {
    Logger::Instance().Debug("MessageQueueManager", 
      "Retrieved " + std::to_string(messages.size()) + " messages");
  }

  return messages;
}

size_t MessageQueueManager::GetMessageCount() const {
  std::lock_guard<std::mutex> lock(message_mutex_);
  return message_queue_.size();
}

void MessageQueueManager::EnqueuePowerStateChange(
    const std::string& old_state, 
    const std::string& new_state) {
  
  std::lock_guard<std::mutex> lock(power_state_mutex_);
  power_state_queue_.push({old_state, new_state});
  
  Logger::Instance().Debug("MessageQueueManager", 
    "Power state change enqueued: " + old_state + " -> " + new_state);
}

std::vector<MessageQueueManager::PowerStateChange> 
MessageQueueManager::GetPendingPowerStateChanges() {
  std::lock_guard<std::mutex> lock(power_state_mutex_);
  
  std::vector<PowerStateChange> changes;
  while (!power_state_queue_.empty()) {
    changes.push_back(power_state_queue_.front());
    power_state_queue_.pop();
  }

  if (!changes.empty()) {
    Logger::Instance().Debug("MessageQueueManager", 
      "Retrieved " + std::to_string(changes.size()) + " power state changes");
  }

  return changes;
}

size_t MessageQueueManager::GetPowerStateChangeCount() const {
  std::lock_guard<std::mutex> lock(power_state_mutex_);
  return power_state_queue_.size();
}

void MessageQueueManager::ClearMessages() {
  std::lock_guard<std::mutex> lock(message_mutex_);
  std::queue<std::string>().swap(message_queue_);
  Logger::Instance().Info("MessageQueueManager", "Message queue cleared");
}

void MessageQueueManager::ClearPowerStateChanges() {
  std::lock_guard<std::mutex> lock(power_state_mutex_);
  std::queue<PowerStateChange>().swap(power_state_queue_);
  Logger::Instance().Info("MessageQueueManager", "Power state queue cleared");
}

void MessageQueueManager::ClearAll() {
  ClearMessages();
  ClearPowerStateChanges();
  Logger::Instance().Info("MessageQueueManager", "All queues cleared");
}

}  // namespace anywp_engine

