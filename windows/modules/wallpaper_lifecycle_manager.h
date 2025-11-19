#ifndef FLUTTER_PLUGIN_WALLPAPER_LIFECYCLE_MANAGER_H_
#define FLUTTER_PLUGIN_WALLPAPER_LIFECYCLE_MANAGER_H_

#include <string>
#include <functional>
#include <atomic>
#include <vector>
#include <WebView2.h>

namespace anywp_engine {

// Forward declarations
struct WallpaperInstance;
class MemoryOptimizer;

/**
 * @brief 壁纸生命周期管理器
 * 
 * 管理壁纸的完整生命周期：
 * - 暂停（省电、锁屏、全屏应用）
 * - 恢复（重新激活）
 * - 验证（检查窗口状态）
 * - 通知（可见性变化）
 * 
 * @since v2.5.0
 */
class WallpaperLifecycleManager {
 public:
  /**
   * @brief 壁纸状态枚举
   */
  enum class WallpaperState {
    ACTIVE,      // 正常运行
    PAUSED,      // 已暂停
    RESUMING,    // 恢复中
    DESTROYED    // 已销毁
  };

  /**
   * @brief 状态变化回调
   * 
   * 参数：
   * - old_state: 旧状态
   * - new_state: 新状态
   * - reason: 状态变化原因
   */
  using StateChangeCallback = std::function<void(
    WallpaperState old_state, 
    WallpaperState new_state, 
    const std::string& reason
  )>;

  /**
   * @brief 窗口验证回调
   * 
   * 返回：窗口是否有效
   */
  using WindowValidationCallback = std::function<bool()>;

  /**
   * @brief 配置恢复回调
   * 
   * 参数：
   * - url: 要恢复的 URL
   * - log_tag: 日志标签
   * 
   * 返回：是否成功恢复
   */
  using ConfigurationRestoreCallback = std::function<bool(
    const std::string& url, 
    const std::string& log_tag
  )>;

  WallpaperLifecycleManager();
  ~WallpaperLifecycleManager();

  // 禁止拷贝和赋值
  WallpaperLifecycleManager(const WallpaperLifecycleManager&) = delete;
  WallpaperLifecycleManager& operator=(const WallpaperLifecycleManager&) = delete;

  /**
   * @brief 初始化管理器
   * 
   * @param memory_optimizer 内存优化器指针（可选）
   */
  void Initialize(MemoryOptimizer* memory_optimizer = nullptr);

  /**
   * @brief 暂停壁纸
   * 
   * @param reason 暂停原因（如 "screen_off", "locked", "fullscreen"）
   * @return true 如果成功暂停
   */
  bool PauseWallpaper(const std::string& reason);

  /**
   * @brief 恢复壁纸
   * 
   * @param reason 恢复原因（如 "screen_on", "unlocked"）
   * @param force_reinit 是否强制重新初始化
   * @return true 如果成功恢复
   */
  bool ResumeWallpaper(const std::string& reason, bool force_reinit = false);

  /**
   * @brief 验证壁纸窗口状态
   * 
   * 检查所有壁纸窗口是否有效
   * 
   * @return true 如果所有窗口都有效
   */
  bool ValidateWallpaperWindows();

  /**
   * @brief 恢复壁纸配置
   * 
   * @param url 要恢复的 URL
   * @param log_tag 日志标签（用于跟踪）
   * @return true 如果成功恢复
   */
  bool RestoreWallpaperConfiguration(const std::string& url, const std::string& log_tag = "Lifecycle");

  /**
   * @brief 通知 Web 内容可见性变化
   * 
   * @param visible 是否可见
   */
  void NotifyWebContentVisibility(bool visible);

  /**
   * @brief 执行脚本到所有壁纸实例
   * 
   * @param script 要执行的 JavaScript 脚本（宽字符）
   */
  void ExecuteScriptToAllInstances(const std::wstring& script);

  /**
   * @brief 获取当前壁纸状态
   */
  WallpaperState GetCurrentState() const;

  /**
   * @brief 检查壁纸是否已暂停
   */
  bool IsPaused() const;

  /**
   * @brief 设置状态变化回调
   */
  void SetStateChangeCallback(StateChangeCallback callback);

  /**
   * @brief 设置窗口验证回调
   */
  void SetWindowValidationCallback(WindowValidationCallback callback);

  /**
   * @brief 设置配置恢复回调
   */
  void SetConfigurationRestoreCallback(ConfigurationRestoreCallback callback);

  /**
   * @brief 设置壁纸实例列表引用
   * 
   * @param instances 壁纸实例列表指针
   */
  void SetWallpaperInstances(std::vector<WallpaperInstance>* instances);

 private:
  // 状态管理
  WallpaperState current_state_ = WallpaperState::ACTIVE;
  std::atomic<bool> is_paused_{false};
  std::string last_pause_reason_;
  std::string last_resume_reason_;

  // 回调函数
  StateChangeCallback state_change_callback_;
  WindowValidationCallback window_validation_callback_;
  ConfigurationRestoreCallback config_restore_callback_;

  // 壁纸实例引用（不拥有所有权）
  std::vector<WallpaperInstance>* wallpaper_instances_ = nullptr;

  // 内存优化器引用（不拥有所有权）
  MemoryOptimizer* memory_optimizer_ = nullptr;

  // 辅助方法
  void ChangeState(WallpaperState new_state, const std::string& reason);
  void PauseWebViewContent();
  void ResumeWebViewContent();
  bool ValidateSingleInstance(const WallpaperInstance& instance);

  // 统计信息
  size_t pause_count_ = 0;
  size_t resume_count_ = 0;
  size_t validation_failure_count_ = 0;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_WALLPAPER_LIFECYCLE_MANAGER_H_

