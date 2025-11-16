#include "service_locator.h"
#include "logger.h"

namespace anywp_engine {

ServiceLocator& ServiceLocator::Instance() {
  static ServiceLocator instance;
  return instance;
}

void ServiceLocator::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  services_.clear();
  Logger::Instance().Info("ServiceLocator", "All services cleared");
}

size_t ServiceLocator::GetServiceCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return services_.size();
}

}  // namespace anywp_engine

