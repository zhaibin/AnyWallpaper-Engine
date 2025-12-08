# AnyWP Engine v2.6.0 - Release Notes

**发布日期**: 2025-12-05
**Flutter Plugin Version**: 2.6.0
**Web SDK Version**: 2.5.0

> **Note**: Flutter Plugin and Web SDK have independent version numbers.

---


## 🌍 macOS 平台支持 (合并 v2.2.0 分支)

### 新增功能
- ✅ **macOS 原生插件**: 基于 Objective-C + AppKit + WKWebView
- ✅ **统一 API**: Windows 和 macOS 使用相同的 Dart API
- ✅ **模块化架构**: MonitorManager, WallpaperManager, PowerManager, MessageBridge
- ✅ **JavaScript SDK**: 平台无关的 SDK，自动适配 WKWebView 消息传递
- ✅ **多显示器支持**: NSScreen API 实现
- ✅ **电源管理**: NSWorkspace 通知集成（屏幕休眠、会话锁定、空闲检测）
- ✅ **状态持久化**: 使用 Application Support 目录存储
- ✅ **内存优化**: 针对 WKWebView 调整阈值（200MB 默认）
- ✅ **文件加密/解密**: 支持 XOR 加密的文件读写

### 文档更新
- ✅ **跨平台集成指南**: `docs/CROSS_PLATFORM_INTEGRATION.md`
- ✅ **macOS 开发者指南**: `docs/MACOS_DEVELOPER_GUIDE.md`
- ✅ **macOS 预编译包集成**: `docs/PRECOMPILED_MACOS_INTEGRATION.md`
- ✅ **macOS 发布流程**: `docs/MACOS_RELEASE_GUIDE.md`
- ✅ **多平台架构设计**: `docs/MULTIPLATFORM_ARCHITECTURE.md`

### 构建脚本
- ✅ **macOS 发布脚本**: `scripts/release_macos.sh`
- ✅ **macOS SDK 构建**: `scripts/build_sdk.sh`
- ✅ **macOS 包验证**: `scripts/verify_precompiled_macos.sh`

### 技术亮点
- **Windows**: WebView2 (Chromium) + Win32 API + C++17
- **macOS**: WKWebView (WebKit) + AppKit + Objective-C
- **共享**: Dart API 层 + TypeScript SDK + 统一消息协议

### 已知限制
- macOS 交互模式（Interactive Mode）暂未实现，需要 Accessibility 权限
- WKWebView 文件访问受沙箱限制
- WKWebView 内存使用通常高于 WebView2 (150-200MB vs 100-150MB)

## 📦 集成 Windows 平台改进 (v2.5.1)

### 本地 HTTP 文件服务器
- 新增 `LocalFileServer` 类，解决 CORS 跨域问题
- 自动分配端口、添加 CORS 头、支持目录浏览

### 鼠标事件干扰问题修复
- 修复其他程序钩子干扰导致的拖拽失效
- 实现多线程轮询 + UI 定时器的双重备份机制

### MouseHookManager 增强
- 新增轮询备份功能（可配置）
- 线程安全队列和原子状态管理

---

- **解决方案**：实现多线程轮询 + UI 定时器的双重备份机制
  - mousedown 时启动后台轮询线程（16ms 间隔，约 60fps）
  - 轮询线程通过 `GetCursorPos()` 获取鼠标位置并将事件放入队列
  - UI 线程定时器（`SetTimer`）定期从队列取出事件并发送到 WebView
  - 仅当检测到钩子超过 50ms 未收到 mousemove 时才启用轮询
  - mouseup 时停止轮询线程和定时器

## 🔧 技术改进

### MouseHookManager 增强
- **新增 `SetPollingFallbackEnabled(bool)`**：启用/禁用轮询备份功能
- **新增 `SetPollingInterval(UINT)`**：设置轮询间隔（毫秒）
- **新增 `SetTimerWindow(HWND)`**：设置 UI 定时器的窗口句柄
- **新增内部方法**：
  - `StartPollingThread()` / `StopPollingThread()`：轮询线程生命周期管理
  - `PollingThreadFunc()`：后台轮询线程主函数
  - `ProcessPendingPolledEvents()`：在 UI 线程处理队列中的事件
  - `UITimerProc()`：UI 定时器回调函数
- **线程安全队列**：使用 `std::mutex` 保护事件队列
- **原子状态**：使用 `std::atomic` 管理线程状态和时间戳

## 📝 备注
- 轮询机制仅在检测到钩子被干扰时激活，正常情况下不会增加 CPU 开销
- 使用 UI 定时器确保事件在正确的线程发送到 WebView（WebView2 要求）
- 此修复对所有使用全局鼠标钩子的第三方程序都有效

---

