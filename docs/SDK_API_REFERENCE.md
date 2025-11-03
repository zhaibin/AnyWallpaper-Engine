# AnyWP SDK v4.0.0 - API 参考文档

## 概述

AnyWP SDK 是自动注入到 WebView2 的 JavaScript 桥接库，提供与 AnyWP Engine 的交互能力。

**版本**: 4.0.0  
**兼容性**: React, Vue, Angular, 原生 HTML  
**自动注入**: 是  

---

## 全局对象

### `window.AnyWP`

SDK 主对象，在页面加载时自动创建。

#### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `version` | string | SDK 版本号（如 "4.0.0"） |
| `dpiScale` | number | DPI 缩放比例 |
| `screenWidth` | number | 屏幕物理宽度（像素） |
| `screenHeight` | number | 屏幕物理高度（像素） |
| `interactionEnabled` | boolean | 是否启用交互模式 |

---

## 核心方法

### `ready(name)`

通知 AnyWP Engine 壁纸已就绪。

**参数**:
- `name` (string): 壁纸名称

**示例**:
```javascript
AnyWP.ready('我的壁纸');
```

---

### `onClick(element, callback, options)`

注册点击区域。这是最常用的方法。

**参数**:
- `element` (string | HTMLElement): 元素选择器或 DOM 元素
- `callback(x, y)` (function): 点击回调函数
  - `x` (number): 物理像素 X 坐标
  - `y` (number): 物理像素 Y 坐标
- `options` (object, 可选): 配置选项

**Options**:
```javascript
{
  immediate: false,      // 立即注册（不延迟）
  waitFor: true,         // 等待元素出现（SPA 推荐）
  maxWait: 10000,        // 最大等待时间（毫秒）
  autoRefresh: true,     // 自动刷新边界
  delay: 100,            // 延迟注册时间（毫秒）
  debug: false           // 显示红色调试边框
}
```

**示例**:
```javascript
// 基础用法
AnyWP.onClick('#myButton', function(x, y) {
  console.log('按钮被点击：', x, y);
  AnyWP.openURL('https://example.com');
});

// SPA 等待模式
AnyWP.onClick('.dynamic-button', callback, { 
  waitFor: true,
  debug: true 
});

// React/Vue 集成
AnyWP.onClick('.increment-btn', () => {
  setCount(c => c + 1);  // React
}, { waitFor: true });
```

---

### `openURL(url)`

在默认浏览器中打开 URL。

**参数**:
- `url` (string): 要打开的 URL

**示例**:
```javascript
AnyWP.openURL('https://github.com');
```

---

### `refreshBounds()`

刷新所有已注册元素的点击边界。

**返回**: number - 刷新的元素数量

**使用场景**:
- SPA 路由切换后
- 动态内容更新后
- 窗口大小改变后

**示例**:
```javascript
// React Router
useEffect(() => {
  setTimeout(() => AnyWP.refreshBounds(), 500);
}, [location]);

// Vue Router
router.afterEach(() => {
  setTimeout(() => AnyWP.refreshBounds(), 500);
});
```

---

### `clearHandlers()`

清除所有已注册的点击处理器。

**使用场景**:
- 组件卸载时
- 重新初始化前

**示例**:
```javascript
// React cleanup
useEffect(() => {
  AnyWP.onClick('.btn', callback);
  return () => AnyWP.clearHandlers();
}, []);
```

---

### `enableDebug()`

启用调试模式。

**效果**:
- 显示红色调试边框
- 输出详细控制台日志
- 显示物理/CSS 坐标

**示例**:
```javascript
AnyWP.enableDebug();
```

**URL 参数**:
```
http://localhost:3000?debug
```

---

### `setSPAMode(enabled)`

手动启用/禁用 SPA 模式。

**参数**:
- `enabled` (boolean): true 启用，false 禁用

**示例**:
```javascript
AnyWP.setSPAMode(true);
```

**注意**: 通常无需手动设置，SDK 会自动检测。

---

### `onMouse(callback)`

监听鼠标事件。

**参数**:
- `callback(event)` (function): 鼠标事件回调
  - `event.type` (string): 事件类型（mousedown/mouseup/mousemove）
  - `event.x` (number): X 坐标
  - `event.y` (number): Y 坐标
  - `event.button` (number): 按钮（0=左键）

**示例**:
```javascript
AnyWP.onMouse(function(event) {
  console.log('鼠标事件:', event.type, event.x, event.y);
});
```

---

### `onKeyboard(callback)`

监听键盘事件。

**参数**:
- `callback(event)` (function): 键盘事件回调

**示例**:
```javascript
AnyWP.onKeyboard(function(event) {
  console.log('键盘事件:', event);
});
```

