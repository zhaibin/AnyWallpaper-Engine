#ifndef ANYWP_ENGINE_SERVICE_LOCATOR_H_
#define ANYWP_ENGINE_SERVICE_LOCATOR_H_

#include <map>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <mutex>

namespace anywp_engine {

/**
 * @brief ServiceLocator - Simple dependency injection container
 * 
 * Purpose: Provides a centralized registry for service instances,
 *          enabling loose coupling and easier testing.
 * 
 * Features:
 * - Service registration by interface type
 * - Service resolution (get registered instance)
 * - Singleton lifecycle management
 * - Thread-safe operations
 * 
 * Usage:
 *   // Register service
 *   auto logger = std::make_shared<ConsoleLogger>();
 *   ServiceLocator::Instance().Register<ILogger>(logger);
 *   
 *   // Resolve service
 *   auto logger = ServiceLocator::Instance().Resolve<ILogger>();
 *   logger->Info("Component", "Message");
 *   
 *   // Check if service exists
 *   if (ServiceLocator::Instance().Has<ILogger>()) {
 *     // Use logger
 *   }
 * 
 * Thread-safe: Yes
 */
class ServiceLocator {
public:
  static ServiceLocator& Instance();
  
  /**
   * Register a service instance
   * 
   * @tparam TInterface Interface type
   * @param instance Service instance
   * 
   * Thread-safe: Yes
   */
  template<typename TInterface>
  void Register(std::shared_ptr<TInterface> instance) {
    std::lock_guard<std::mutex> lock(mutex_);
    services_[std::type_index(typeid(TInterface))] = instance;
  }
  
  /**
   * Resolve a service instance
   * 
   * @tparam TInterface Interface type
   * @return Service instance
   * @throws std::runtime_error if service not registered
   * 
   * Thread-safe: Yes
   */
  template<typename TInterface>
  std::shared_ptr<TInterface> Resolve() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = services_.find(std::type_index(typeid(TInterface)));
    if (it == services_.end()) {
      throw std::runtime_error(
        "Service not registered: " + std::string(typeid(TInterface).name())
      );
    }
    
    return std::static_pointer_cast<TInterface>(it->second);
  }
  
  /**
   * Try to resolve a service instance
   * 
   * @tparam TInterface Interface type
   * @param out_instance Output instance
   * @return true if service found
   * 
   * Thread-safe: Yes
   */
  template<typename TInterface>
  bool TryResolve(std::shared_ptr<TInterface>& out_instance) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = services_.find(std::type_index(typeid(TInterface)));
    if (it != services_.end()) {
      out_instance = std::static_pointer_cast<TInterface>(it->second);
      return true;
    }
    return false;
  }
  
  /**
   * Check if service is registered
   * 
   * @tparam TInterface Interface type
   * @return true if service is registered
   * 
   * Thread-safe: Yes
   */
  template<typename TInterface>
  bool Has() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return services_.find(std::type_index(typeid(TInterface))) != services_.end();
  }
  
  /**
   * Unregister a service
   * 
   * @tparam TInterface Interface type
   * 
   * Thread-safe: Yes
   */
  template<typename TInterface>
  void Unregister() {
    std::lock_guard<std::mutex> lock(mutex_);
    services_.erase(std::type_index(typeid(TInterface)));
  }
  
  /**
   * Clear all registered services
   * 
   * Thread-safe: Yes
   */
  void Clear();
  
  /**
   * Get number of registered services
   * 
   * @return Service count
   * 
   * Thread-safe: Yes
   */
  size_t GetServiceCount() const;

private:
  ServiceLocator() = default;
  ~ServiceLocator() = default;
  
  ServiceLocator(const ServiceLocator&) = delete;
  ServiceLocator& operator=(const ServiceLocator&) = delete;
  
  std::map<std::type_index, std::shared_ptr<void>> services_;
  mutable std::mutex mutex_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_SERVICE_LOCATOR_H_

