# Phase 1 渐进式重构 - 超保守执行计划

## 策略：零风险渐进式集成

### 核心原则
1. ✅ **绝不删除现有代码**
2. ✅ **添加新模块作为可选功能**
3. ✅ **旧方法委托给新模块**
4. ✅ **每步立即编译测试**
5. ✅ **保持向后兼容**

---

## Step 1: PowerManager 集成（零风险版本）

### 1.1 修改头文件（添加，不删除）

**在 `windows/anywp_engine_plugin.h` 中**：

```cpp
// 在文件开头的 include 部分添加
#include "modules/power_manager.h"

// 在 private 成员部分末尾添加（不删除任何现有成员）
class AnyWPEnginePlugin : public flutter::Plugin {
  // ... 现有所有成员保持不变 ...
  
  // ========== New: PowerManager Integration (v1.4.0+) ==========
  std::unique_ptr<PowerManager> power_manager_;  // 新增
};
```

**不删除任何现有内容**：
- ❌ 不删除 PowerState 枚举
- ❌ 不删除任何成员变量
- ❌ 不删除任何方法声明

### 1.2 修改构造函数（添加初始化）

**在 `windows/anywp_engine_plugin.cpp` 构造函数末尾添加**：

```cpp
AnyWPEnginePlugin::AnyWPEnginePlugin() {
  // ... 现有所有初始化代码保持不变 ...
  
  // Setup power saving monitoring
  SetupPowerSavingMonitoring();
  
  // ========== New: Initialize PowerManager module (v1.4.0+) ==========
  std::cout << "[AnyWP] [Refactor] Initializing PowerManager module..." << std::endl;
  try {
    power_manager_ = std::make_unique<PowerManager>();
    std::cout << "[AnyWP] [Refactor] PowerManager module initialized successfully" << std::endl;
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [Refactor] ERROR: Failed to initialize PowerManager: " 
              << e.what() << std::endl;
  }
}
```

### 1.3 修改析构函数（添加清理）

**在 `windows/anywp_engine_plugin.cpp` 析构函数开头添加**：

```cpp
AnyWPEnginePlugin::~AnyWPEnginePlugin() {
  std::cout << "[AnyWP] Plugin destructor - starting cleanup" << std::endl;
  
  // ========== New: Cleanup PowerManager module (v1.4.0+) ==========
  if (power_manager_) {
    std::cout << "[AnyWP] [Refactor] Cleaning up PowerManager module..." << std::endl;
    try {
      power_manager_->Enable(false);
      power_manager_.reset();
    } catch (const std::exception& e) {
      std::cout << "[AnyWP] [Refactor] ERROR: Failed to cleanup PowerManager: " 
                << e.what() << std::endl;
    }
  }
  
  // ... 现有所有清理代码保持不变 ...
}
```

### 1.4 编译测试（验证集成）

```bash
cd example
flutter clean
flutter build windows --debug
```

**预期结果**：
- ✅ 编译成功
- ✅ PowerManager 模块被初始化
- ✅ 所有现有功能正常

**如果编译失败**：
- 立即回滚
- 分析错误原因

### 1.5 功能测试

```bash
.\scripts\test.bat
```

**验证**：
- 基础功能正常
- 省电功能正常
- 无崩溃或异常

---

## Step 2: 方法委托（可选，后续执行）

**只在 Step 1 成功后才执行**

### 示例：修改 `UpdatePowerState()` 方法

```cpp
void AnyWPEnginePlugin::UpdatePowerState() {
  // ========== New: Delegate to PowerManager (v1.4.0+) ==========
  if (power_manager_) {
    try {
      power_manager_->UpdatePowerState();
      // 可选：同步状态回旧成员变量（保持兼容性）
      power_state_ = static_cast<PowerState>(
        static_cast<int>(power_manager_->GetCurrentState())
      );
      return;
    } catch (const std::exception& e) {
      std::cout << "[AnyWP] [Refactor] PowerManager delegation failed, "
                << "falling back to legacy implementation: " << e.what() << std::endl;
    }
  }
  
  // ========== Legacy implementation (fallback) ==========
  // ... 原有代码保持不变 ...
}
```

**优点**：
- ✅ 如果新模块失败，自动降级到旧代码
- ✅ 零风险
- ✅ 可以逐个方法迁移

---

## Step 3-8: 其他模块（后续）

等 Step 1 和 Step 2 稳定后，再继续：
- Step 3: MonitorManager 集成
- Step 4: MouseHookManager 集成
- Step 5: WindowManager 创建
- Step 6: WebViewLifecycleManager 创建
- Step 7: WallpaperInstanceManager 创建
- Step 8: 清理旧代码（最后一步）

---

## 预期时间表

### 本次执行（Step 1）
- 修改代码：10 分钟
- 编译测试：5 分钟
- 功能验证：10 分钟
- **总计**：25 分钟

### 完整 Phase 1（所有步骤）
- Step 1-2: 1 小时
- Step 3-4: 1 小时
- Step 5-7: 4 小时
- Step 8: 2 小时
- **总计**：8 小时

---

## 风险控制

### 如果 Step 1 失败
- 立即 `git restore` 回滚
- 分析错误日志
- 调整方案

### 如果 Step 1 成功
- 提交代码 `git commit`
- 继续 Step 2

---

## 决策点

**现在立即执行 Step 1？**

**是**: 我立即开始修改代码（25 分钟）
**否**: 您后续自行参考文档执行

---

**您希望我现在立即执行 Step 1 吗？** 🚀

(Step 1 只是添加 PowerManager 初始化，不删除任何代码，风险极低)

