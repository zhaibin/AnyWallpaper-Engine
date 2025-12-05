#ifndef FLUTTER_PLUGIN_AUTO_RECOVERY_MANAGER_H_
#define FLUTTER_PLUGIN_AUTO_RECOVERY_MANAGER_H_

#include <string>
#include <map>
#include <mutex>
#include <functional>
#include <atomic>

namespace anywp_engine {

/**
 * @brief 自动恢复管理器
 * 
 * 管理壁纸配置的保存和自动恢复功能：
 * - 保存每个监视器的壁纸配置
 * - 检测 Explorer 重启等系统事件
 * - 自动重新应用壁纸配置
 * 
 * @since v2.5.0
 */
class AutoRecoveryManager {
 public:
  /**
   * @brief 壁纸配置结构
   */
  struct WallpaperConfig {
    std::string url;                    // 壁纸 URL
    int monitor_index;                  // 监视器索引
    bool enable_mouse_transparent;      // 是否启用鼠标穿透
    
    WallpaperConfig() 
      : monitor_index(-1), enable_mouse_transparent(false) {}
    
    WallpaperConfig(const std::string& url_, int monitor_idx, bool mouse_transparent)
      : url(url_), monitor_index(monitor_idx), enable_mouse_transparent(mouse_transparent) {}
  };

  /**
   * @brief 恢复回调类型
   * 
   * 参数：
   * - config: 要恢复的配置
   * 
   * 返回：是否成功恢复
   */
  using RecoveryCallback = std::function<bool(const WallpaperConfig& config)>;

  AutoRecoveryManager();
  ~AutoRecoveryManager();

  // 禁止拷贝和赋值
  AutoRecoveryManager(const AutoRecoveryManager&) = delete;
  AutoRecoveryManager& operator=(const AutoRecoveryManager&) = delete;

  /**
   * @brief 启用/禁用自动恢复
   * 
   * @param enabled true 启用，false 禁用
   */
  void SetEnabled(bool enabled);

  /**
   * @brief 检查是否启用自动恢复
   */
  bool IsEnabled() const;

  /**
   * @brief 保存壁纸配置
   * 
   * @param monitor_index 监视器索引
   * @param url 壁纸 URL
   * @param enable_mouse_transparent 是否启用鼠标穿透
   */
  void SaveWallpaperConfig(int monitor_index, const std::string& url, bool enable_mouse_transparent);

  /**
   * @brief 移除壁纸配置
   * 
   * @param monitor_index 监视器索引
   */
  void RemoveWallpaperConfig(int monitor_index);

  /**
   * @brief 清除所有配置
   */
  void ClearAllConfigs();

  /**
   * @brief 获取指定监视器的配置
   * 
   * @param monitor_index 监视器索引
   * @return 配置指针，如果不存在返回 nullptr
   */
  const WallpaperConfig* GetConfig(int monitor_index) const;

  /**
   * @brief 获取所有已保存的配置
   */
  std::map<int, WallpaperConfig> GetAllConfigs() const;

  /**
   * @brief 获取需要恢复的配置列表
   * 
   * v2.5.0+ Thread Safety Fix: 返回配置列表，由调用者在主线程执行恢复
   * 避免在 C++ 后台线程创建 WebView2 导致的异步回调问题
   * 
   * @return 需要恢复的配置映射（monitor_index -> config），如果不需要恢复则返回空映射
   */
  std::map<int, WallpaperConfig> GetConfigsForRecovery();

  /**
   * @brief 完成恢复（重置恢复标志）
   * 
   * 当主插件完成恢复操作后调用此方法，重置 is_recovering_ 标志
   */
  void CompleteRecovery();

  /**
   * @brief 执行自动恢复（已弃用，仅保留用于向后兼容）
   * 
   * @deprecated 使用 GetConfigsForRecovery() 替代，由主插件发送消息给 Flutter 主线程
   * @return 成功恢复的配置数量
   */
  size_t ExecuteAutoRecovery();

  /**
   * @brief 设置恢复回调
   * 
   * 当需要恢复壁纸时调用此回调
   */
  void SetRecoveryCallback(RecoveryCallback callback);

  /**
   * @brief 获取统计信息
   */
  size_t GetSavedConfigCount() const;
  size_t GetRecoveryAttemptCount() const;
  size_t GetSuccessfulRecoveryCount() const;

 private:
  // 配置存储（monitor_index -> config）
  std::map<int, WallpaperConfig> saved_configs_;
  mutable std::mutex config_mutex_;

  // 启用标志
  std::atomic<bool> enabled_{false};

  // 恢复状态
  std::atomic<bool> is_recovering_{false};

  // 回调函数
  RecoveryCallback recovery_callback_;

  // 统计信息
  std::atomic<size_t> recovery_attempt_count_{0};
  std::atomic<size_t> successful_recovery_count_{0};

  // 辅助方法
  bool RecoverSingleConfig(const WallpaperConfig& config);
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_AUTO_RECOVERY_MANAGER_H_

