#ifndef ANYWP_ENGINE_WORKERW_HEALTH_MONITOR_H_
#define ANYWP_ENGINE_WORKERW_HEALTH_MONITOR_H_

#include <windows.h>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

namespace anywp_engine {

/**
 * WorkerW Health Monitor - 监控 WorkerW 窗口的健康状态并自动恢复
 * 
 * 功能：
 * - 周期性验证 WorkerW 窗口的有效性
 * - 检测 WorkerW 失效后自动触发恢复回调
 * - 支持自定义检查间隔和恢复策略
 * - 线程安全的状态管理
 * 
 * 使用场景：
 * - 显示设置变更后 WorkerW 失效
 * - 桌面刷新导致 WorkerW 重建
 * - 系统休眠/唤醒后 WorkerW 异常
 * - 用户更换桌面背景
 * 
 * 示例：
 * ```cpp
 * WorkerWHealthMonitor monitor;
 * monitor.SetRecoveryCallback([this]() {
 *   // 重新查找 WorkerW 并重建壁纸
 *   this->RecoverWallpaper();
 * });
 * monitor.StartMonitoring(workerw_hwnd, 5000); // 5秒检查一次
 * ```
 */
class WorkerWHealthMonitor {
public:
  /**
   * 恢复回调函数类型
   * 当检测到 WorkerW 失效时，会调用此回调
   */
  using RecoveryCallback = std::function<void()>;
  
  /**
   * WorkerW 健康状态
   */
  enum class HealthStatus {
    UNKNOWN,     // 未知状态（未开始监控）
    HEALTHY,     // 健康（WorkerW 有效）
    UNHEALTHY,   // 不健康（WorkerW 失效）
    RECOVERING   // 恢复中
  };

  WorkerWHealthMonitor();
  ~WorkerWHealthMonitor();
  
  // 禁止复制
  WorkerWHealthMonitor(const WorkerWHealthMonitor&) = delete;
  WorkerWHealthMonitor& operator=(const WorkerWHealthMonitor&) = delete;

  /**
   * 开始监控 WorkerW 窗口
   * @param workerw WorkerW 窗口句柄
   * @param check_interval_ms 检查间隔（毫秒），默认 3000ms
   * @return true 成功，false 失败
   */
  bool StartMonitoring(HWND workerw, int check_interval_ms = 3000);
  
  /**
   * 停止监控
   */
  void StopMonitoring();
  
  /**
   * 更新 WorkerW 句柄（恢复后调用）
   * @param workerw 新的 WorkerW 窗口句柄
   */
  void UpdateWorkerW(HWND workerw);
  
  /**
   * 设置恢复回调函数
   * @param callback 恢复回调
   */
  void SetRecoveryCallback(RecoveryCallback callback);
  
  /**
   * 获取当前健康状态
   */
  HealthStatus GetHealthStatus() const;
  
  /**
   * 手动触发健康检查
   * @return true WorkerW 健康，false WorkerW 失效
   */
  bool CheckHealth();
  
  /**
   * 获取连续失败次数
   */
  int GetConsecutiveFailures() const;
  
  /**
   * 获取上次检查时间
   */
  std::chrono::steady_clock::time_point GetLastCheckTime() const;
  
  /**
   * 是否正在监控
   */
  bool IsMonitoring() const;

private:
  // WorkerW 句柄
  HWND workerw_hwnd_;
  
  // 监控线程
  std::thread monitor_thread_;
  std::atomic<bool> should_stop_;
  std::atomic<bool> is_monitoring_;
  
  // 健康状态
  std::atomic<HealthStatus> health_status_;
  std::atomic<int> consecutive_failures_;
  std::chrono::steady_clock::time_point last_check_time_;
  
  // 检查间隔
  int check_interval_ms_;
  
  // 恢复回调
  RecoveryCallback recovery_callback_;
  std::mutex callback_mutex_;
  
  // 恢复控制（防止频繁恢复）
  std::chrono::steady_clock::time_point last_recovery_time_;
  int min_recovery_interval_ms_ = 2000; // 最小恢复间隔 2 秒
  
  // 互斥锁
  mutable std::mutex mutex_;
  
  /**
   * 监控线程主循环
   */
  void MonitorThreadProc();
  
  /**
   * 检查 WorkerW 是否有效
   * @return true 有效，false 失效
   */
  bool IsWorkerWValid();
  
  /**
   * 检查 WorkerW 是否仍然是壁纸层
   * 验证 SHELLDLL_DefView 结构是否完整
   */
  bool IsWorkerWStillWallpaperLayer();
  
  /**
   * 触发恢复回调
   */
  void TriggerRecovery();
  
  /**
   * 记录健康检查结果
   */
  void RecordCheckResult(bool is_healthy);
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_WORKERW_HEALTH_MONITOR_H_

