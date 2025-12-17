# AnyWP Engine v2.6.7 - Release Notes

**发布日期**: 2025-12-17
**Flutter Plugin Version**: 2.6.7
**Web SDK Version**: 2.5.0

> **Note**: Flutter Plugin and Web SDK have independent version numbers.

---

## 🐛 Bug 修复

### 修复 Windows 10 退出白屏和启动白屏闪烁问题

**问题**: 
- Windows 10 上关闭应用程序后，桌面残留白色窗口
- Windows 10 上启动壁纸时出现短暂白屏闪烁
- Windows 11 上偶发退出后窗口未完全清理

**根本原因**:
- Windows 10 的 DWM (Desktop Window Manager) 在窗口销毁时机与 Windows 11 不同
- WebView2 子窗口的销毁顺序影响父窗口的可见性
- 窗口创建时默认可见，导致在 WebView2 加载前显示白色背景

**修复内容**:

**退出白屏修复**:
- 在 `StopWallpaper` 最开始**立即隐藏所有窗口**
- 使用 `FindWindowW` 循环查找所有 `AnyWallpaperHost` 窗口
- 隐藏窗口后修改标题为空，防止重复查找
- 增强 `ResourceTracker::CleanupAll` 的孤儿窗口清理

**启动白屏修复**:
- 创建窗口时**不使用** `WS_VISIBLE`，先设置透明再显示
- 设置 WebView2 背景为透明（ARGB 0,0,0,0）
- **导航完成后**才将窗口 alpha 设为 255

**技术细节**:

```cpp
// 退出时立即隐藏所有窗口（防止白屏残留）
HWND hwnd = NULL;
while ((hwnd = FindWindowW(L"AnyWallpaperHost", NULL)) != NULL) {
    ShowWindow(hwnd, SW_HIDE);
    SetWindowTextW(hwnd, L"");  // 防止重复找到
}

// 启动时窗口创建为透明（防止白屏闪烁）
// 1. 创建时 alpha=0
SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
// 2. 导航完成后 alpha=255
SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
```

**影响文件**:
- `windows/anywp_engine_plugin.cpp` - 窗口生命周期管理
- `windows/modules/webview_manager.cpp` - WebView2 背景透明设置
- `windows/modules/window_manager.cpp` - 窗口创建逻辑
- `windows/utils/resource_tracker.cpp` - 孤儿窗口清理机制

**测试结果**:
- ✅ Windows 10: 退出后无白屏残留
- ✅ Windows 10: 启动无白屏闪烁
- ✅ Windows 11: 功能正常，无回归问题

---

### 简化窗口清理逻辑

**修改内容**:
- 移除过于激进的立即隐藏逻辑（避免影响 Windows 11）
- 简化多监视器和单监视器的清理代码
- 保留核心逻辑：先隐藏窗口，再销毁
- 如果 `DestroyWindow` 失败，执行备选方案：
  - `SetParent(HWND_MESSAGE)` - 解除父子关系
  - 移动窗口到屏幕外
  - 设置完全透明

---

## ⚙️ 技术改进

### WorkerW 健康监控优化

- 改进健康监控线程的生命周期管理
- 修复监控重复启动/停止的问题
- 优化 Explorer 进程 ID 检测逻辑

---

## 📦 发布包

| 包名 | 说明 | 适用用户 |
|------|------|----------|
| `anywp_engine_v2.6.7_precompiled.zip` | Windows 预编译包 | Flutter 开发者（推荐） |
| `anywp_engine_v2.6.7_source.zip` | Windows 源码包 | 需要自定义修改的开发者 |
| `anywp_web_sdk_v2.5.0.zip` | Web SDK 包 | HTML 壁纸开发者 |

> macOS 预编译包将在 macOS 机器上构建后补充上传。

---

## 升级指南

直接替换旧版本即可，无需修改代码。

**预编译包用户**:
```bash
# 替换 bin/ 和 lib/ 目录下的文件
```

**源码用户**:
```bash
# 重新编译插件
flutter clean
flutter pub get
flutter build windows
```

---

**完整更新日志**: [CHANGELOG_CN.md](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md)

