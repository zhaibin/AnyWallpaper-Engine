# 更新日志

所有重要的项目变更都将记录在此文件中。

## [4.0.0] - 2025-11-03 - 🎉 重大更新：React/Vue SPA 完整支持

### ✨ 新增功能
- **SPA 框架自动检测**：自动识别 React、Vue、Angular 应用
- **智能路由监听**：自动监听 pushState/replaceState/popstate 事件
- **DOM 变化监听**：使用 MutationObserver 自动检测动态内容变化
- **元素自动重新挂载**：SPA 路由切换后自动重新绑定点击区域
- **ResizeObserver 集成**：自动跟踪元素尺寸和位置变化
- **等待元素出现**：`waitFor` 选项支持异步加载的动态元素
- **手动边界刷新**：提供 `refreshBounds()` API 手动刷新所有点击区域
- **处理器清理**：提供 `clearHandlers()` API 清理所有注册的点击处理器
- **React/Vue 生命周期辅助**：`useReactEffect()` 和 `vueLifecycle()` 辅助函数

### 🔧 API 改进
- `onClick()` 新增选项：
  - `immediate`: 立即注册（不延迟）
  - `waitFor`: 等待元素出现（默认 true）
  - `maxWait`: 最大等待时间（默认 10000ms）
  - `autoRefresh`: 自动刷新边界（默认 true）
  - `delay`: 自定义延迟时间（默认 100ms）
- 移除硬编码的 2000ms 延迟
- 支持自动检测并启用 SPA 模式
- 支持手动启用/禁用 SPA 模式：`setSPAMode(enabled)`

### 📚 文档
- 新增 **Web 开发者集成指南**（中英文）
  - `docs/WEB_DEVELOPER_GUIDE_CN.md`
  - `docs/WEB_DEVELOPER_GUIDE.md`
- 详细的 React 集成最佳实践
- 详细的 Vue 2/3 集成最佳实践
- SPA 路由处理指南
- 性能优化建议
- 调试技巧和常见问题解答

### 📝 示例
- 新增 `examples/test_react.html`：完整的 React 集成示例
  - Counter 组件
  - 可点击的卡片
  - 事件日志
  - SPA 模式展示
- 新增 `examples/test_vue.html`：完整的 Vue 3 集成示例
  - 多标签页切换
  - Todo List 应用
  - Counter 组件
  - 动态内容处理

### 🐛 修复
- 修复 SPA 路由切换后点击区域失效的问题
- 修复动态内容加载后点击区域不准确的问题
- 修复元素重新挂载后点击检测失败的问题
- 改进调试边框的更新和清理逻辑

### ⚡ 性能优化
- 使用 ResizeObserver 替代定时轮询
- 优化 DOM 变化监听，只在必要时刷新
- 路由切换后智能延迟刷新（500ms）
- 减少不必要的边界计算

### 🔄 向后兼容
- 完全向后兼容 v3.x API
- 旧代码无需修改即可继续工作
- 新功能默认启用，可选退出

---

## [3.1.3] - 2025-11-01 - 文档增强：打包与集成指南

### 📦 新增文档

#### 新增：完整的打包和使用指南

**新增文件**:
1. **[QUICK_INTEGRATION.md](QUICK_INTEGRATION.md)** - 30秒快速集成指南
   - 三种集成方式（本地路径、Git、pub.dev）
   - 完整代码示例
   - 常用场景速查

2. **[docs/PACKAGE_USAGE_GUIDE_CN.md](docs/PACKAGE_USAGE_GUIDE_CN.md)** - 详细打包使用文档
   - 三种集成方式详细对比
   - 本地路径引用完整说明
   - Git 仓库引用完整流程
   - pub.dev 发布详细步骤
   - 完整功能示例代码
   - API 参考文档
   - 故障排除指南

**更新文件**:
- **[README.md](README.md)** - 添加集成指南链接

### ✨ 主要内容

#### 1. 三种集成方式

| 方式 | 适用场景 | 配置方法 |
|------|---------|---------|
| **本地路径** | 开发测试 | `path: ../AnyWP_Engine` |
| **Git 仓库** | 团队协作 | `git: url + ref` |
| **pub.dev** | 生产环境 | `anywp_engine: ^1.0.0` |

#### 2. 完整使用示例

提供了以下示例代码：
- 最小化示例（10行代码启动壁纸）
- 完整功能示例（带UI控制器）
- 常用场景示例（透明、交互、本地文件）

#### 3. API 完整参考

- `initializeWallpaper()` - 详细参数说明
- `stopWallpaper()` - 清理和停止
- `navigateToUrl()` - 动态导航

#### 4. 故障排除

- WebView2 Runtime 安装
- 路径格式问题
- 权限问题
- 调试技巧

