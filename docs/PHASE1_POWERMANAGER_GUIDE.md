# Phase 1 PowerManager 集成 - 详细操作指南

## 当前状态
- ✅ 头文件已更新：添加了 `#include "modules/power_manager.h"`
- ✅ 头文件已更新：移除了 PowerState 枚举和相关成员变量
- ✅ 头文件已更新：添加了 `std::unique_ptr<PowerManager> power_manager_`

## 下一步：修改主文件 cpp

由于主文件有 4176 行，手动修改容易出错。建议采用分步替换策略。

### 策略 1: 初始化 PowerManager（在构造函数中）

**查找位置**: `AnyWPEnginePlugin::AnyWPEnginePlugin()` 构造函数

**添加代码**:
```cpp
// 在构造函数末尾添加
power_manager_ = std::make_unique<PowerManager>();

// 设置回调
power_manager_->SetOnStateChanged([this](auto old_state, auto new_state) {
  // 处理状态变更
  if (new_state == PowerManager::PowerState::FULLSCREEN_APP || 
      new_state == PowerManager::PowerState::LOCKED) {
    // 暂停壁纸
    is_paused_ = true;
    NotifyWebContentVisibility(false);
  } else if (old_state != PowerManager::PowerState::ACTIVE && 
             new_state == PowerManager::PowerState::ACTIVE) {
    // 恢复壁纸
    is_paused_ = false;
    NotifyWebContentVisibility(true);
  }
});

power_manager_->SetOnPause([this](const std::string& reason) {
  std::cout << "[AnyWP] Power Manager pausing wallpaper: " << reason << std::endl;
  is_paused_ = true;
  NotifyWebContentVisibility(false);
});

power_manager_->SetOnResume([this](const std::string& reason) {
  std::cout << "[AnyWP] Power Manager resuming wallpaper: " << reason << std::endl;
  is_paused_ = false;
  NotifyWebContentVisibility(true);
});

// 启用省电功能
power_manager_->Enable(true);
```

### 策略 2: 清理 PowerManager（在析构函数中）

**查找位置**: `AnyWPEnginePlugin::~AnyWPEnginePlugin()` 析构函数

**添加代码**:
```cpp
// 在析构函数开始处添加
if (power_manager_) {
  power_manager_->Enable(false);
  power_manager_.reset();
}
```

### 策略 3: 替换方法调用

由于工作量巨大，我建议使用以下简化方案：

#### 方案 A：保留方法，内部委托（推荐）

**不删除这些方法**，而是修改为委托给 PowerManager：

```cpp
// 示例：修改 SetupPowerSavingMonitoring
void AnyWPEnginePlugin::SetupPowerSavingMonitoring() {
  if (power_manager_) {
    power_manager_->Enable(true);
  }
}

// 示例：修改 UpdatePowerState
void AnyWPEnginePlugin::UpdatePowerState() {
  if (power_manager_) {
    power_manager_->UpdatePowerState();
  }
}

// 示例：修改 IsFullscreenAppActive
bool AnyWPEnginePlugin::IsFullscreenAppActive() {
  if (power_manager_) {
    return power_manager_->IsFullscreenAppActive();
  }
  return false;
}
```

**优点**:
- ✅ 风险最低
- ✅ 不需要查找所有调用点
- ✅ 向后兼容
- ✅ 可以逐步删除这些委托方法

#### 方案 B：完全删除并替换所有调用点（风险高，不推荐现在做）

**删除所有 Power 相关方法**，然后全局搜索替换调用：

```cpp
// 旧代码
UpdatePowerState();

// 新代码
if (power_manager_) power_manager_->UpdatePowerState();
```

**缺点**:
- ❌ 需要查找所有调用点（可能有几十处）
- ❌ 容易遗漏
- ❌ 风险高

---

## 推荐的实施步骤

### Step 1: 更新构造函数和析构函数
- 在构造函数中初始化 `power_manager_`
- 在析构函数中清理 `power_manager_`

### Step 2: 修改方法为委托（不删除）
保留现有方法签名，内部委托给 `power_manager_`：
- `SetupPowerSavingMonitoring()` → 委托给 `power_manager_->Enable(true)`
- `CleanupPowerSavingMonitoring()` → 委托给 `power_manager_->Enable(false)`
- `UpdatePowerState()` → 委托给 `power_manager_->UpdatePowerState()`
- `IsFullscreenAppActive()` → 委托给 `power_manager_->IsFullscreenAppActive()`
- `PauseWallpaper()` → 委托给 `power_manager_->Pause()`
- `ResumeWallpaper()` → 委托给 `power_manager_->Resume()`
- 等等...

### Step 3: 编译测试
- 编译确保无错误
- 运行功能测试

### Step 4: （可选）后续清理
在确认功能正常后，可以逐步删除这些委托方法，直接调用 `power_manager_`。

---

## 预期效果

### 立即效果（方案 A）
- ✅ 头文件减少 ~60 行（已完成）
- ✅ 代码更清晰
- ✅ 职责分离
- ⚠️ cpp 文件行数暂时不减少（因为保留了委托方法）

### 后续效果（方案 B，可选）
- ✅ cpp 文件最终减少 ~613 行
- ✅ 彻底移除重复代码

---

## 我的建议

**现在执行**: 方案 A（委托方法）
**原因**:
1. 最安全，风险最低
2. 可以立即编译测试
3. 不破坏现有调用
4. 后续可以逐步清理

**是否继续执行方案 A？**

如果继续，我会：
1. 修改构造函数和析构函数
2. 将 10 个 Power 相关方法改为委托
3. 编译测试

预计时间：30-45 分钟

