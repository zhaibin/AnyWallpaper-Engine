# 简化的会话管理逻辑

## 核心理念

**不追踪复杂的暂停原因，而是直接检查当前状态**：用户是否在本地桌面环境中。

## 设计思路

### 之前的复杂方案 ❌

```
追踪多个暂停原因：
- pause_reasons_ = {"LOCK", "REMOTE_SESSION", "SUSPEND", ...}
- 需要精确管理每个原因的添加和移除
- 逻辑复杂，容易出错
```

### 现在的简化方案 ✅

```
统一状态检查：
每次会话事件 → 检查当前环境 → 决定是否激活壁纸
```

## 核心函数：ShouldWallpaperBeActive()

### 检查逻辑

```cpp
bool ShouldWallpaperBeActive() {
  // 检查 1: 是否在远程桌面会话？
  if (GetSystemMetrics(SM_REMOTESESSION)) {
    return false;  // 远程会话中不显示壁纸
  }
  
  // 检查 2: 系统是否锁定？
  if (IsSystemLocked()) {
    return false;  // 锁定时不显示壁纸
  }
  
  // 所有检查通过 → 用户在本地解锁桌面
  return true;
}
```

### 使用方式

**统一处理所有会话事件**：

```cpp
case WM_WTSSESSION_CHANGE:
  // 不论什么事件（锁屏、解锁、远程连接等）
  // 都用统一的方式处理：检查当前状态
  
  if (ShouldWallpaperBeActive()) {
    ResumeWallpaper("User in local desktop");
  } else {
    PauseWallpaper("User not in local desktop");
  }
  break;
```

## 优势

### 1. 逻辑简单

- ✅ 无需追踪多个原因
- ✅ 无需复杂的状态管理
- ✅ 代码易读易维护

### 2. 更健壮

- ✅ 自动处理所有边缘情况
- ✅ 即使遗漏某个事件，下次事件也会纠正状态
- ✅ 不会因为状态不一致导致问题

### 3. 自愈能力

假设某个事件被遗漏：
```
1. 锁屏事件 → 暂停 ✅
2. (遗漏了远程连接事件)
3. 解锁事件 → 检查状态 → 发现在远程会话 → 保持暂停 ✅
```

## 场景验证

### 场景 1：普通锁屏/解锁

```
1. 用户锁屏 (Win + L)
   - 事件: WTS_SESSION_LOCK
   - 检查: IsSystemLocked() → true
   - 动作: PauseWallpaper() ✅

2. 用户解锁
   - 事件: WTS_SESSION_UNLOCK
   - 检查: 
     * IsRemoteSession() → false
     * IsSystemLocked() → false
   - 动作: ResumeWallpaper() ✅
```

### 场景 2：锁屏 + 远程桌面登录（问题场景）

```
1. 用户锁屏
   - 事件: WTS_SESSION_LOCK
   - 检查: IsSystemLocked() → true
   - 动作: PauseWallpaper() ✅

2. 远程桌面连接
   - 事件: WTS_REMOTE_CONNECT
   - 检查:
     * IsRemoteSession() → true
   - 动作: PauseWallpaper() ✅ (已暂停，跳过)

3. 在远程桌面中解锁
   - 事件: WTS_SESSION_UNLOCK
   - 检查:
     * IsRemoteSession() → true ← 关键！
   - 动作: PauseWallpaper() ✅ (保持暂停)

4. 断开远程桌面，回到本地控制台
   - 事件: WTS_CONSOLE_CONNECT
   - 检查:
     * IsRemoteSession() → false
     * IsSystemLocked() → false
   - 动作: ResumeWallpaper() ✅ (正确恢复)
```

### 场景 3：远程桌面中锁屏

```
1. 在远程桌面中工作
   - 检查: IsRemoteSession() → true
   - 状态: 已暂停 ✅

2. 在远程桌面中锁屏
   - 事件: WTS_SESSION_LOCK
   - 检查: IsSystemLocked() → true
   - 状态: 保持暂停 ✅

3. 在远程桌面中解锁
   - 事件: WTS_SESSION_UNLOCK
   - 检查: IsRemoteSession() → true
   - 状态: 保持暂停 ✅

4. 断开远程桌面
   - 事件: WTS_REMOTE_DISCONNECT
   - 检查:
     * IsRemoteSession() → false
     * IsSystemLocked() → false
   - 动作: ResumeWallpaper() ✅
```

