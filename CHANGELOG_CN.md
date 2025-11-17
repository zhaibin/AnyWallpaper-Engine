# 更新日志

所有重要的项目变更都将记录在此文件中。

## [2.2.0] - 2025-11-17 (Multi-Platform Release 🎉)

### 🌍 新增功能 - macOS 平台支持
- ✅ **macOS 原生插件**: 基于 Objective-C + AppKit + WKWebView
- ✅ **统一 API**: Windows 和 macOS 使用相同的 Dart API
- ✅ **模块化架构**: MonitorManager, WallpaperManager, PowerManager, MessageBridge
- ✅ **JavaScript SDK**: 平台无关的 SDK，自动适配 WKWebView 消息传递
- ✅ **多显示器支持**: NSScreen API 实现
- ✅ **电源管理**: NSWorkspace 通知集成（屏幕休眠、会话锁定、空闲检测）
- ✅ **状态持久化**: 使用 Application Support 目录存储
- ✅ **内存优化**: 针对 WKWebView 调整阈值（200MB 默认）

### 📖 文档更新
- ✅ **多平台架构设计**: `docs/MULTIPLATFORM_ARCHITECTURE.md`
- ✅ **macOS 开发者指南**: `docs/MACOS_DEVELOPER_GUIDE.md`
- ✅ **更新 README**: 添加平台支持说明

### 🔧 架构改进
- ✅ **平台抽象**: 清晰的平台特定和共享组件分离
- ✅ **代码组织**: `macos/` 和 `windows/` 平台特定实现
- ✅ **构建系统**: CMake + CocoaPods 支持

### 🎯 技术亮点
- **Windows**: WebView2 (Chromium) + Win32 API + C++17
- **macOS**: WKWebView (WebKit) + AppKit + Objective-C
- **共享**: Dart API 层 + TypeScript SDK + 设计模式

### 📋 已知限制
- macOS 交互模式（Interactive Mode）暂未实现，需要 Accessibility 权限
- WKWebView 文件访问受沙箱限制
- WKWebView 内存使用通常高于 WebView2 (150-200MB vs 100-150MB)

---

## [2.1.10] - 2025-11-16

### 新增功能 (Added)

#### 🔢 版本号 API
- 新增 `AnyWPEngine.getSDKVersion()` API，用于获取内置 Web SDK 版本号
- 优化 `AnyWPEngine.getPluginVersion()` 文档说明
- 支持在 Dart 和 JavaScript 中查询版本信息

#### 🔐 自定义协议与文件加密
- 新增自定义 URL 协议 `anywp://file?path=<path>` 支持
- 新增 `AnyWPEngine.encryptFile(sourcePath, destPath)` API（Dart + JavaScript）
- 新增 `AnyWPEngine.decryptFile(encryptedPath, destPath)` API（Dart + JavaScript）
- 实现零拷贝内容交付：协议自动解密文件
- 新增 MIME 类型自动检测（支持图片/视频格式）
- 开发者可自定义缓存路径，引擎不管理固定目录

#### 📖 文档完善
- 新增 `docs/VERSION_MANAGEMENT.md` - 版本号统一管理指南
- 更新 `docs/DEVELOPER_API_REFERENCE.md` - 添加版本号和加密 API 文档
- 更新 `docs/WEB_DEVELOPER_GUIDE_CN.md` - 添加自定义协议完整指南
- 更新 `docs/FOR_FLUTTER_DEVELOPERS.md` - 更新 API 列表

### 修复 (Fixed)

#### 🖱️ 交互式壁纸点击支持
- 修复 Windows 11 上某些应用使用多层窗口结构导致的点击冲突问题
- 实现更智能的点击检测：优先检测目标应用窗口区域，仅在空白区域转发事件
- 修复特定应用（如 WinRAR、资源管理器）的点击不响应问题
- 新增窗口层次遍历，确保点击正确转发到目标应用

#### 🔧 内部优化
- 增强 CustomSchemeHandler 错误日志输出
- 添加 MimeTypeDetector 用于正确的 MIME 类型检测
- 改进自定义协议资源加载性能

### 依赖更新 (Dependencies)
- 无外部依赖变更

---

## [2.1.9] - 2025-11-15

### 新增功能 (Added)

#### 🖱️ 自适应交互式点击系统（v2.1.9 核心功能）
**架构设计**:
- 实现 3 级智能检测机制：目标应用识别 → 窗口区域验证 → iframe 广告检测
- 新增 `ConflictDetector` 模块（`windows/utils/conflict_detector.cpp/h`）
- 新增 `EventDispatcher` 模块（`windows/modules/event_dispatcher.cpp/h`，v2.1.0 合并至 `anywp_engine_plugin.cpp`）

**智能检测能力**:
1. **目标应用识别**
   - 实时识别点击目标窗口的应用程序
   - 检测 Chrome/Edge 浏览器（标题特征）
   - 检测文件管理器（窗口类名）
   - 检测其他常见应用（标题特征）

