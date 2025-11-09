#ifndef ANYWP_ENGINE_IFRAME_DETECTOR_H_
#define ANYWP_ENGINE_IFRAME_DETECTOR_H_

#include <string>
#include <vector>
#include <mutex>

namespace anywp_engine {

/**
 * IframeInfo - Information about an iframe element
 */
struct IframeInfo {
  std::string id;
  std::string src;
  std::string click_url;
  int left;
  int top;
  int width;
  int height;
  bool visible;
};

/**
 * IframeDetector - Detect and manage iframe click regions
 * 
 * Features:
 * - Parse iframe data from WebView JavaScript
 * - Hit-test for iframe at specific coordinates
 * - Support for multiple iframes
 * - Thread-safe operations
 * 
 * Use Case: Detect clicks on advertisement iframes
 */
class IframeDetector {
public:
  IframeDetector();
  ~IframeDetector();

  // Update iframe data from JSON
  void UpdateIframes(const std::string& json_data);
  
  // v1.4.0+: Static helper to update external iframe vector (for WallpaperInstance)
  static bool UpdateIframeVector(const std::string& json_data, std::vector<IframeInfo>& target_iframes);
  
  // Hit-test: find iframe at point (screen coordinates)
  IframeInfo* GetIframeAtPoint(int x, int y);
  
  // v1.4.0+: Static helper to find iframe in external vector
  static IframeInfo* GetIframeAtPointInVector(int x, int y, std::vector<IframeInfo>& iframes);
  
  // Get all iframes
  const std::vector<IframeInfo>& GetIframes() const;
  
  // Clear all iframes
  void Clear();
  
  // Get count
  size_t GetCount() const;

private:
  // Parse JSON data into IframeInfo structures
  bool ParseIframeJson(const std::string& json_data, std::vector<IframeInfo>& iframes);
  
  // Extract JSON field value
  std::string ExtractJsonValue(const std::string& json, const std::string& key);
  
  std::vector<IframeInfo> iframes_;
  mutable std::mutex mutex_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_IFRAME_DETECTOR_H_

