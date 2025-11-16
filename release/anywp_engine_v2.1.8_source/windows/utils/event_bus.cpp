#include "event_bus.h"
#include "logger.h"
#include <algorithm>

namespace anywp_engine {

EventBus& EventBus::Instance() {
  static EventBus instance;
  return instance;
}

EventBus::EventBus()
    : next_subscription_id_(1),
      history_enabled_(false),
      max_history_size_(1000) {
  Logger::Instance().Info("EventBus", "EventBus initialized");
}

EventBus::~EventBus() {
  Clear();
}

std::shared_ptr<EventSubscription> EventBus::Subscribe(
    const std::string& event_type,
    EventHandler handler,
    int priority) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  int subscription_id = next_subscription_id_++;
  
  Subscriber subscriber;
  subscriber.id = subscription_id;
  subscriber.handler = std::move(handler);
  subscriber.priority = priority;
  
  subscribers_[event_type].push_back(subscriber);
  
  // Sort by priority
  std::sort(subscribers_[event_type].begin(), subscribers_[event_type].end());
  
  Logger::Instance().Debug("EventBus", 
    "Subscribed to '" + event_type + "' (ID: " + std::to_string(subscription_id) + 
    ", Priority: " + std::to_string(priority) + ")");
  
  return std::make_shared<EventSubscription>(subscription_id, event_type);
}

void EventBus::Unsubscribe(std::shared_ptr<EventSubscription> subscription) {
  if (!subscription) {
    return;
  }
  
  std::lock_guard<std::mutex> lock(mutex_);
  
  const std::string& event_type = subscription->GetEventType();
  int subscription_id = subscription->GetId();
  
  auto it = subscribers_.find(event_type);
  if (it != subscribers_.end()) {
    auto& subs = it->second;
    subs.erase(
      std::remove_if(subs.begin(), subs.end(),
        [subscription_id](const Subscriber& s) { return s.id == subscription_id; }),
      subs.end()
    );
    
    if (subs.empty()) {
      subscribers_.erase(it);
    }
    
    Logger::Instance().Debug("EventBus", 
      "Unsubscribed from '" + event_type + "' (ID: " + std::to_string(subscription_id) + ")");
  }
}

void EventBus::Publish(const Event& event) {
  std::vector<Subscriber> handlers_to_call;
  
  {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Add to history if enabled
    if (history_enabled_) {
      event_history_.push_back(event);
      if (event_history_.size() > max_history_size_) {
        event_history_.erase(event_history_.begin());
      }
    }
    
    // Get all subscribers for this event type
    auto it = subscribers_.find(event.type);
    if (it != subscribers_.end()) {
      handlers_to_call = it->second;
    }
  }
  
  // Call handlers outside of lock to avoid deadlocks
  for (const auto& subscriber : handlers_to_call) {
    try {
      subscriber.handler(event);
    } catch (const std::exception& e) {
      Logger::Instance().Error("EventBus", 
        "Exception in event handler for '" + event.type + "': " + e.what());
    } catch (...) {
      Logger::Instance().Error("EventBus", 
        "Unknown exception in event handler for '" + event.type + "'");
    }
  }
  
  Logger::Instance().Debug("EventBus", 
    "Published event '" + event.type + "' from '" + event.source + 
    "' to " + std::to_string(handlers_to_call.size()) + " subscribers");
}

void EventBus::Publish(const std::string& event_type, const std::string& source) {
  Event event(event_type, source);
  Publish(event);
}

std::vector<Event> EventBus::GetHistory(size_t count) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!history_enabled_ || event_history_.empty()) {
    return {};
  }
  
  size_t start_index = event_history_.size() > count ? 
                       event_history_.size() - count : 0;
  
  return std::vector<Event>(
    event_history_.begin() + start_index,
    event_history_.end()
  );
}

void EventBus::ClearHistory() {
  std::lock_guard<std::mutex> lock(mutex_);
  event_history_.clear();
  Logger::Instance().Debug("EventBus", "Event history cleared");
}

void EventBus::SetHistoryTracking(bool enabled, size_t max_size) {
  std::lock_guard<std::mutex> lock(mutex_);
  history_enabled_ = enabled;
  max_history_size_ = max_size;
  
  if (!enabled) {
    event_history_.clear();
  } else {
    event_history_.reserve(max_size);
  }
  
  Logger::Instance().Info("EventBus", 
    std::string("Event history tracking ") + (enabled ? "enabled" : "disabled") +
    " (max size: " + std::to_string(max_size) + ")");
}

size_t EventBus::GetSubscriberCount(const std::string& event_type) const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = subscribers_.find(event_type);
  if (it != subscribers_.end()) {
    return it->second.size();
  }
  return 0;
}

std::vector<std::string> EventBus::GetActiveEventTypes() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<std::string> types;
  types.reserve(subscribers_.size());
  
  for (const auto& pair : subscribers_) {
    if (!pair.second.empty()) {
      types.push_back(pair.first);
    }
  }
  
  return types;
}

void EventBus::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  subscribers_.clear();
  event_history_.clear();
  Logger::Instance().Info("EventBus", "All subscriptions cleared");
}

}  // namespace anywp_engine