2. **窗口区域验证**
   - 精确检测点击位置是否在目标窗口边界内
   - 使用 `GetWindowRect` 确保点击在有效区域
   - 避免误触发桌面或其他应用区域

3. **iframe 广告检测**
   - 解析 iframe 位置和尺寸数据
   - 识别广告链接（常见广告域名白名单）
   - 阻止广告区域点击转发
   - 允许合法链接的正常点击

**冲突解决策略**:
- Chrome/Edge 浏览器 → 仅在窗口内转发事件（避免误触桌面链接）
- 文件管理器 → 精确匹配窗口区域
- iframe 广告区域 → 阻止转发
- 其他应用 → 基本窗口边界检测

#### 🛠️ 技术改进
- 新增 `SafetyMacros.h` 用于统一异常处理
- 优化 MouseHook 性能（减少不必要的日志）
- 新增详细的冲突检测日志（Verbose 模式）

### 修复 (Fixed)

#### 🖱️ 浏览器点击冲突修复
**问题场景**:
- 在 Chrome/Edge 中打开任意网页（如 Google 搜索）
- 点击链接时，浏览器正常导航
- 但桌面上相同位置的链接也被意外点击（如果存在）
- 导致两个页面同时打开（浏览器 + 桌面壁纸）

**根本原因**:
- `LowLevelMouseProc` 钩子拦截所有鼠标事件
- 未正确过滤目标窗口
- 同时转发事件到浏览器和 WebView2 壁纸

**解决方案**:
- 实现 ConflictDetector：智能识别目标应用和窗口区域
- 如果目标是 Chrome/Edge：阻止事件转发到壁纸
- 仅当点击位置不在任何应用窗口内时，才转发到壁纸

**测试验证**:
- ✅ Chrome 中点击链接：仅浏览器响应，壁纸不响应
- ✅ 桌面空白处点击：壁纸正常响应
- ✅ 文件管理器操作：仅管理器响应，壁纸不响应

---

## [2.1.8] - 2025-02-26 凌晨

### 修复 (Fixed)

#### 🖱️ 完善点击支持（基于 v2.1.7）
**症状**:
- JavaScript `window.onclick` 事件可以捕获桌面点击
- 但点击可点击元素（`<a>`, `<button>` 等）时不触发其默认行为
- 例如：点击 `<a href="...">` 链接时，`onclick` 触发但不跳转

**根本原因（分析过程）**:
1. **初步假设**：鼠标钩子只发送 `mouseup` 事件
   - 实际：`SendClickToWebView` 会同时发送 `mousedown` + `mouseup`
   - 排除此假设
   
2. **第二假设**：浏览器未识别点击手势
   - 实际：手动添加了 `mousedown` + `mouseup` 事件发送
   - 但问题依旧
   
3. **第三假设**：JavaScript 事件冒泡被阻止
   - 实际：`window.onclick` 可以触发，说明事件冒泡正常
   - 但元素默认行为未触发
   
4. **真正原因**（最终确认）：
   - WebView2 要求完整的点击序列：`mousedown` → `mouseup` → `click`
   - 只发送 `mousedown` + `mouseup` 不会触发 `click` 事件
   - 而 `<a>` 等元素的默认行为依赖 `click` 事件

**解决方案**:
- 修改 `SendClickToWebView` 方法，使用浏览器 API 构造完整的点击序列
- 发送顺序：`mousedown` → `mouseup` → `click`
- 使用 JavaScript `dispatchEvent` 模拟真实用户点击

**实施细节**:
- 更新 `anywp_engine_plugin.cpp` 中的 `SendClickToWebView` 方法
- 使用 `ExecuteScript` 在 WebView2 中执行 JavaScript 代码
- 代码内容：创建并分发 `MouseEvent` 事件（类型：`mousedown`, `mouseup`, `click`）

**测试结果**:
- ✅ 点击链接 (`<a href="...">`) → 正常跳转
- ✅ 点击按钮 (`<button>`) → 触发 `onclick`
- ✅ 点击输入框 (`<input>`) → 获得焦点
- ✅ `window.onclick` → 仍然正常触发

---

## [2.1.7] - 2025-02-25 晚

### 修复 (Fixed)

#### 🖱️ 修复全屏应用暂停机制与点击转发（临时移除 sendInput）
**已知问题（来自 v2.1.5）**:
- 全屏应用覆盖壁纸时，C++ 层检测到全屏状态
- 但无法通知 JavaScript（`ExecuteScript` 回调被阻止）
- 导致壁纸继续在后台运行消耗资源

**v2.1.7 新问题**:
- 实现了鼠标点击转发到 WebView2
- 但在某些场景下（如打开全屏应用），点击事件被意外转发
- 导致资源占用增加、可能出现异常行为

**解决方案**:
1. **移除 SendInput 实现**（临时措施）
   - 保留 `SendClickToWebView` 方法框架
   - 暂时不使用 `SendInput` API 发送鼠标事件
   - 避免在全屏场景下的异常行为

