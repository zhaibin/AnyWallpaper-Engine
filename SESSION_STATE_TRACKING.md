# 会话状态跟踪改进

## 问题

用户报告：从锁屏状态恢复后，视频没有自动播放，恢复次数为0。

## 根本原因

之前的实现使用 `WTSQuerySessionInformationW()` API 动态查询锁定状态，但这个方法：
1. **不可靠**：某些 Windows 版本可能不支持 `WTSSessionInfoEx`
2. **复杂**：需要处理缓冲区、结构体、内存释放
3. **性能开销**：每次检查都要调用系统 API

## 解决方案：事件驱动的状态跟踪

### 核心思想

**不要查询状态，而是跟踪状态**：
- 收到 `WTS_SESSION_LOCK` → 记录锁定
- 收到 `WTS_SESSION_UNLOCK` → 记录解锁
- 收到 `WTS_CONSOLE_DISCONNECT` → 记录进入远程会话
- 收到 `WTS_CONSOLE_CONNECT` → 记录回到本地

### 实现

#### 1. 添加状态标志

```cpp
// windows/anywp_engine_plugin.h
std::atomic<bool> is_session_locked_{false};   // 锁定状态
std::atomic<bool> is_remote_session_{false};   // 远程会话状态
```

使用 `std::atomic` 保证线程安全，无需 mutex。

#### 2. 初始化状态

```cpp
void SetupPowerSavingMonitoring() {
  // 检查初始是否在远程会话
  is_remote_session_.store(GetSystemMetrics(SM_REMOTESESSION) != 0);
  // 假设启动时未锁定（合理假设）
  is_session_locked_.store(false);
}
```

#### 3. 事件驱动更新

```cpp
case WM_WTSSESSION_CHANGE:
  switch (wParam) {
    case WTS_SESSION_LOCK:
      is_session_locked_.store(true);    // 标记锁定
      break;
    case WTS_SESSION_UNLOCK:
      is_session_locked_.store(false);   // 标记解锁
      break;
    case WTS_CONSOLE_DISCONNECT:
    case WTS_REMOTE_CONNECT:
      is_remote_session_.store(true);    // 标记远程会话
      break;
    case WTS_CONSOLE_CONNECT:
    case WTS_REMOTE_DISCONNECT:
      is_remote_session_.store(false);   // 标记本地会话
      break;
  }
  
  // 统一检查
  if (ShouldWallpaperBeActive()) {
    ResumeWallpaper();
  } else {
    PauseWallpaper();
  }
```

#### 4. 简化状态检查

```cpp
bool ShouldWallpaperBeActive() {
  bool locked = is_session_locked_.load();
  bool remote = is_remote_session_.load();
  
  // 只有在本地且未锁定时才激活壁纸
  return !locked && !remote;
}
```

## 优势对比

| 特性 | 旧方案（查询 API） | 新方案（事件跟踪） |
|-----|------------------|------------------|
| **可靠性** | 中（依赖 API 支持） | 高（事件驱动） |
| **性能** | 低（每次调用 API） | 高（读取内存标志） |
| **复杂度** | 高（处理缓冲区、结构体） | 低（简单布尔值） |
| **代码量** | ~40 行 | ~10 行 |
| **调试** | 难（API 行为不透明） | 易（状态清晰可见） |

## 预期日志

### 锁屏事件

```
[PowerSaving] ========== SESSION CHANGE EVENT ==========
[PowerSaving] Event code: 7  (WTS_SESSION_LOCK)
[PowerSaving] Event: System LOCKED
[PowerSaving] Checking if wallpaper should be active...
[PowerSaving] Session check: locked=1, remote=0
[PowerSaving] → System is LOCKED, wallpaper should be PAUSED
[PowerSaving] ========== PAUSING WALLPAPER ==========
[PowerSaving] =========================================
```

### 解锁事件（本地）

```
[PowerSaving] ========== SESSION CHANGE EVENT ==========
[PowerSaving] Event code: 8  (WTS_SESSION_UNLOCK)
[PowerSaving] Event: System UNLOCKED
[PowerSaving] Checking if wallpaper should be active...
[PowerSaving] Session check: locked=0, remote=0
[PowerSaving] → Local UNLOCKED desktop, wallpaper should be ACTIVE
[PowerSaving] ========== RESUMING WALLPAPER ==========
[PowerSaving] =========================================
```