---

## [3.1.2] - 2025-11-01 - 安全增强与调试优化

### 🔒 安全增强

#### URL Blacklist System
- 新增 URL 黑名单功能，防止恶意网页访问敏感系统路径
- 默认封禁：`file:///c:/windows`, `file:///c:/program files` 及子目录

#### Security Logger
- 新增完整的安全日志系统：
  - Navigation attempts
  - Permission requests (camera/microphone/location)
  - Content security policy violations
- 所有安全事件自动记录到日志

### 🐛 Bug Fixes

#### Mouse Hook 稳定性
- 修复在大型监视器（3840x2160）上的鼠标钩子崩溃
- 修复偶发性内存泄漏和访问违例
- 完善线程安全处理

#### Resource Management
- 新增 ResourceTracker 统一管理所有窗口句柄
- 自动清理未释放的资源
- 改进析构函数清理逻辑

### 🔧 改进

#### Navigation Safety
- 所有 URL 导航前进行黑名单验证
- 拒绝访问的 URL 会写入日志并通知用户
- 支持子路径匹配（如 `/windows/system32`）

#### Display Change Monitoring
- 新增显示器变化监听器
- 支持分辨率变化、显示器添加/移除等事件
- Flutter 端可注册回调响应显示器变化

### 📊 性能

- 优化鼠标钩子性能（避免频繁坐标转换）
- 减少不必要的日志输出（非调试模式）
- 改进窗口创建和销毁流程

---

## [3.1.1] - 2025-10-31 - 最终优化：无缝壁纸体验

### ✨ 重大改进

#### 完美的桌面集成
- **智能 Z-Order 排序**：壁纸窗口自动定位到桌面图标正下方
  - 定位到 `SHELLDLL_DefView` 和 `WorkerW` 之间
  - 使用 `SWP_NOSENDCHANGING | SWP_NOACTIVATE` 避免闪烁
  - 自动重试机制确保成功

#### 鼠标点击穿透优化
- **智能点击区域管理**：
  - 桌面图标区域完全可点击（启用穿透）
  - 网页互动区域精确响应（禁用穿透）
  - 自动根据点击坐标动态切换
- **性能优化**：
  - 使用低级鼠标钩子（`WH_MOUSE_LL`）
  - 最小化性能开销
  - 仅在必要时调用 `SetWindowLongPtr`

#### 窗口管理改进
- **独立子窗口架构**：
  - WebView2 托管在独立子窗口中
  - 使用 `WS_CHILD | WS_VISIBLE` 样式
  - 支持多显示器（每个显示器独立子窗口）
- **生命周期管理**：
  - 完整的创建/销毁流程
  - 资源自动清理
  - 内存泄漏防护

### 🔧 技术实现

#### Mouse Hook System
```cpp
// 全局鼠标钩子
static HHOOK mouseHook_;
// 动态窗口透明度管理
LRESULT CALLBACK LowLevelMouseProc(...)
```

#### Z-Order Management
```cpp
// 智能定位算法
FindDesktopWindow()
SetWindowPos(..., HWND_BOTTOM - 1)
```

### 📝 API 不变
- 保持 Flutter API 完全兼容
- 无需修改现有代码
- 无缝升级体验

### 🐛 已知问题修复
- ✅ 修复桌面图标无法点击（完全解决）
- ✅ 修复窗口 Z-order 错误（完全解决）
- ✅ 修复资源泄漏问题
- ✅ 修复多显示器支持

---

## [3.1.0] - 2025-10-30 - 多显示器支持 + 完整 API

### 🎨 新增功能

#### 🖥️ 完整的多显示器支持
- **显示器枚举**：
  ```dart
  List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();
  ```
- **独立壁纸控制**：每个显示器可以显示不同的网页壁纸
- **显示器信息**：
  - 索引、名称（如 `\\.\DISPLAY1`）
  - 分辨率（宽度 x 高度）
  - 位置（x, y 坐标）
  - 是否为主显示器

#### 🔄 灵活的壁纸管理
- **多显示器模式**：
  ```dart
  // 在显示器 0 上启动壁纸
  await AnyWPEngine.initializeWallpaper(
    url: 'https://example.com',
    monitorIndex: 0,
  );
  
  // 在显示器 1 上启动另一个壁纸
  await AnyWPEngine.initializeWallpaper(
    url: 'https://another.com',
    monitorIndex: 1,
  );
  ```

- **单一模式（向后兼容）**：
  ```dart
  // 不指定 monitorIndex，使用主显示器
  await AnyWPEngine.initializeWallpaper(
    url: 'https://example.com',
  );
  ```

#### 🌐 独立 URL 导航
- 每个显示器的壁纸可独立导航到不同 URL：
  ```dart
  await AnyWPEngine.navigateToUrl(
    url: 'https://newpage.com',
    monitorIndex: 0,
  );
  ```

