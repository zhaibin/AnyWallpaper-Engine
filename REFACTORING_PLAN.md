# AnyWP Engine 模块化提升重构计划

## 📊 当前状况分析

### 代码规模
- **主插件文件**: `anywp_engine_plugin.cpp` - **3932 行**
- **公共方法数**: 49 个
- **现有模块**: 15 个（`windows/modules/`）
- **现有工具类**: 18 个（`windows/utils/`）

### 模块化率评估
- **已模块化**: ~60%（15个功能模块 + 18个工具类）
- **待优化**: ~40%（主插件文件仍包含大量业务逻辑）

---

## 🎯 重构目标

1. **减少主插件文件行数**: 从 3932 行减少到 **< 2000 行**
2. **提升模块化率**: 从 60% 提升到 **≥ 85%**
3. **提高代码可测试性**: 每个新模块单元测试覆盖率 ≥ 95%
4. **保持向后兼容**: 不破坏现有 API 接口

---

## 📦 新增模块规划

### 1. WebMessageHandler 模块
**目标**: 统一处理所有 WebView2 消息

**功能整合**:
- `HandleWebMessage()` - 消息路由
- `HandleIframeDataWebMessage()` - iframe 数据
- `HandleOpenUrlWebMessage()` - URL 打开请求
- `HandleReadyWebMessage()` - 就绪通知
- `HandleLogWebMessage()` - 日志消息
- `HandleConsoleLogWebMessage()` - 控制台日志
- `HandleSaveStateWebMessage()` - 保存状态
- `HandleLoadStateWebMessage()` - 加载状态
- `HandleClearStateWebMessage()` - 清除状态

**预计减少行数**: ~300 行

---

### 2. WallpaperLifecycleManager 模块
**目标**: 管理壁纸生命周期（暂停/恢复/销毁）

**功能整合**:
- `PauseWallpaper()` - 暂停壁纸
- `ResumeWallpaper()` - 恢复壁纸
- `ValidateWallpaperWindows()` - 验证窗口
- `RestoreWallpaperConfiguration()` - 恢复配置
- `NotifyWebContentVisibility()` - 可见性通知
- `ExecuteScriptToAllInstances()` - 脚本执行

**预计减少行数**: ~400 行

---

### 3. AutoRecoveryManager 模块
**目标**: 自动恢复功能独立管理

**功能整合**:
- `SetAutoRecoveryEnabled()` - 启用/禁用
- `SaveWallpaperConfig()` - 保存配置
- `RemoveWallpaperConfig()` - 移除配置
- `HandleAutoRecovery()` - 执行恢复
- `SaveWallpaperConfigurationManually()` - 手动保存

**当前状态**: 逻辑散布在主插件中
**预计减少行数**: ~200 行

---

### 4. WorkerWRecoveryManager 模块
**目标**: WorkerW 窗口恢复策略管理

**功能整合**:
- `RecoverWorkerW()` - 主恢复入口
- `RecoverWorkerW_Reparent()` - 重新父子关系
- 与 `WorkerWHealthMonitor` 协作

**预计减少行数**: ~250 行

---

### 5. WallpaperConfigurationManager 模块
**目标**: 统一管理壁纸配置参数

**功能整合**:
- `idle_timeout_ms_` - 空闲超时
- `memory_threshold_mb_` - 内存阈值
- `cleanup_interval_minutes_` - 清理间隔
- `auto_power_saving_enabled_` - 自动省电
- 配置验证和持久化

**预计减少行数**: ~150 行

---

### 6. ScriptInjectionManager 模块
**目标**: SDK 注入和脚本管理

**功能整合**:
- `InjectAnyWallpaperSDK()` - SDK 注入
- `LoadSDKScript()` - 加载脚本
- `SetupMessageBridge()` - 消息桥接

**预计减少行数**: ~100 行

---

### 7. PermissionConfigurator 模块
**目标**: WebView2 权限和安全配置

**功能整合**:
- `ConfigurePermissions()` - 配置权限
- `SetupSecurityHandlers()` - 安全处理器
- 与 `PermissionManager` 协作

