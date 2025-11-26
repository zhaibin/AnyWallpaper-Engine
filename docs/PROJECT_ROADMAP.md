# AnyWP Engine 项目分析与路线图

> **版本**: v2.5.1 | **更新时间**: 2025-11-26

---

## 📊 项目概览

### 基本信息

| 项目 | 说明 |
|------|------|
| **名称** | AnyWP Engine |
| **类型** | Flutter Windows 插件 |
| **当前版本** | 引擎 v2.5.1 / SDK v2.5.0 |
| **核心功能** | 将 WebView2 嵌入为交互式桌面壁纸 |
| **技术栈** | Flutter 3.0+ / C++17 / WebView2 / TypeScript |
| **目标平台** | Windows 10/11 |

### 核心特性

```
┌─────────────────────────────────────────────────────────────────┐
│                    AnyWP Engine 特性矩阵                        │
├─────────────────────────────────────────────────────────────────┤
│  🖼️ WebView2 壁纸      │  📺 多显示器支持      │  🔄 自动恢复     │
│  🖱️ 鼠标穿透/交互     │  ⌨️ 键盘监听         │  🔌 热插拔显示器  │
│  🔋 智能省电          │  💾 状态持久化       │  🎮 双向通信     │
│  🔒 安全沙箱          │  📊 性能监控         │  🧪 98.5%测试覆盖 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🏗️ 架构分析

### 模块统计

| 类别 | 数量 | 说明 |
|------|------|------|
| **核心模块** (`modules/`) | 25 | 功能模块 |
| **工具类** (`utils/`) | 21 | 通用工具 |
| **SDK 模块** (`sdk/src/`) | 8 | TypeScript 模块 |
| **单元测试** | 209+ | 测试用例 |
| **测试覆盖率** | 98.5% | 代码覆盖 |

### C++ 核心模块 (25个)

```
windows/modules/
├── 壁纸核心
│   ├── instance_manager.cpp        # 壁纸实例管理
│   ├── wallpaper_lifecycle_manager # 生命周期管理
│   ├── wallpaper_configuration_manager # 配置管理
│   └── window_manager.cpp          # 窗口创建/Z序管理
│
├── WebView2 管理
│   ├── webview_manager.cpp         # WebView2 生命周期
│   ├── webview_configurator.cpp    # WebView2 配置
│   ├── script_injection_manager.cpp # SDK 注入
│   └── web_message_handler.cpp     # 消息处理
│
├── 事件系统
│   ├── mouse_hook_manager.cpp      # 鼠标钩子（含轮询备份）
│   ├── keyboard_hook_manager.cpp   # 键盘钩子
│   ├── event_dispatcher.cpp        # 事件分发（O(1)查找）
│   └── iframe_detector.cpp         # iframe 边界检测
│
├── 显示器管理
│   ├── monitor_manager.cpp         # 多显示器枚举
│   ├── display_change_coordinator.cpp # 显示变更协调
│   └── initialization_coordinator.cpp # 初始化协调
│
├── 恢复与健康
│   ├── auto_recovery_manager.cpp   # Explorer 重启恢复
│   ├── workerw_recovery_manager.cpp # WorkerW 恢复
│   └── workerw_health_monitor.cpp  # WorkerW 健康监控
│
├── 性能与电源
│   ├── power_manager.cpp           # 省电优化
│   ├── memory_optimizer.cpp        # 内存优化
│   └── cache_manager.cpp           # 缓存管理
│
├── 桥接层
│   ├── flutter_bridge.cpp          # Flutter MethodChannel
│   └── sdk_bridge.cpp              # JavaScript SDK 桥接
│
└── 其他
    ├── custom_scheme_handler.cpp   # anywp:// 协议处理
    └── permission_configurator.cpp # 权限配置