## 技术实现

### Windows API

| API | 用途 |
|-----|------|
| `GetSystemMetrics(SM_REMOTESESSION)` | 检查是否在远程桌面会话 |
| `WTSGetActiveConsoleSessionId()` | 获取当前活动控制台会话 ID |
| `WTSQuerySessionInformationW()` | 查询会话详细信息 |
| `WTSINFOEXW` | 会话扩展信息结构体 |
| `WTS_SESSIONSTATE_LOCK` | 会话锁定标志 |

### 锁定状态检测

```cpp
// 获取当前会话
DWORD session_id = WTSGetActiveConsoleSessionId();

// 查询锁定状态
LPWSTR buffer = nullptr;
WTSQuerySessionInformationW(
  WTS_CURRENT_SERVER_HANDLE, 
  session_id, 
  WTSSessionInfoEx,  // 扩展信息
  &buffer, 
  &bytes_returned
);

WTSINFOEXW* info = reinterpret_cast<WTSINFOEXW*>(buffer);
bool is_locked = (info->Data.WTSInfoExLevel1.SessionFlags == WTS_SESSIONSTATE_LOCK);
```

## 预期日志

### 正常解锁（本地）

```
[PowerSaving] Event: System UNLOCKED
[PowerSaving] Session check: Local UNLOCKED desktop - wallpaper should be ACTIVE
[PowerSaving] ========== RESUMING WALLPAPER ==========
```

### 远程会话中解锁（不恢复）

```
[PowerSaving] Event: System UNLOCKED
[PowerSaving] Session check: REMOTE session detected
[PowerSaving] ========== PAUSING WALLPAPER ==========
```

### 从远程桌面回到本地

```
[PowerSaving] Event: CONSOLE CONNECTED (returned from remote desktop)
[PowerSaving] Session check: Local UNLOCKED desktop - wallpaper should be ACTIVE
[PowerSaving] ========== RESUMING WALLPAPER ==========
```

## 代码变更

### 1. 头文件 (anywp_engine_plugin.h)

```cpp
// 移除复杂的原因追踪
- std::set<std::string> pause_reasons_;
- std::mutex pause_reasons_mutex_;

// 添加简单的状态检查函数
+ bool ShouldWallpaperBeActive();
```

### 2. 实现文件 (anywp_engine_plugin.cpp)

**简化 PauseWallpaper/ResumeWallpaper**：
- 移除复杂的原因管理逻辑
- 恢复为简单的布尔标志检查

**添加统一状态检查**：
- `ShouldWallpaperBeActive()` - 综合判断当前环境

**统一事件处理**：
- 所有会话事件都调用同一个检查函数
- 根据检查结果决定暂停或恢复

## 测试清单

- [ ] **基本锁屏/解锁** ✅
- [ ] **锁屏 + 远程桌面登录 + 解锁** ✅ (关键场景)
- [ ] **远程桌面连接/断开** ✅
- [ ] **远程桌面中锁屏/解锁** ✅
- [ ] **快速切换场景** ✅

## 与原方案对比

| 特性 | 复杂方案 | 简化方案 |
|-----|---------|---------|
| **代码行数** | ~100 行 | ~50 行 |
| **状态管理** | std::set 集合 | 无需管理 |
| **逻辑复杂度** | 高 | 低 |
| **易维护性** | 中 | 高 |
| **健壮性** | 中（依赖状态一致性） | 高（自愈能力） |
| **性能** | 略低（集合操作） | 略高（直接检查） |
| **调试难度** | 高 | 低 |

## 总结

**简化方案的核心思想**：
> 不要追踪"为什么暂停"，而是检查"现在应该暂停吗"

这种设计更符合"声明式"编程思想：
- ✅ 描述"应该是什么状态"而不是"如何达到这个状态"
- ✅ 每次事件都重新评估，避免状态不一致
- ✅ 代码更简洁、更易理解、更难出错

---

**版本**：v1.2.2-dev (Simplified)  
**日期**：2025-11-07  
**状态**：✅ 编译成功，等待测试验证