---

## SPA 框架集成

### React

```jsx
import { useEffect, useState } from 'react';

function MyComponent() {
  const [count, setCount] = useState(0);

  useEffect(() => {
    if (!window.AnyWP) return;

    AnyWP.ready('My App');
    
    AnyWP.onClick('.increment-btn', () => {
      setCount(c => c + 1);
    }, { waitFor: true });
    
  }, []); // 空依赖数组 - 只注册一次

  return <button className="increment-btn">+</button>;
}
```

### Vue 3

```vue
<script setup>
import { ref, onMounted } from 'vue';

const count = ref(0);

onMounted(() => {
  if (window.AnyWP) {
    AnyWP.ready('My App');
    
    AnyWP.onClick('.increment-btn', () => {
      count.value++;
    }, { waitFor: true });
  }
});
</script>
```

### Vue 2

```javascript
export default {
  mounted() {
    if (window.AnyWP) {
      AnyWP.ready('My App');
      AnyWP.onClick('.btn', this.handleClick, { waitFor: true });
    }
  }
}
```

---

## 事件系统

### 自定义事件

SDK 使用 CustomEvent 与 Native 通信。

#### `AnyWP:click`
点击事件（onClick 处理器监听此事件）

```javascript
window.addEventListener('AnyWP:click', function(event) {
  const { x, y } = event.detail;
  console.log('点击：', x, y);
});
```

#### `AnyWP:mouse`
鼠标事件（onMouse 处理器监听此事件）

```javascript
window.addEventListener('AnyWP:mouse', function(event) {
  const { type, x, y, button } = event.detail;
});
```

#### `AnyWP:keyboard`
键盘事件（onKeyboard 处理器监听此事件）

#### `AnyWP:interactionMode`
交互模式变化

```javascript
window.addEventListener('AnyWP:interactionMode', function(event) {
  const { enabled } = event.detail;
  console.log('交互模式：', enabled);
});
```

---

## 坐标系统

### 物理像素 vs CSS 像素

SDK 内部使用**物理像素**（考虑 DPI 缩放）。

```javascript
// 例如 DPI 缩放 2x
AnyWP.dpiScale      // 2
window.innerWidth   // 1920 (CSS 像素)
AnyWP.screenWidth   // 3840 (物理像素 = 1920 * 2)
```

**开发者无需关心**：SDK 自动处理转换。

---

## 最佳实践

### 1. 使用 waitFor 选项（SPA）

```javascript
// ✅ 推荐
AnyWP.onClick('.btn', callback, { waitFor: true });

// ❌ 不推荐
setTimeout(() => {
  AnyWP.onClick('.btn', callback);
}, 2000);
```

### 2. 使用函数式更新（React）

```javascript
// ✅ 正确
AnyWP.onClick('#btn', () => setCount(c => c + 1));

// ❌ 错误
AnyWP.onClick('#btn', () => setCount(count + 1));
```

### 3. 只在顶层组件初始化一次

```javascript
// App.jsx
useEffect(() => {
  if (window.AnyWP) {
    AnyWP.ready('My App');
    AnyWP.setSPAMode(true);
  }
}, []);
```

### 4. 路由切换后刷新边界

```javascript
// React Router
useEffect(() => {
  setTimeout(() => AnyWP.refreshBounds(), 500);
}, [location]);
```

---

## 调试

### 启用调试模式

```javascript
AnyWP.enableDebug();
```

或在 URL 中添加：
```
?debug
```

### 查看已注册处理器

```javascript
console.log('处理器数量:', AnyWP._clickHandlers.length);
AnyWP._clickHandlers.forEach((h, i) => {
  console.log(`Handler ${i}:`, h.element, h.bounds);
});
```

### 测试点击坐标

```javascript
// 手动触发测试
AnyWP._handleClick(500, 300);
```

---

## 版本历史

### v4.0.0 (2025-11-03)
- ✨ 完整的 React/Vue SPA 支持
- ✨ 智能路由和 DOM 监听
- ✨ ResizeObserver 集成
- ✨ `waitFor` / `autoRefresh` 选项
- ✨ `refreshBounds()` / `clearHandlers()` API

### v3.1.0
- 基础点击处理
- iframe 支持
- 鼠标/键盘事件

---

## 相关文档

- [Web 开发者集成指南（中文）](WEB_DEVELOPER_GUIDE_CN.md)
- [Web 开发者集成指南（英文）](WEB_DEVELOPER_GUIDE.md)
- [API 桥接](API_BRIDGE.md)
- [技术实现](TECHNICAL_NOTES.md)

---

**最后更新**: 2025-11-03  
**维护者**: AnyWP Engine Team