```

### C++ 工具类 (21个)

```
windows/utils/
├── 核心工具
│   ├── logger.cpp                  # 统一日志系统
│   ├── error_handler.cpp           # 统一错误处理
│   ├── config_manager.cpp          # 配置管理
│   └── service_locator.cpp         # 依赖注入
│
├── 安全工具
│   ├── input_validator.cpp         # 输入验证
│   ├── url_validator.cpp           # URL 验证
│   └── permission_manager.cpp      # 权限管理
│
├── 性能工具
│   ├── cpu_profiler.cpp            # CPU 分析
│   ├── memory_profiler.cpp         # 内存分析
│   ├── performance_benchmark.cpp   # 性能基准
│   └── startup_optimizer.cpp       # 启动优化
│
├── 状态管理
│   ├── state_persistence.cpp       # 持久化存储
│   ├── state_manager.cpp           # 状态管理
│   └── message_queue_manager.cpp   # 消息队列
│
├── 辅助工具
│   ├── event_bus.cpp               # 事件总线
│   ├── resource_tracker.cpp        # 资源追踪
│   ├── conflict_detector.cpp       # 冲突检测
│   ├── desktop_wallpaper_helper.cpp # 桌面壁纸辅助
│   └── mime_type_detector.cpp      # MIME 类型检测
│
└── Header-Only
    ├── circuit_breaker.h           # 熔断器
    ├── retry_handler.h             # 重试处理
    └── safety_macros.h             # 安全宏
```

### TypeScript SDK 模块 (8个)

```
sdk/src/
├── core/
│   ├── AnyWP.ts                    # 主 API 类
│   └── init.ts                     # 初始化逻辑
├── modules/
│   ├── animations.ts               # 动画控制
│   ├── click.ts                    # 点击事件
│   ├── drag.ts                     # 拖拽支持
│   ├── events.ts                   # 事件系统
│   ├── file.ts                     # 文件操作
│   ├── spa.ts                      # SPA 支持
│   ├── storage.ts                  # 状态存储
│   ├── wallpaper.ts                # 壁纸控制
│   └── webmessage.ts               # 消息通信
└── utils/
    ├── bounds.ts                   # 边界计算
    ├── coordinates.ts              # 坐标转换
    ├── debug.ts                    # 调试工具
    ├── logger.ts                   # 日志工具
    └── throttle.ts                 # 节流工具
