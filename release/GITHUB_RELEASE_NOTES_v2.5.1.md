# AnyWP Engine v2.5.1 - Release Notes

**发布日期**: 2025-11-26
**Flutter Plugin Version**: 2.5.1
**Web SDK Version**: 2.5.0

> **Note**: Flutter Plugin and Web SDK have independent version numbers.

---


## 版本信息
- **引擎版本**: v2.5.1
- **SDK 版本**: v2.5.0 (未更新)

## 🎉 新增功能

### 本地 HTTP 文件服务器
- **功能说明**：新增 `LocalFileServer` 类，用于在本地启动 HTTP 服务器
- **解决问题**：
  - 避免本地文件加载的 CORS 跨域问题
  - 启用需要 HTTP 协议的现代 Web API
  - 支持本地 HTML 壁纸文件的可靠加载
- **使用方式**：
  ```dart
  // 添加依赖到 pubspec.yaml:
  // shelf: ^1.4.1
  // shelf_static: ^1.1.2
  
  final server = LocalFileServer();
  final baseUrl = await server.start('/path/to/files');
  // 访问: $baseUrl/your_wallpaper.html
  await server.stop();
  ```
- **特性**：
  - 自动分配可用端口
  - 自动添加 CORS 头
  - 支持目录浏览
  - 轻量级无额外进程

## 🐛 Bug 修复

### 鼠标事件干扰问题修复
- **问题描述**：当其他程序（如 lxwp.exe）同时运行时，mousedown 后的 mousemove 事件无法传递到 WebView
- **影响**：拖拽功能失效，用户体验受损
- **根本原因**：其他程序的低级鼠标钩子在 mousedown 后阻断了 mousemove 事件的传递链（不调用 CallNextHookEx）
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