### 🔧 API 改进

#### 新增方法
- `getMonitors()` → `List<MonitorInfo>`
  - 返回所有显示器信息
  - 包含分辨率、位置、是否为主显示器

#### 参数增强
所有主要方法新增 `monitorIndex` 可选参数：
- `initializeWallpaper(..., monitorIndex: int?)`
- `stopWallpaper(monitorIndex: int?)`
- `navigateToUrl(..., monitorIndex: int?)`

#### 新增数据模型
```dart
class MonitorInfo {
  final int index;
  final String name;
  final int width;
  final int height;
  final int x;
  final int y;
  final bool isPrimary;
}
```

### 📱 示例应用升级

#### 新增功能
- 显示器列表 UI
- 每个显示器独立控制面板
- 每个显示器独立 URL 输入框
- "Start All" / "Stop All" 批量操作
- 实时显示器状态指示

### 🐛 Bug Fixes
- 修复多显示器环境下坐标计算问题
- 修复显示器索引不匹配的问题
- 改进窗口定位算法

### 📚 文档更新
- 所有文档更新为多显示器 API
- 新增多显示器使用示例
- API 参考完全更新

---

## [3.0.0] - 2025-10-30 - API 重构 + 多显示器支持

### 🚀 重大变更

#### API 完全重构
**旧版 API (弃用)**:
```dart
// ❌ 已弃用
await AnyWPEngine.startWallpaper(url);
await AnyWPEngine.setMouseTransparent(true);
```

**新版 API**:
```dart
// ✅ 推荐使用
await AnyWPEngine.initializeWallpaper(
  url: url,
  isMouseTransparent: true,  // 默认 true
);
```

#### 命名一致性
- `startWallpaper` → `initializeWallpaper`
- `setMouseTransparent` → 合并到 `initializeWallpaper` 参数中
- 参数全部使用命名参数

### ✨ 新增功能

#### 1. 多显示器支持（初步）
- C++ 层新增显示器枚举 API
- 支持获取所有显示器信息
- 为每个显示器独立创建壁纸窗口

#### 2. 简化的初始化流程
```dart
// 一行代码启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  isMouseTransparent: true,
);
```

#### 3. 更好的错误处理
- 所有方法返回明确的错误信息
- 参数验证更严格

### 🔧 内部改进

#### C++ 架构优化
- 重构 Plugin 类，更清晰的方法命名
- 改进显示器管理逻辑
- 优化资源管理和清理

#### Flutter 层优化
- 简化 MethodChannel 调用
- 统一错误处理逻辑
- 改进日志输出

### 📚 文档更新
- 所有示例代码更新为新 API
- 添加 API 迁移指南
- 完善注释文档

### ⚠️ Breaking Changes
- 必须使用新的 `initializeWallpaper` API
- `setMouseTransparent` 不再作为独立方法
- 所有参数改为命名参数

### 🔄 向后兼容
- 旧 API 标记为 `@Deprecated` 但仍可使用
- 提供详细的迁移指南
- 建议在 v4.0.0 前完成迁移

---

## [2.0.0] - 2025-10-29 - 完整功能实现

### ✨ 核心功能

#### WebView2 集成
- 完整的 WebView2 Runtime 支持
- 异步初始化流程
- 环境复用机制

#### 桌面壁纸模式
- 窗口定位到桌面图标后方
- 支持鼠标穿透（桌面图标可点击）
- 支持交互模式（网页可交互）

#### JavaScript 桥接
- `anywp_sdk.js` 自动注入
- 点击事件处理
- URL 打开功能
- 鼠标/键盘事件支持

### 🎯 API

#### Dart API
```dart
await AnyWPEngine.startWallpaper(url);
await AnyWPEngine.stopWallpaper();
await AnyWPEngine.setMouseTransparent(true);
await AnyWPEngine.navigateToUrl(url);
```

#### JavaScript API
```javascript
AnyWP.ready('My Wallpaper');
AnyWP.onClick(element, callback);
AnyWP.openURL(url);
AnyWP.onMouse(callback);
AnyWP.onKeyboard(callback);
```

### 📱 示例应用
- 完整的 UI 控制面板
- URL 输入和导航
- 鼠标模式切换
- 常用场景按钮

### 📝 测试 HTML
- `test_simple.html` - 基础功能
- `test_api.html` - 完整 API 演示
- `test_iframe_ads.html` - 复杂场景测试

---

## [1.0.0] - 2025-10-28 - 初始发布

### 🎉 初始功能
- Windows 平台支持
- 基础的 WebView2 集成
- 简单的壁纸显示功能
- Flutter MethodChannel 通信