### 在远程会话中解锁（关键场景）

```
[PowerSaving] ========== SESSION CHANGE EVENT ==========
[PowerSaving] Event code: 8  (WTS_SESSION_UNLOCK)
[PowerSaving] Event: System UNLOCKED
[PowerSaving] Checking if wallpaper should be active...
[PowerSaving] Session check: locked=0, remote=1  ← 关键！
[PowerSaving] → In REMOTE session, wallpaper should be PAUSED
[PowerSaving] ========== PAUSING WALLPAPER ==========  ← 保持暂停
[PowerSaving] =========================================
```

### 回到本地控制台

```
[PowerSaving] ========== SESSION CHANGE EVENT ==========
[PowerSaving] Event code: 5  (WTS_CONSOLE_CONNECT)
[PowerSaving] Event: CONSOLE CONNECTED (returned from remote desktop)
[PowerSaving] Checking if wallpaper should be active...
[PowerSaving] Session check: locked=0, remote=0  ← 两个标志都为false
[PowerSaving] → Local UNLOCKED desktop, wallpaper should be ACTIVE
[PowerSaving] ========== RESUMING WALLPAPER ==========  ← 正确恢复
[PowerSaving] =========================================
```

## 测试场景

### 场景 1：普通锁屏/解锁

```
1. 锁屏 (Win + L)
   - is_session_locked_ = true
   - ShouldWallpaperBeActive() → false
   - PauseWallpaper() ✅

2. 解锁
   - is_session_locked_ = false
   - is_remote_session_ = false
   - ShouldWallpaperBeActive() → true
   - ResumeWallpaper() ✅
```

### 场景 2：锁屏 + 远程桌面

```
1. 锁屏
   - is_session_locked_ = true
   - PauseWallpaper() ✅

2. 远程连接
   - is_remote_session_ = true
   - (已暂停，跳过) ✅

3. 远程中解锁
   - is_session_locked_ = false
   - is_remote_session_ = true ← 仍为true！
   - ShouldWallpaperBeActive() → false
   - PauseWallpaper() ✅ (保持暂停)

4. 回到本地
   - is_remote_session_ = false
   - is_session_locked_ = false
   - ShouldWallpaperBeActive() → true
   - ResumeWallpaper() ✅ (正确恢复)
```

## 技术细节

### 原子操作

使用 `std::atomic<bool>` 的优势：
- **线程安全**：无需 mutex
- **性能**：无锁操作
- **简单**：类似普通布尔值的使用方式

```cpp
// 写入（在事件处理线程）
is_session_locked_.store(true);

// 读取（在检查函数）
bool locked = is_session_locked_.load();
```

### Windows 事件映射

| WTS 事件 | 含义 | 更新状态 |
|---------|------|---------|
| `WTS_SESSION_LOCK` (7) | 系统锁定 | `is_session_locked_ = true` |
| `WTS_SESSION_UNLOCK` (8) | 系统解锁 | `is_session_locked_ = false` |
| `WTS_CONSOLE_DISCONNECT` (3) | 离开本地控制台 | `is_remote_session_ = true` |
| `WTS_CONSOLE_CONNECT` (5) | 回到本地控制台 | `is_remote_session_ = false` |
| `WTS_REMOTE_CONNECT` (1) | 远程桌面连接 | `is_remote_session_ = true` |
| `WTS_REMOTE_DISCONNECT` (2) | 远程桌面断开 | `is_remote_session_ = false` |

## 调试提示

### 查看状态变化

日志中会显示每次状态检查：
```
Session check: locked=0, remote=1
```

### 验证状态一致性

如果发现状态不一致：
1. 检查日志中的事件序列
2. 确认每个事件都正确更新了标志
3. 确认初始化时的状态正确

## 修改文件

- `windows/anywp_engine_plugin.h` - 添加两个 atomic 标志
- `windows/anywp_engine_plugin.cpp` - 实现事件驱动的状态跟踪

## 向后兼容性

✅ **完全兼容**：仅内部实现变化，API 和行为保持一致

---

**版本**：v1.2.2-dev (State Tracking)  
**日期**：2025-11-07  
**状态**：✅ 编译成功，等待测试验证  
**改进**：从 API 查询改为事件驱动跟踪，更可靠、更高效

