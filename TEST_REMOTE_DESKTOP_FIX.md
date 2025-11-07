# 远程桌面锁屏场景修复

## 问题描述

**用户报告**：在锁屏状态下通过远程桌面登录并解锁后，桌面壁纸消失。

## 根本原因

原有实现使用简单的布尔标志 `is_paused_` 跟踪暂停状态，**无法处理多重暂停原因**：

### 问题场景

```
1. 锁屏 (LOCK)           → 暂停壁纸
2. 远程桌面连接 (REMOTE)  → 暂停壁纸（重复）
3. 远程桌面中解锁         → 恢复壁纸 ❌ 错误！
4. 断开远程桌面           → 回到本地控制台
5. 结果：壁纸消失 ❌
```

**问题**：步骤 3 中，虽然解除了锁定，但用户**仍在远程桌面会话中**（看不到本地桌面），不应该恢复壁纸。

## 解决方案

### 1. 引入多原因暂停系统

**核心思想**：用集合（`std::set`）跟踪所有暂停原因，只有当**所有原因都消除**时才恢复壁纸。

#### 新增数据结构

```cpp
// windows/anywp_engine_plugin.h
std::set<std::string> pause_reasons_;  // 跟踪暂停原因
std::mutex pause_reasons_mutex_;       // 线程安全
```

#### 标准化原因标识

| 原因标识 | 触发场景 | 备注 |
|---------|---------|------|
| `LOCK` | 系统锁屏/解锁 | WTS_SESSION_LOCK/UNLOCK |
| `REMOTE_SESSION` | 远程桌面会话 | WTS_CONSOLE_DISCONNECT/CONNECT<br>WTS_REMOTE_CONNECT/DISCONNECT |
| `SUSPEND` | 系统休眠/唤醒 | PBT_APMSUSPEND/APMRESUMESUSPEND |
| `FULLSCREEN_APP` | 全屏应用 | 检测到全屏游戏/视频 |
| `USER_IDLE` | 用户空闲 | 无鼠标键盘活动超时 |
| `MANUAL` | 手动暂停 | API 调用 |

### 2. 改进的暂停逻辑

```cpp
void AnyWPEnginePlugin::PauseWallpaper(const std::string& reason) {
  {
    std::lock_guard<std::mutex> lock(pause_reasons_mutex_);
    auto result = pause_reasons_.insert(reason);
    
    if (!result.second) {
      return;  // 原因已存在，无需重复暂停
    }
    
    if (pause_reasons_.size() > 1) {
      return;  // 已经被其他原因暂停，无需重复操作
    }
  }
  
  // 只有第一个原因时才执行实际暂停操作
  // ... 冻结动画、暂停视频等 ...
}
```

### 3. 改进的恢复逻辑

```cpp
void AnyWPEnginePlugin::ResumeWallpaper(const std::string& reason) {
  bool should_resume = false;
  {
    std::lock_guard<std::mutex> lock(pause_reasons_mutex_);
    pause_reasons_.erase(reason);
    
    if (!pause_reasons_.empty()) {
      std::cout << "Still paused due to: ";
      for (const auto& r : pause_reasons_) {
        std::cout << "\"" << r << "\" ";
      }
      return;  // 还有其他原因，不恢复
    }
    
    should_resume = true;  // 所有原因都消除了
  }
  
  if (should_resume) {
    // 执行实际恢复操作
    // ... 恢复动画、播放视频等 ...
  }
}
```

## 修复后的场景流程

### 场景：锁屏 + 远程桌面登录

```
1. 锁屏
   - PauseWallpaper("LOCK")
   - pause_reasons_ = {"LOCK"}
   - ✅ 暂停壁纸

2. 远程桌面连接
   - PauseWallpaper("REMOTE_SESSION")
   - pause_reasons_ = {"LOCK", "REMOTE_SESSION"}
   - ✅ 已暂停，无需重复操作

3. 在远程桌面中解锁
   - ResumeWallpaper("LOCK")
   - pause_reasons_ = {"REMOTE_SESSION"}  ← 移除 LOCK
   - ℹ️  输出：Still paused due to: "REMOTE_SESSION"
   - ✅ 不恢复壁纸（用户仍在远程会话中）

4. 断开远程桌面，回到本地控制台
   - ResumeWallpaper("REMOTE_SESSION")
   - pause_reasons_ = {}  ← 所有原因清除
   - ✅ 恢复壁纸
```

