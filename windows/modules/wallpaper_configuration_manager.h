#ifndef FLUTTER_PLUGIN_WALLPAPER_CONFIGURATION_MANAGER_H_
#define FLUTTER_PLUGIN_WALLPAPER_CONFIGURATION_MANAGER_H_

#include <windows.h>
#include <string>
#include <mutex>

namespace anywp_engine {

/**
 * @brief 壁纸配置管理器
 * 
 * 统一管理壁纸的运行时配置参数：
 * - 空闲超时
 * - 内存阈值
 * - 清理间隔
 * - 自动省电设置
 * 
 * @since v2.5.0
 */
class WallpaperConfigurationManager {
 public:
  WallpaperConfigurationManager();
  ~WallpaperConfigurationManager();

  // 禁止拷贝和赋值
  WallpaperConfigurationManager(const WallpaperConfigurationManager&) = delete;
  WallpaperConfigurationManager& operator=(const WallpaperConfigurationManager&) = delete;

  // 空闲超时设置（毫秒）
  void SetIdleTimeout(DWORD timeout_ms);
  DWORD GetIdleTimeout() const;

  // 内存阈值设置（MB）
  void SetMemoryThreshold(size_t threshold_mb);
  size_t GetMemoryThreshold() const;

  // 清理间隔设置（分钟）
  void SetCleanupInterval(int interval_minutes);
  int GetCleanupInterval() const;

  // 自动省电开关
  void SetAutoPowerSavingEnabled(bool enabled);
  bool IsAutoPowerSavingEnabled() const;

  // 加载默认配置
  void LoadDefaults();

  // 从文件加载配置
  bool LoadFromFile(const std::string& config_path);

  // 保存配置到文件
  bool SaveToFile(const std::string& config_path);

  // 验证配置有效性
  bool ValidateConfig() const;

 private:
  mutable std::mutex config_mutex_;

  // 配置参数
  DWORD idle_timeout_ms_ = 5 * 60 * 1000;  // 默认 5 分钟
  size_t memory_threshold_mb_ = 150;        // 默认 150MB
  int cleanup_interval_minutes_ = 15;       // 默认 15 分钟
  bool auto_power_saving_enabled_ = true;   // 默认启用
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_WALLPAPER_CONFIGURATION_MANAGER_H_

