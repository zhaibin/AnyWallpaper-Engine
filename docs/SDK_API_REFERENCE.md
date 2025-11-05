# AnyWP SDK v4.0.0 API 参考

## 📖 快速开始

```javascript
// SDK 自动注入到 window.AnyWP
if (window.AnyWP) {
  AnyWP.ready('我的壁纸');
  AnyWP.onClick('#button', (x, y) => {
    AnyWP.openURL('https://example.com');
  });
}
```

---

## 🎯 核心 API

### `ready(name)`

通知壁纸已就绪。

**参数**：
- `name` (string) - 壁纸名称

**示例**：
```javascript
AnyWP.ready('我的壁纸');
```

---

### `onClick(element, callback, options)`

注册可点击区域。

**参数**：
- `element` (string|Element) - CSS 选择器或 DOM 元素
- `callback(x, y)` (Function) - 点击回调函数
- `options` (Object) - 可选配置

**Options**：
| 选项 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `waitFor` | boolean | true | 等待元素出现 |
| `maxWait` | number | 10000 | 最大等待时间（ms） |
| `immediate` | boolean | false | 立即注册（不延迟） |
| `autoRefresh` | boolean | true | 自动刷新边界 |
| `delay` | number | 100 | 延迟时间（ms） |
| `debug` | boolean | false | 显示调试边框 |

**示例**：
```javascript
// 基础用法
AnyWP.onClick('#button', (x, y) => {
  console.log('点击位置:', x, y);
});

// SPA 推荐（等待元素）
AnyWP.onClick('.dynamic-button', callback, { 
  waitFor: true 
});

// 调试模式
AnyWP.onClick('#button', callback, { 
  debug: true 
});
```

---

### `openURL(url)`

在默认浏览器中打开 URL。

**参数**：
- `url` (string) - URL 地址

**示例**：
```javascript
AnyWP.openURL('https://github.com');
```

---

### `refreshBounds()`

刷新所有已注册元素的点击边界。

**返回**：
- (number) 刷新的元素数量

**示例**：
```javascript
// SPA 路由切换后
const refreshed = AnyWP.refreshBounds();
console.log('刷新了', refreshed, '个元素');
```

---

### `clearHandlers()`

清除所有已注册的点击处理器。

**示例**：
```javascript
// 组件卸载时
AnyWP.clearHandlers();
```

---

### `enableDebug()`

启用调试模式（显示红色边框）。

**示例**：
```javascript
AnyWP.enableDebug();
```

---

### `setSPAMode(enabled)`

手动启用/禁用 SPA 模式。

**参数**：
- `enabled` (boolean) - 是否启用

**示例**：
```javascript
AnyWP.setSPAMode(true);
```

---

### `onMouse(callback)`

监听鼠标事件。

**参数**：
- `callback(event)` (Function) - 事件回调

**Event 对象**：
```javascript
{
  type: 'mousedown' | 'mouseup' | 'mousemove',
  x: number,      // 物理像素 X
  y: number,      // 物理像素 Y
  button: number  // 0=左键
}
```

**示例**：
```javascript
AnyWP.onMouse((event) => {
  console.log('鼠标事件:', event.type, event.x, event.y);
});
```

---

### `onKeyboard(callback)`

监听键盘事件。

**参数**：
- `callback(event)` (Function) - 事件回调

**示例**：
```javascript
AnyWP.onKeyboard((event) => {
  console.log('键盘事件:', event);
});
```

---

### `onVisibilityChange(callback)` 🆕

监听壁纸可见性变化（用于省电优化）。

**参数**：
- `callback(visible)` (Function) - 可见性回调
  - `visible` (boolean) - `true` 表示可见，`false` 表示隐藏

**何时触发**：
- 系统锁屏时 → `visible = false`
- 系统解锁时 → `visible = true`
- 全屏应用启动时 → `visible = false`
- 用户空闲超时时 → `visible = false`
- 手动暂停时 → `visible = false`

**自动行为**：
- SDK 会**自动暂停**所有 `<video>` 和 `<audio>` 元素
- SDK 会**自动恢复**之前播放的媒体
- 开发者可添加自定义暂停/恢复逻辑