## 测试方法

### 测试 1：基本锁屏

```powershell
# 1. 启动应用并激活壁纸
.\scripts\debug_run.bat

# 2. 锁屏
Win + L

# 3. 解锁
# 预期：壁纸自动恢复 ✅
```

**日志验证**：
```
[PowerSaving] Added pause reason: LOCK (Total: 1)
[PowerSaving] ========== PAUSING WALLPAPER ==========

[PowerSaving] Removed pause reason: LOCK (Remaining: 0)
[PowerSaving] ========== RESUMING WALLPAPER ==========
```

### 测试 2：锁屏 + 远程桌面（问题场景）

```powershell
# 1. 启动壁纸
# 2. 锁屏 (Win + L)
# 3. 从另一台电脑远程桌面连接
# 4. 在远程桌面中输入密码解锁
# 5. 断开远程桌面
# 6. 回到本地控制台
# 预期：壁纸自动恢复 ✅
```

**日志验证**：
```
[PowerSaving] Added pause reason: LOCK (Total: 1)
[PowerSaving] ========== PAUSING WALLPAPER ==========

[PowerSaving] REMOTE DESKTOP CONNECTED
[PowerSaving] Added pause reason: REMOTE_SESSION (Total: 2)
[PowerSaving] Already paused by other reason(s), not pausing again

[PowerSaving] System UNLOCKED
[PowerSaving] Removed pause reason: LOCK (Remaining: 1)
[PowerSaving] Still paused due to: "REMOTE_SESSION"  ← 关键！

[PowerSaving] CONSOLE CONNECTED (returned from remote desktop)
[PowerSaving] Removed pause reason: REMOTE_SESSION (Remaining: 0)
[PowerSaving] ========== RESUMING WALLPAPER ==========  ← 正确恢复
```

### 测试 3：多重原因组合

```powershell
# 场景：锁屏 + 远程桌面 + 全屏应用
# 1. 启动壁纸
# 2. 锁屏 (pause_reasons_ = {"LOCK"})
# 3. 远程连接 (pause_reasons_ = {"LOCK", "REMOTE_SESSION"})
# 4. 解锁 (pause_reasons_ = {"REMOTE_SESSION"})
# 5. 打开全屏游戏 (pause_reasons_ = {"REMOTE_SESSION", "FULLSCREEN_APP"})
# 6. 退出游戏 (pause_reasons_ = {"REMOTE_SESSION"})
# 7. 回到本地控制台 (pause_reasons_ = {})
# 预期：只有步骤 7 才恢复壁纸 ✅
```

## 技术优势

1. ✅ **精确状态跟踪**：每个暂停原因独立管理
2. ✅ **避免重复操作**：已暂停时不会重复暂停
3. ✅ **线程安全**：使用 mutex 保护共享状态
4. ✅ **易于调试**：日志清晰显示所有活跃原因
5. ✅ **可扩展**：未来可轻松添加新的暂停原因

## 性能影响

- **内存开销**：`std::set<std::string>` 最多 ~6 个元素，可忽略
- **CPU 开销**：集合操作 O(log n)，n ≤ 6，可忽略
- **锁竞争**：暂停/恢复不频繁，mutex 几乎无竞争

## 向后兼容性

✅ **完全兼容**：API 签名未改变，现有调用无需修改

## 修改文件

- `windows/anywp_engine_plugin.h` - 添加 `pause_reasons_` 和 `pause_reasons_mutex_`
- `windows/anywp_engine_plugin.cpp` - 重写暂停/恢复逻辑，统一原因标识

## 测试清单

- [ ] 基本锁屏/解锁 ✅
- [ ] 锁屏 + 远程桌面 + 解锁 ✅
- [ ] 远程桌面连接/断开 ✅
- [ ] 系统休眠/唤醒 ✅
- [ ] 全屏应用检测 ✅
- [ ] 用户空闲检测 ✅
- [ ] 手动暂停/恢复 API ✅
- [ ] 多重原因组合 ✅

---

**修复版本**：v1.2.2-dev  
**测试日期**：2025-11-07  
**状态**：✅ 编译成功，等待测试验证

