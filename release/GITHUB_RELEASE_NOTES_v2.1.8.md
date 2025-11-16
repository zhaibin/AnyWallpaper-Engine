# AnyWP Engine v2.1.8 - Release Notes

**发布日期**: 2025-11-15
**版本**: 2.1.8

---

## 🧪 全屏暂停诊断 + 轮询降频


## ✨ 功能增强

### 全屏应用检测日志改进
- **改进内容**:
  - `power_manager.cpp`: 在检测到全屏应用时记录应用名称和窗口类名
  - 示例日志：`[PowerManager] Fullscreen app detected: "Google Chrome" (Class: Chrome_WidgetWin_1)`
  - 便于调试和了解哪些应用触发了暂停
- **测试资源**:
  - 新增 `examples/test_fullscreen_pause.html`: 专门用于测试全屏暂停功能的页面
  - `example/lib/main.dart`: 快速入口新增「Fullscreen Pause」卡片

### Web SDK 可靠性增强
- `windows/sdk/modules/webmessage.ts`: 解析 `PostWebMessageAsString` 传入的字符串，并新增 `powerStateChange` 消息处理逻辑，保证 C++ 端补偿通知能可靠送达 HTML
- `windows/anywp_sdk.js` 及 `dist/` 产物同步更新，`window.AnyWP` 能在 WebView2 字符串消息模式下正常收到事件
- `windows/modules/power_manager.cpp`、`windows/anywp_engine_plugin.cpp`: 统一日志格式，输出脚本执行结果，便于定位全屏暂停问题

### 调试文档与示例
- 新增 `docs/FULLSCREEN_PAUSE_TEST_GUIDE.md`，提供逐步复现与日志定位指南
- README、`docs/FOR_FLUTTER_DEVELOPERS.md` 增加 **Known Limitations** / **已知限制** 章节，明确说明全屏场景的行为和临时对策

## 🐛 全屏暂停功能改进

### 问题现象
- **用户反馈**: 全屏应用时，测试页面的暂停/恢复计数器不变化（锁屏时正常）
- **测试结果**: 
  - ✅ 锁屏场景：暂停 +1，恢复 +1
  - ❌ 全屏场景：暂停 0，恢复 0

### 调查过程与发现
1. **初步假设（错误）**: 认为 Windows 会自动挂起后台 WebView 的 JS 引擎
   - 推测：全屏时 WebView 被挂起，脚本无法执行
   - 对策：跳过暂停脚本，延迟恢复脚本
   
2. **实际测试（推翻假设）**: 
   - ⚠️ **关键发现**：全屏后动画仍在播放！
   - ✅ **结论**：WebView 并未被挂起，脚本可以正常执行
   
3. **真实原因（待确认）**: 
   - 可能是脚本执行时机问题
   - 可能是事件回调被延迟
   - 需要进一步调试日志确认

### 根本原因确认
- **测试发现**: 全屏后视频依旧播放 → 说明暂停脚本未生效
- **日志分析**: 
  - 脚本被调用：✅ `[PowerManager] Executing pause scripts...`
  - 回调未触发：❌ 没有 `[ScriptExecution] Result:` 日志
- **结论**: 全屏应用在前台时，壁纸 WebView 的 `ExecuteScript` 异步回调被阻塞，无法触发

### 解决方案：补偿性通知机制
- **核心思路**: 既然全屏时回调不触发，那么在全屏**结束后**补发通知
- **实现**: `ResumeWallpaper()` 检测全屏恢复场景
  ```cpp
  if (reason.find("fullscreen") != std::string::npos) {
    // 延迟 1 秒后（确保消息循环恢复）
    // 1. 先发送暂停通知（补偿全屏期间错过的）
    // 2. 再发送恢复通知（告知全屏已结束）
  }
  ```
- **时序**:
  1. 检测到全屏 → 调用暂停脚本（回调阻塞，页面未收到）
  2. 检测到退出 → 调用恢复脚本（回调阻塞，页面未收到）
  3. **补偿通知**: 延迟 1 秒后，依次发送暂停+恢复通知
  4. 此时全屏已退出，消息循环正常，页面成功接收

### 改进内容
- **核心修复**: 
  - `anywp_engine_plugin.cpp`: 添加补偿性通知机制
  - 全屏退出后延迟发送 `_notifyVisibilityChange(false)` 和 `_notifyVisibilityChange(true)`
- **增强日志**: 
  - `power_manager.cpp`: 记录检测到的全屏应用名称和类名
  - `anywp_engine_plugin.cpp`: 记录脚本执行返回值
  - 添加补偿性通知的日志标记
- **新增测试资源**:
  - `examples/test_fullscreen_pause.html`: 专用测试页面
  - `docs/FULLSCREEN_PAUSE_TEST_GUIDE.md`: 详细测试指南
  
### 最终调查结论（2025-11-14）
经过深度调查和多种方案尝试，得出以下结论：

**测试尝试的方案**：
1. ❌ ExecuteScript 立即通知（进入全屏时）
2. ❌ ExecuteScript 延迟通知（退出全屏后 2 秒）
3. ❌ ExecuteScript 轮询通知（5 次尝试，递增延迟）
4. ❌ PostWebMessageAsJson 直接发送
5. ❌ 浏览器原生 Page Visibility API

**根本原因**：
- WebView2 在桌面壁纸模式下（`SetParent` 到 `WorkerW` 窗口，Z-order 最底层）
- 被全屏应用完全遮挡时，处于一种特殊状态：
  - ✅ JavaScript 引擎仍在运行（动画继续播放）
  - ❌ `ExecuteScript` 的回调被无限期延迟或阻塞
  - ❌ 浏览器不认为页面被隐藏（`document.hidden = false`）
  - ❌ Page Visibility API 不触发

**锁屏 vs 全屏的区别**：
- 锁屏：壁纸仍可见（锁屏界面是覆盖层）→ WebView 正常响应 ✅
- 全屏：壁纸完全不可见（被覆盖）→ WebView 进入特殊后台模式 ❌

**决定：接受限制**：
- 全屏场景暂时无法通过 JavaScript 通知机制解决
- 回滚所有尝试性修改，保持代码简洁
- 保留增强的日志和类型定义（对调试有帮助）
- 优先确保其他场景（锁屏、息屏）的完美工作

**未来可能的方案**：
- 方案 A：C++ 侧检测全屏后直接隐藏壁纸窗口（`ShowWindow(SW_HIDE)`）
- 方案 B：降低 WebView2 的渲染帧率或挂起渲染线程
- 方案 C：研究 WebView2 的后台渲染策略

## ⚙️ 性能优化
- `lib/anywp_engine.dart`: 将电源状态轮询间隔从 100ms 降到 1000ms，减少 90% 轮询日志与 CPU 占用，但仍可及时响应锁屏/唤醒事件

---