**示例**：
```javascript
// 基础用法
AnyWP.onVisibilityChange(function(visible) {
  if (visible) {
    console.log('壁纸可见 - 恢复动画');
    resumeAnimations();
  } else {
    console.log('壁纸隐藏 - 暂停动画以省电');
    pauseAnimations();
  }
});

// 保存和恢复状态
let animationState = { frame: 0 };

AnyWP.onVisibilityChange(function(visible) {
  if (visible) {
    // 从保存的状态恢复
    resumeFromFrame(animationState.frame);
  } else {
    // 保存当前状态
    animationState.frame = getCurrentFrame();
    pauseAnimation();
  }
});

// Canvas 动画优化
let isVisible = true;

AnyWP.onVisibilityChange(function(visible) {
  isVisible = visible;
});

function animate() {
  if (isVisible) {
    // 只在可见时渲染
    ctx.clearRect(0, 0, width, height);
    drawFrame();
  }
  requestAnimationFrame(animate);
}
```

**💡 提示**：
- ✅ SDK 自动处理视频/音频，大多数情况下无需额外代码
- ✅ 对于自定义动画，使用此 API 优化性能
- ✅ 保存状态以实现流畅的暂停/恢复体验
- ⚡ 恢复速度 <50ms，用户几乎感觉不到暂停

---

## 🔄 SPA 支持

### 自动检测

SDK 自动检测以下框架：
- React
- Vue
- Angular

检测到后自动启用：
- ✅ 路由变化监听
- ✅ DOM 变化监听
- ✅ 自动刷新边界

### 手动配置

```javascript
// 强制启用 SPA 模式
AnyWP.setSPAMode(true);

// 注册时使用 waitFor
AnyWP.onClick('.button', callback, { waitFor: true });

// 路由切换后手动刷新
router.afterEach(() => {
  setTimeout(() => AnyWP.refreshBounds(), 500);
});
```

---

## 📐 坐标系统

### 物理像素 vs CSS 像素

SDK 内部使用物理像素，自动处理 DPI 缩放。

**属性**：
- `AnyWP.dpiScale` - DPI 缩放比例
- `AnyWP.screenWidth` - 物理宽度
- `AnyWP.screenHeight` - 物理高度

**示例**：
```javascript
console.log('DPI:', AnyWP.dpiScale);        // 2.0
console.log('CSS 宽度:', window.innerWidth); // 1920px
console.log('物理宽度:', AnyWP.screenWidth);  // 3840px
```

**开发者无需关心**：使用 CSS 定位即可，SDK 自动转换。

---

## 🐛 调试

### 启用调试模式

```javascript
// 方法 1：代码启用
AnyWP.enableDebug();

// 方法 2：URL 参数
// http://example.com?debug
```

### 查看注册状态

```javascript
console.log('已注册处理器:', AnyWP._clickHandlers.length);
console.log('SPA 模式:', AnyWP._spaMode);
console.log('DPI 缩放:', AnyWP.dpiScale);
```

### 手动测试点击

```javascript
// 模拟点击事件
AnyWP._handleClick(500, 300);
```

---

## ⚡ 性能优化

### 减少注册数量

```javascript
// ❌ 不推荐 - 为每个元素注册
items.forEach(item => {
  AnyWP.onClick(`#item-${item.id}`, callback);
});

// ✅ 推荐 - 注册父容器
AnyWP.onClick('#item-list', (x, y) => {
  // 根据坐标判断子元素
});
```

### 禁用不必要的自动刷新

```javascript
// 对于静态元素
AnyWP.onClick('.static-btn', callback, {
  autoRefresh: false
});
```

---

## 🌐 浏览器兼容性

**运行环境**: WebView2 (Chromium Edge)

**支持特性**：
- ✅ ES6+ 全部语法
- ✅ MutationObserver
- ✅ ResizeObserver
- ✅ 所有现代 Web API

---

## 📝 示例

完整示例请查看：
- `examples/test_simple.html` - 基础示例
- `examples/test_react.html` - React 集成
- `examples/test_vue.html` - Vue 集成
- `examples/test_basic_click.html` - 纯 HTML 示例

---

**版本**: 4.0.0  
**更新**: 2025-11-03

