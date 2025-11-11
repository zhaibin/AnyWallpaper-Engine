#ifndef ANYWP_ENGINE_EVENT_BUS_H_
#define ANYWP_ENGINE_EVENT_BUS_H_

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <any>
#include <chrono>

namespace anywp_engine {

/**
 * @brief Event data structure
 */
struct Event {
  std::string type;                          // Event type (e.g., "wallpaper.initialized")
  std::map<std::string, std::any> data;      // Event data payload
  std::chrono::system_clock::time_point timestamp;
  std::string source;                        // Event source module
  
  Event(const std::string& event_type, const std::string& event_source = "")
    : type(event_type), 
      source(event_source),
      timestamp(std::chrono::system_clock::now()) {}
  
  // Helper to get typed data
  template<typename T>
  T GetData(const std::string& key, const T& default_value = T()) const {
    auto it = data.find(key);
    if (it != data.end()) {
      try {
        return std::any_cast<T>(it->second);
      } catch (const std::bad_any_cast&) {
        return default_value;
      }
    }
    return default_value;
  }
  
  // Helper to set data
  template<typename T>
  void SetData(const std::string& key, const T& value) {
    data[key] = value;
  }
};

/**
 * @brief Event handler callback type
 */
using EventHandler = std::function<void(const Event&)>;

/**
 * @brief Event subscription handle for unsubscribing
 */
class EventSubscription {
public:
  EventSubscription(int id, const std::string& event_type)
    : id_(id), event_type_(event_type) {}
  
  int GetId() const { return id_; }
  const std::string& GetEventType() const { return event_type_; }
  
private:
  int id_;
  std::string event_type_;
};

/**
 * @brief EventBus - Decoupled event-driven communication system
 * 
 * Features:
 * - Subscribe to specific event types
 * - Publish events to all subscribers
 * - Unsubscribe using subscription handle
 * - Event filtering and prioritization
 * - Thread-safe operations
 * - Event history tracking
 * 
 * Usage:
 *   // Subscribe to events
 *   auto sub = EventBus::Instance().Subscribe("wallpaper.initialized", 
 *     [](const Event& e) {
 *       std::cout << "Wallpaper initialized on monitor " 
 *                 << e.GetData<int>("monitor_index", -1) << std::endl;
 *     });
 *   
 *   // Publish event
 *   Event event("wallpaper.initialized", "WindowManager");
 *   event.SetData("monitor_index", 0);
 *   event.SetData("success", true);
 *   EventBus::Instance().Publish(event);
 *   
 *   // Unsubscribe
 *   EventBus::Instance().Unsubscribe(sub);
 * 
 * Common Event Types:
 * - "wallpaper.initialized" - Wallpaper initialization completed
 * - "wallpaper.stopped" - Wallpaper stopped
 * - "wallpaper.navigation.started" - Navigation started
 * - "wallpaper.navigation.completed" - Navigation completed
 * - "monitor.changed" - Monitor configuration changed
 * - "webview.created" - WebView created
 * - "webview.error" - WebView error occurred
 * - "state.saved" - State saved successfully
 * - "state.loaded" - State loaded successfully
 * - "permission.denied" - Permission check failed
 * - "error.occurred" - Generic error
 * 
 * Thread-safe: Yes
 */
class EventBus {
public:
  static EventBus& Instance();
  
  /**
   * Subscribe to an event type
   * 
   * @param event_type Event type to listen for
   * @param handler Callback function to invoke when event occurs
   * @param priority Higher priority handlers are invoked first (default: 0)
   * @return Subscription handle for unsubscribing
   * 
   * Thread-safe: Yes
   */
  std::shared_ptr<EventSubscription> Subscribe(
    const std::string& event_type,
    EventHandler handler,
    int priority = 0
  );
  
  /**
   * Unsubscribe from events
   * 
   * @param subscription Subscription handle from Subscribe()
   * 
   * Thread-safe: Yes
   */
  void Unsubscribe(std::shared_ptr<EventSubscription> subscription);
  
  /**
   * Publish an event to all subscribers
   * 
   * @param event Event to publish
   * 
   * Thread-safe: Yes
   */
  void Publish(const Event& event);
  
  /**
   * Publish event with simplified API
   * 
   * @param event_type Event type
   * @param source Source module (optional)
   * 
   * Thread-safe: Yes
   */
  void Publish(const std::string& event_type, const std::string& source = "");
  
  /**
   * Get event history (last N events)
   * 
   * @param count Number of recent events to retrieve (default: 100)
   * @return Vector of recent events
   * 
   * Thread-safe: Yes
   */
  std::vector<Event> GetHistory(size_t count = 100) const;
  
  /**
   * Clear event history
   * 
   * Thread-safe: Yes
   */
  void ClearHistory();
  
  /**
   * Enable/disable event history tracking
   * 
   * @param enabled true to enable history tracking
   * @param max_size Maximum history size (default: 1000)
   * 
   * Thread-safe: Yes
   */
  void SetHistoryTracking(bool enabled, size_t max_size = 1000);
  
  /**
   * Get subscription count for an event type
   * 
   * @param event_type Event type
   * @return Number of active subscriptions
   * 
   * Thread-safe: Yes
   */
  size_t GetSubscriberCount(const std::string& event_type) const;
  
  /**
   * Get all active event types
   * 
   * @return Vector of event types with active subscriptions
   * 
   * Thread-safe: Yes
   */
  std::vector<std::string> GetActiveEventTypes() const;
  
  /**
   * Remove all subscriptions
   * 
   * Thread-safe: Yes
   */
  void Clear();

private:
  EventBus();
  ~EventBus();
  
  EventBus(const EventBus&) = delete;
  EventBus& operator=(const EventBus&) = delete;
  
  struct Subscriber {
    int id;
    EventHandler handler;
    int priority;
    
    bool operator<(const Subscriber& other) const {
      return priority > other.priority;  // Higher priority first
    }
  };
  
  int next_subscription_id_;
  std::map<std::string, std::vector<Subscriber>> subscribers_;
  
  // Event history
  bool history_enabled_;
  size_t max_history_size_;
  std::vector<Event> event_history_;
  
  mutable std::mutex mutex_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_EVENT_BUS_H_

