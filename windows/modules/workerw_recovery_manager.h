#ifndef FLUTTER_PLUGIN_WORKERW_RECOVERY_MANAGER_H_
#define FLUTTER_PLUGIN_WORKERW_RECOVERY_MANAGER_H_

#include <windows.h>
#include <functional>
#include <string>
#include <atomic>
#include <WebView2.h>
#include <wrl.h>

namespace anywp_engine {

// Forward declarations
struct WallpaperInstance {
  int monitor_index;
  bool enable_mouse_transparent;
  HWND webview_host_hwnd;
  HWND worker_w_hwnd;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview;
};

/**
 * @brief WorkerW 窗口恢复管理器
 * 
 * 处理 WorkerW 窗口的恢复策略：
 * - 检测 WorkerW 窗口是否失效
 * - Lively 风格的重新父子关系策略
 * - 必要时触发壁纸重建
 * 
 * @since v2.5.0
 */
class WorkerWRecoveryManager {
 public:
  /**
   * @brief 恢复策略枚举
   */
  enum class RecoveryStrategy {
    NONE,              // 无需恢复
    REPARENT,          // 重新设置父子关系
    RECREATE_WORKERW,  // 重新创建 WorkerW
    FULL_REINIT        // 完全重新初始化
  };

  /**
   * @brief 重新父子关系回调
   * 
   * 参数：
   * - instance: 需要重新父子关系的壁纸实例
   * 
   * 返回：是否成功
   */
  using ReparentCallback = std::function<bool(WallpaperInstance* instance)>;

  /**
   * @brief 重建请求回调
   * 
   * 参数：
   * - reason: 重建原因
   */
  using RecreateRequestCallback = std::function<void(const std::string& reason)>;

  WorkerWRecoveryManager();
  ~WorkerWRecoveryManager();

  // 禁止拷贝和赋值
  WorkerWRecoveryManager(const WorkerWRecoveryManager&) = delete;
  WorkerWRecoveryManager& operator=(const WorkerWRecoveryManager&) = delete;

  /**
   * @brief 初始化管理器
   */
  void Initialize();

  /**
   * @brief 执行 WorkerW 恢复（主入口）
   * 
   * 自动检测并决定使用哪种恢复策略
   * 
   * @return 使用的恢复策略
   */
  RecoveryStrategy RecoverWorkerW();

  /**
   * @brief 重新设置父子关系（策略1）
   * 
   * 适用于 WorkerW 窗口仍然存在但父子关系断开的情况
   * 
   * @return true 如果成功重新父子关系
   */
  bool RecoverWorkerW_Reparent();

  /**
   * @brief 检查 WorkerW 是否需要恢复
   * 
   * @param instance 要检查的壁纸实例
   * @return 建议的恢复策略
   */
  RecoveryStrategy CheckWorkerWStatus(const WallpaperInstance* instance);

  /**
   * @brief 验证 WorkerW 窗口的有效性
   * 
   * @param worker_w WorkerW 窗口句柄
   * @return true 如果窗口有效且父子关系正确
   */
  bool ValidateWorkerW(HWND worker_w);

  /**
   * @brief 设置重新父子关系回调
   */
  void SetReparentCallback(ReparentCallback callback);

  /**
   * @brief 设置重建请求回调
   */
  void SetRecreateRequestCallback(RecreateRequestCallback callback);

  /**
   * @brief 获取统计信息
   */
  size_t GetReparentAttemptCount() const;
  size_t GetSuccessfulReparentCount() const;
  size_t GetRecreateRequestCount() const;

 private:
  // 回调函数
  ReparentCallback reparent_callback_;
  RecreateRequestCallback recreate_request_callback_;

  // 恢复状态
  std::atomic<bool> is_recovering_{false};

  // 统计信息
  std::atomic<size_t> reparent_attempt_count_{0};
  std::atomic<size_t> successful_reparent_count_{0};
  std::atomic<size_t> recreate_request_count_{0};

  // 辅助方法
  bool ReparentSingleInstance(WallpaperInstance* instance);
  HWND FindOrCreateWorkerW();
  bool VerifyProgmanHierarchy(HWND worker_w);
};

}  // namespace anywp_engine

#endif  // FLUTTER_PLUGIN_WORKERW_RECOVERY_MANAGER_H_