```

---

## ✅ 已完成功能 (v1.0 - v2.5.1)

### 核心功能

| 版本 | 功能 | 状态 |
|------|------|------|
| v1.0 | WebView2 基础集成 | ✅ |
| v1.0 | 壁纸 Z 序管理（图标后方） | ✅ |
| v1.0 | 鼠标穿透模式 | ✅ |
| v1.2 | 多显示器支持 | ✅ |
| v1.2 | 应用级存储隔离 | ✅ |
| v1.3 | 热插拔显示器支持 | ✅ |
| v2.0 | 模块化架构重构 | ✅ |
| v2.1 | 双向通信 (Flutter ↔ JS) | ✅ |
| v2.3 | 自动恢复（Explorer 重启） | ✅ |
| v2.4 | 恢复回调（状态恢复） | ✅ |
| v2.5 | 键盘监听 | ✅ |
| v2.5.1 | 本地 HTTP 文件服务器 | ✅ |
| v2.5.1 | 鼠标事件轮询备份机制 | ✅ |

### 性能与稳定性

| 功能 | 状态 | 说明 |
|------|------|------|
| 智能省电 | ✅ | 锁屏/空闲/全屏时自动暂停 |
| 即时恢复 | ✅ | <50ms 恢复时间 |
| 事件分发优化 | ✅ | O(1) 查找，<5ms 延迟 |
| 内存优化 | ✅ | 自动清理，阈值监控 |
| 统一日志系统 | ✅ | 多级别、线程安全、文件输出 |
| 统一错误处理 | ✅ | 自动重试、熔断器、错误统计 |
| 单元测试 | ✅ | 209+ 用例，98.5% 覆盖率 |

### JavaScript SDK

| 功能 | 状态 | API |
|------|------|-----|
| 自动注入 | ✅ | 无需手动加载 |
| 状态持久化 | ✅ | `saveState()` / `loadState()` |
| 可见性检测 | ✅ | `onVisibilityChange()` |
| 点击事件 | ✅ | `onClick()` |
| 拖拽支持 | ✅ | `onDragStart/End()` |
| 双向通信 | ✅ | `sendToFlutter()` / `onMessage()` |
| 键盘事件 | ✅ | `onKeyDown()` / `onKeyUp()` |
| 动画控制 | ✅ | `pauseAnimations()` / `resumeAnimations()` |

---

## ⚠️ 已知问题与限制

### 1. 全屏应用场景 (v2.1.7+)

| 问题 | 说明 |
|------|------|
| **现象** | 全屏应用覆盖壁纸时，无法通知 JS 暂停 |
| **原因** | WebView2 的 `ExecuteScript` 回调在窗口被完全覆盖时被阻塞 |
| **影响** | 壁纸在全屏时继续消耗资源（虽然不可见） |
| **临时方案** | 手动调用 `pauseWallpaper()` 或关闭应用 |
| **优先级** | 🟡 中等 |

### 2. 代码中的 TODO/FIXME (14处)

```
windows/anywp_engine_plugin.cpp:        1 处
windows/modules/power_manager.cpp:      2 处
windows/modules/permission_configurator: 3 处
windows/modules/script_injection_manager: 1 处
windows/modules/wallpaper_lifecycle_manager: 1 处
windows/modules/wallpaper_configuration_manager: 2 处
windows/modules/cache_manager.cpp:      1 处
windows/utils/state_persistence.cpp:    2 处
windows/utils/input_validator.cpp:      1 处
```

### 3. 潜在优化点

| 项目 | 当前状态 | 改进方向 |
|------|---------|---------|
| WebView2 缓存 | 基础实现 | 更智能的缓存策略 |
| 内存管理 | 阈值监控 | 按需加载、内存压缩 |
| 启动时间 | ~530ms | 延迟初始化优化 |

---

## 🗺️ 路线图 (Roadmap)

### 📅 短期计划 (v2.6 - Q1 2026)

#### v2.6.0 - 交互增强

| 功能 | 优先级 | 说明 |
|------|--------|------|
| **自定义透明度** | 🔴 高 | 支持 0-100% 透明度调节 |
| **全屏应用检测优化** | 🔴 高 | 改进全屏状态下的 JS 通知机制 |
| **手势支持** | 🟡 中 | 多点触控、缩放、旋转手势 |
| **右键菜单** | 🟡 中 | 壁纸右键上下文菜单 |

#### v2.7.0 - 多媒体支持

| 功能 | 优先级 | 说明 |
|------|--------|------|
| **音频可视化 API** | 🔴 高 | 系统音频可视化数据接口 |
| **视频壁纸优化** | 🔴 高 | 硬件加速、低功耗视频播放 |
| **音量控制** | 🟡 中 | 壁纸音频音量控制 |
| **GIF/WebP 支持** | 🟢 低 | 原生动图支持 |

### 📅 中期计划 (v3.0 - Q2-Q3 2026)

#### v3.0.0 - 平台扩展

| 功能 | 优先级 | 说明 |
|------|--------|------|
| **系统托盘集成** | 🔴 高 | 最小化到托盘、快捷控制 |
| **开机自启动** | 🔴 高 | 系统启动时自动运行 |
| **壁纸预览** | 🟡 中 | 设置前预览效果 |
| **壁纸切换动画** | 🟡 中 | 淡入淡出、滑动等过渡效果 |
| **定时切换** | 🟡 中 | 按时间/事件自动切换壁纸 |

#### v3.1.0 - 插件系统

| 功能 | 优先级 | 说明 |
|------|--------|------|
| **插件架构** | 🔴 高 | 动态加载插件 DLL |
| **效果插件 API** | 🔴 高 | 自定义视觉效果接口 |
| **数据源插件** | 🟡 中 | 天气、股票、新闻等数据源 |
| **插件市场** | 🟢 低 | 在线插件发现与安装 |

### 📅 长期计划 (v4.0+ - 2027)

#### v4.0.0 - 生态系统

| 功能 | 优先级 | 说明 |
|------|--------|------|
| **壁纸画廊/市场** | 🔴 高 | 在线壁纸分享与下载 |
| **壁纸创作工具** | 🟡 中 | 可视化壁纸编辑器 |
| **云同步** | 🟡 中 | 跨设备壁纸配置同步 |
| **社区平台** | 🟢 低 | 用户社区、评分、评论 |

#### v4.1.0 - 高级功能

| 功能 | 优先级 | 说明 |
|------|--------|------|
| **AI 壁纸生成** | 🟡 中 | 集成 AI 图像生成 |
| **桌面小组件** | 🟡 中 | 可拖拽的桌面小工具 |
| **3D 壁纸支持** | 🟢 低 | WebGL 3D 场景壁纸 |
| **VR/AR 桌面** | 🟢 低 | 虚拟现实桌面体验 |

---

## 📊 版本里程碑

```
v1.0 ─────────────────────────────────────────────────────────────►
      │ WebView2 基础 │ 鼠标穿透 │ 基本壁纸功能
      