2. **保留 JavaScript 事件监听**
   - `window.addEventListener('click')` 仍然存在
   - 可以捕获直接在 WebView2 窗口上的点击
   - 但不会接收桌面其他位置的点击

**影响**:
- ❌ 无法实现"点击桌面任意位置触发壁纸事件"功能
- ✅ 避免了全屏应用场景下的资源浪费
- ✅ 保持了基本的交互能力（直接点击 WebView2 窗口）

**后续计划**:
- 需要重新设计点击转发机制
- 可能方案：
  - 仅在 PowerState 为 ACTIVE 时转发点击
  - 添加"交互模式"开关，由用户控制是否启用
  - 使用更安全的事件注入方式

---

## [2.1.6] - 2025-02-25 下午

### 新增功能 (Added)

#### 🖱️ 点击事件支持（试验性）
- 实现低级鼠标钩子 (`SetWindowsHookEx` + `WH_MOUSE_LL`)
- 捕获桌面任意位置的点击事件
- 使用 `SendInput` API 将点击事件转发到 WebView2
- JavaScript 可以通过 `window.addEventListener('click')` 监听

**使用方式**:
```javascript
window.addEventListener('click', function(e) {
  console.log('Clicked at', e.clientX, e.clientY);
});
```

**技术实现**:
- `LowLevelMouseProc` 钩子回调函数
- `SendClickToWebView` 方法转发事件
- `INPUT` 结构体模拟鼠标输入

**注意事项**:
- 仅在 `enable_mouse_transparent = false` 时启用
- 可能影响性能（全局钩子）
- 可能与其他应用冲突（已在 v2.1.9 修复）

---

## [2.1.5] - 2025-02-24

### 已知问题 (Known Issues)

#### ⚠️ 全屏应用暂停机制限制
**场景**: 当全屏应用（游戏、视频播放器、全屏浏览器）覆盖壁纸时

**现象**:
- ✅ C++ 层正确检测到全屏状态（`PowerManager::UpdatePowerState`）
- ✅ 壁纸动画继续在后台运行（虽然不可见，但消耗资源）
- ❌ **无法通知 JavaScript 暂停/恢复事件**

**根本原因**:
- WebView2 的 `ExecuteScript` 回调被阻止（当窗口完全被遮挡时）
- 这是 WebView2 的设计限制，非 Bug
- 回调队列被延迟，直到窗口重新可见

**对比**: 锁屏场景
- 锁屏是一个**叠加层**，壁纸窗口仍然部分可见
- WebView2 可以正常执行脚本
- ✅ 暂停/恢复机制完美工作

**影响**:
- 全屏应用运行时，壁纸继续消耗 CPU/GPU（虽然不可见）
- 用户无法通过 JavaScript 监听到 `visibilitychange` 事件
- 只能通过手动调用 `pauseWallpaper()` 或关闭应用来节省资源

**临时解决方案**:
- 用户可以手动调用 `AnyWPEngine.pauseWallpaper()`
- 或者关闭应用以完全释放资源

**长期计划**:
- 研究 WebView2 的窗口可见性检测机制
- 可能需要使用其他方式（如 Native 消息队列）通知 JavaScript

---

## [2.1.4] - 2025-02-23

### 修复 (Fixed)

#### 🔄 双向通信可靠性提升
- 修复消息队列竞争条件
- 添加消息去重机制
- 优化轮询间隔（1秒）

---

## [2.1.3] - 2025-02-22

### 新增功能 (Added)

#### 📡 增强双向通信
- JavaScript 发送消息到 Flutter: `window.AnyWP.sendMessage(data)`
- Flutter 发送消息到 JavaScript: `AnyWPEngine.sendMessage(message: {...})`
- 支持标准消息格式（id, type, timestamp, data）

---

## [2.1.0 - 2.1.2] - 2025-02-20 至 2025-02-21

### 新增功能 (Added)
- 电源管理 API
- 内存优化 API
- 配置 API（空闲超时、内存阈值、清理间隔）

### 架构重构
- Phase2: 模块化重构（FlutterBridge, DisplayChangeCoordinator, InstanceManager, WindowManager）

---

## [2.0.0] - 2025-02-15

### 重大更新 (Major)

#### 🏗️ 模块化架构重构
- 拆分 13 个核心模块（`windows/modules/`）
- 拆分 17 个工具类（`windows/utils/`）
- 新增 3 个接口抽象（`windows/interfaces/`）
- ServiceLocator 依赖注入模式

#### 🧪 测试覆盖率
- 209+ 单元测试
- 98.5% 代码覆盖率
- 轻量级测试框架

---

## [1.0.0 - 1.3.5] - 2025-01 至 2025-02-10

### 核心功能
- 基础壁纸功能实现
- 多显示器支持
- 热插拔支持
- JavaScript SDK
- 状态持久化
- 应用级存储隔离

---

**格式说明**:
- 🌍 新增功能
- 🔧 修复
- 📖 文档
- 🏗️ 架构
- ⚠️ 已知问题
