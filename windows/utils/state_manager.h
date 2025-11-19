#ifndef FLUTTER_PLUGIN_STATE_MANAGER_H_
#define FLUTTER_PLUGIN_STATE_MANAGER_H_

#include <atomic>
#include <mutex>
#include <functional>

namespace anywp_engine {

/**
 * @brief 状态管理器（工具类）
 * 
 * 统一管理插件的各种状态：
 * - 电源状态
 * - 会话状态
 * - 暂停/恢复状态
 * - 状态同步
 * 
 * @since v2.5.0
 */
class StateManager {
 public:
  /**
   * @brief 电源状态枚举
   */
  enum class PowerState {
    ACTIVE,
    IDLE,
    SCREEN_OFF,
    LOCKED,
    FULLSCREEN_APP,
    PAUSED
  };

  /**
   * @brief 状态变化回调类型
   */
  using StateChangeCallback = std::function<void(PowerState old_state, PowerState new_state)>;

  StateManager();
  ~StateManager();

  // 禁止拷贝和赋值
  StateManager(const StateManager&) = delete;
  StateManager& operator=(const StateManager&) = delete;

  // 电源状态管理
  void SetPowerState(PowerState state);
  PowerState GetPowerState() const;
  PowerState GetLastPowerState() const;

  // 会话状态管理
  void SetSessionLocked(bool locked);
  bool IsSessionLocked() const;

  void SetRemoteSession(bool remote);
  bool IsRemoteSession() const;

  // 暂停状态
  void SetPaused(bool paused);
  bool IsPaused() const;

  // 状态变化通知
  void SetStateChangeCallback(StateChangeCallback callback);

  // 辅助方法
  bool ShouldWallpaperBeActive() const;
  std::string PowerStateToString(PowerState state) const;

 private:
  PowerState current_power_state_ = PowerState::ACTIVE;
  PowerState last_power_state_ = PowerState::ACTIVE;
  
  std::atomic<bool> is_session_locked_{false};
  std::atomic<bool> is_remote_session_{false};
  std::atomic<bool> is_paused_{false};

  StateChangeCallback state_change_callback_;
  mutable std::mutex state_mutex_;
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_STATE_MANAGER_H_