v2.0 ─────────────────────────────────────────────────────────────►
      │ 模块化架构 │ 多显示器 │ 省电优化 │ 双向通信
      
v2.5 ─────────────────────────────────────────────────────────────► 当前
      │ 自动恢复 │ 键盘监听 │ HTTP服务器 │ 轮询备份

v3.0 ─────────────────────────────────────────────────────────────► 计划
      │ 系统托盘 │ 插件系统 │ 音频可视化 │ 壁纸预览
      
v4.0 ─────────────────────────────────────────────────────────────► 远期
      │ 壁纸市场 │ 云同步 │ AI生成 │ 3D支持
```

---

## 🔧 技术债务清理计划

### 高优先级

| 项目 | 位置 | 说明 |
|------|------|------|
| 清理 TODO/FIXME | 14 处 | 处理遗留的代码标记 |
| 全屏检测优化 | power_manager | 改进全屏应用检测逻辑 |
| 权限配置完善 | permission_configurator | 完成权限系统实现 |

### 中优先级

| 项目 | 位置 | 说明 |
|------|------|------|
| 状态持久化优化 | state_persistence | 改进存储效率 |
| 缓存策略优化 | cache_manager | 智能缓存淘汰 |
| 脚本注入优化 | script_injection_manager | 减少注入延迟 |

### 低优先级

| 项目 | 说明 |
|------|------|
| 代码注释完善 | 补充关键逻辑注释 |
| 文档更新 | 同步最新 API 文档 |
| 示例代码更新 | 添加更多使用示例 |

---

## 📈 质量指标目标

| 指标 | 当前 | v3.0 目标 | v4.0 目标 |
|------|------|----------|----------|
| 测试覆盖率 | 98.5% | ≥99% | ≥99.5% |
| 代码重复率 | <5% | <3% | <2% |
| TODO/FIXME | 14 | 0 | 0 |
| 文档完整度 | 85% | 95% | 100% |
| Bug 修复周期 | ~3天 | ~2天 | ~1天 |

---

## 🎯 开发优先级原则

```
1. 稳定性 > 新功能
   - 先修复已知问题，再添加新特性
   
2. 性能 > 功能丰富度
   - 保持低资源占用是核心竞争力
   
3. 用户体验 > 技术实现
   - 以用户需求为导向，而非技术驱动
   
4. 向后兼容 > 激进重构
   - 确保现有用户无缝升级
```

---

## 📝 贡献指南

### 新功能开发流程

```
1. 创建 Issue 讨论需求
2. Fork 仓库并创建特性分支
3. 实现功能 + 单元测试（≥95%覆盖率）
4. 更新文档和 CHANGELOG
5. 提交 PR 并等待 Review
6. 合并后更新版本号
```

### 代码质量要求

- [ ] 所有公共 API 有文档注释
- [ ] 关键操作有 try-catch 保护
- [ ] 使用 `Logger::Instance()` 记录日志
- [ ] 单元测试覆盖率 ≥95%
- [ ] 无 Linter 警告或错误

---

**文档版本**: v1.0  
**创建时间**: 2025-11-26  
**维护者**: AnyWP Engine Team