**预计减少行数**: ~150 行

---

### 8. CacheManager 模块
**目标**: 缓存和清理管理

**功能整合**:
- `ClearWebViewCache()` - 清除缓存
- `PeriodicCleanup()` - 定期清理
- `last_cleanup_` 时间戳管理

**预计减少行数**: ~100 行

---

## 📈 工具类增强

### 9. StateManager 工具类
**目标**: 统一状态管理

**功能**:
- 电源状态追踪
- 会话状态追踪
- 暂停/恢复状态同步

**预计减少行数**: ~100 行

---

### 10. MessageQueueManager 工具类
**目标**: 消息队列管理

**功能整合**:
- `pending_messages_` 队列
- `pending_power_state_changes_` 队列
- `GetPendingMessages()`
- `GetPendingPowerStateChanges()`

**预计减少行数**: ~80 行

---

## 🔧 重构步骤

### Phase 1: 基础架构（第1-2周）
1. ✅ 创建重构分支 `refactor/modularization-improvement`
2. 创建新模块框架（空壳 + 接口定义）
3. 添加单元测试框架

### Phase 2: 消息处理模块化（第3周）
1. 实现 `WebMessageHandler` 模块
2. 迁移所有 WebMessage 处理逻辑
3. 单元测试（覆盖率 ≥ 95%）
4. 集成测试

### Phase 3: 生命周期管理模块化（第4周）
1. 实现 `WallpaperLifecycleManager` 模块
2. 迁移暂停/恢复逻辑
3. 单元测试 + 集成测试

### Phase 4: 恢复机制模块化（第5周）
1. 实现 `AutoRecoveryManager` 模块
2. 实现 `WorkerWRecoveryManager` 模块
3. 单元测试 + 恢复场景测试

### Phase 5: 配置和注入模块化（第6周）
1. 实现 `WallpaperConfigurationManager`
2. 实现 `ScriptInjectionManager`
3. 实现 `PermissionConfigurator`
4. 实现 `CacheManager`
5. 单元测试

### Phase 6: 工具类增强（第7周）
1. 实现 `StateManager`
2. 实现 `MessageQueueManager`
3. 单元测试

### Phase 7: 集成和优化（第8周）
1. 主插件文件重构（移除已迁移代码）
2. 全面集成测试
3. 性能测试和优化
4. 文档更新

### Phase 8: 代码审查和发布（第9周）
1. 代码审查
2. 边缘场景测试
3. 合并到主分支
4. 发布 v2.5.0

---

## 📊 预期成果

### 代码行数优化
- **主插件文件**: 3932 → **< 1800 行**（减少 **54%**）
- **新增模块**: 8 个
- **新增工具类**: 2 个

### 模块化率提升
- **当前**: ~60%
- **目标**: **≥ 85%**
- **提升**: +25 个百分点

### 可测试性提升
- **新模块单元测试覆盖率**: ≥ 95%
- **整体测试覆盖率**: ≥ 85%

### 可维护性提升
- **平均方法行数**: < 50 行
- **平均类行数**: < 300 行
- **模块间耦合度**: 降低 40%

---

## ⚠️ 风险控制

### 兼容性风险
- ✅ 保持所有公共 API 不变
- ✅ 渐进式重构，逐步迁移
- ✅ 每个阶段都进行回归测试

### 性能风险
- ✅ 模块间调用使用内联优化
- ✅ 避免不必要的数据复制
- ✅ 性能基准测试对比

### 稳定性风险
- ✅ 充分的单元测试和集成测试
- ✅ 长时间运行测试（24小时+）
- ✅ 边缘场景覆盖

---

## 📝 成功标准

1. ✅ 主插件文件 < 2000 行
2. ✅ 模块化率 ≥ 85%
3. ✅ 单元测试覆盖率 ≥ 95%（新模块）
4. ✅ 性能损失 < 5%
5. ✅ 所有现有功能保持 100% 兼容
6. ✅ 无新增 Linter 警告
7. ✅ 文档完整更新

---

**创建时间**: 2025-11-19  
**目标版本**: v2.5.0  
**预计完成**: 2026-01-15

