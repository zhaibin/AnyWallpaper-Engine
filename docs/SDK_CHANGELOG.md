# AnyWP SDK 更新日志

## [4.0.0] - 2025-11-03 - React/Vue SPA 完整支持

### ✨ 新增功能

#### SPA 框架支持
- 自动检测 React、Vue、Angular 框架
- 智能路由监听（pushState/replaceState/popstate）
- DOM 变化监听（MutationObserver）
- 元素自动重新挂载和边界刷新
- ResizeObserver 集成

#### 新增 API
- `refreshBounds()` - 手动刷新所有点击边界
- `clearHandlers()` - 清除所有注册的处理器
- `setSPAMode(enabled)` - 手动启用/禁用 SPA 模式

#### onClick 增强选项
- `waitFor: true` - 等待元素出现（SPA 推荐）
- `maxWait: 10000` - 最大等待时间
- `immediate: false` - 立即注册
- `autoRefresh: true` - 自动刷新边界
- `delay: 100` - 延迟时间
- `debug: false` - 调试模式

### 🐛 修复
- 修复 SPA 路由切换后点击区域失效
- 修复动态内容加载后点击不准确
- 修复元素重新挂载后检测失败
- 改进调试边框更新逻辑

### ⚡ 性能优化
- 使用 ResizeObserver 替代定时轮询
- 优化 DOM 变化监听
- 减少不必要的边界计算

### 🔄 向后兼容
- 完全兼容 v3.x API
- 旧代码无需修改

---

## [3.1.0] - 2025-10-30

### 初始功能
- `ready(name)` - 通知就绪
- `onClick(element, callback, options)` - 注册点击
- `openURL(url)` - 打开 URL
- `onMouse(callback)` - 鼠标事件
- `onKeyboard(callback)` - 键盘事件
- `enableDebug()` - 调试模式

### 基础属性
- `version` - SDK 版本
- `dpiScale` - DPI 缩放
- `screenWidth` / `screenHeight` - 屏幕尺寸
- `interactionEnabled` - 交互状态

