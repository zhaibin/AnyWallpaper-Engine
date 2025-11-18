// v2.3.2+: Lifecycle management implementation
// This file contains implementation of lifecycle helper methods

#include "anywp_engine_plugin.h"
#include "utils/logger.h"

namespace anywp_engine {

// v2.3.2+: Get active wallpaper instance count
// Returns the total number of active wallpaper instances (single + multi-monitor)
size_t AnyWPEnginePlugin::GetActiveInstanceCount() const {
  size_t count = 0;
  
  // Count multi-monitor instances
  {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(instances_mutex_));
    count += wallpaper_instances_.size();
  }
  
  // Count legacy single-monitor instance
  if (webview_host_hwnd_ != nullptr && is_initialized_) {
    count++;
  }
  
  return count;
}

}  // namespace anywp_engine

