# AnyWP Engine - Web 开发者集成指南

## 📖 概述

本指南面向 Web 开发者，帮助你将网页集成到 AnyWP Engine 作为交互式桌面壁纸。

**支持的技术栈**：
- ✅ 原生 HTML/CSS/JavaScript
- ✅ React (包括 Create React App, Next.js 等)
- ✅ Vue 2/3 (包括 Nuxt.js 等)
- ✅ Angular
- ✅ 其他任何 Web 框架

---

## 🚀 快速开始

### 1. 基础集成（静态网页）

```html
<!DOCTYPE html>
<html>
<head>
  <title>我的壁纸</title>
</head>
<body>
  <button id="myButton">点击我</button>

  <script>
    // SDK 会自动注入，直接使用
    if (window.AnyWP) {
      // 通知壁纸已就绪
      AnyWP.ready('我的壁纸');

      // 注册点击区域
      AnyWP.onClick('#myButton', function(x, y) {
        console.log('按钮被点击了！');
        AnyWP.openURL('https://example.com');
      });
    }
  </script>
</body>
</html>
```

---

## 🎯 核心 API

### AnyWP 对象

SDK 会自动注入 `window.AnyWP` 全局对象。

#### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `version` | string | SDK 版本号 |
| `dpiScale` | number | DPI 缩放比例 |
| `screenWidth` | number | 屏幕物理宽度（像素） |
| `screenHeight` | number | 屏幕物理高度（像素） |
| `interactionEnabled` | boolean | 是否启用交互模式 |

#### 方法

##### `ready(name)`
通知 AnyWP Engine 壁纸已就绪。

```javascript
AnyWP.ready('我的超酷壁纸');
```

##### `onClick(element, callback, options)`
注册点击区域。

**参数**：
- `element`: 元素选择器（字符串）或 DOM 元素对象
- `callback(x, y)`: 点击回调函数
- `options`: 配置选项（可选）

**Options 选项**：
```javascript
{
  immediate: false,      // 立即注册（不等待）
  waitFor: true,         // 等待元素出现（SPA 推荐）
  maxWait: 10000,        // 最大等待时间（毫秒）
  autoRefresh: true,     // 自动刷新边界
  delay: 100,            // 延迟注册时间（毫秒）
  debug: false           // 显示调试边框
}
```

**示例**：
```javascript
// 基础用法
AnyWP.onClick('#button', function(x, y) {
  console.log('点击位置:', x, y);
});

// 等待元素出现（SPA 推荐）
AnyWP.onClick('.dynamic-button', callback, { 
  waitFor: true,
  maxWait: 5000 
});

// 立即注册 + 调试模式
AnyWP.onClick(document.querySelector('#btn'), callback, { 
  immediate: true,
  debug: true 
});
```

##### `openURL(url)`
在默认浏览器中打开 URL。

```javascript
AnyWP.openURL('https://github.com');
```

##### `refreshBounds()`
刷新所有已注册元素的点击边界。

```javascript
// SPA 路由切换后调用
AnyWP.refreshBounds();
```

##### `clearHandlers()`
清除所有已注册的点击处理器。

```javascript
AnyWP.clearHandlers();
```

##### `enableDebug()`
启用调试模式（显示点击区域红框）。

```javascript
AnyWP.enableDebug();
```

##### `setSPAMode(enabled)`
手动启用/禁用 SPA 模式。

```javascript
AnyWP.setSPAMode(true);  // 启用
```

##### `onMouse(callback)`
监听鼠标事件。

```javascript
AnyWP.onMouse(function(event) {
  console.log('鼠标:', event);
});
```

##### `onKeyboard(callback)`
监听键盘事件。

```javascript
AnyWP.onKeyboard(function(event) {
  console.log('键盘:', event);
});
```

---

## ⚛️ React 集成

### 使用 useEffect Hook

```jsx
import React, { useEffect, useState } from 'react';

function MyWallpaper() {
  const [count, setCount] = useState(0);

  useEffect(() => {
    if (!window.AnyWP) return;

    // 初始化
    AnyWP.enableDebug();
    AnyWP.ready('React 壁纸');

    // 注册点击区域（使用 waitFor）
    AnyWP.onClick('.increment-btn', () => {
      setCount(c => c + 1);
    }, { waitFor: true });

    AnyWP.onClick('.open-link', () => {
      AnyWP.openURL('https://react.dev');
    }, { waitFor: true });

    // 清理函数（组件卸载时）
    return () => {
      console.log('Component unmounting');
    };
  }, []); // 空依赖数组 - 只注册一次

  return (
    <div>
      <h1>计数器: {count}</h1>
      <button className="increment-btn">增加</button>
      <button className="open-link">打开 React 文档</button>
    </div>
  );
}
```

### React Router 路由切换

```jsx
import { useEffect } from 'react';
import { useLocation } from 'react-router-dom';

function App() {
  const location = useLocation();

  useEffect(() => {
    // 路由变化时刷新边界
    if (window.AnyWP) {
      setTimeout(() => {
        AnyWP.refreshBounds();
      }, 500);
    }
  }, [location]);

  return <Routes>...</Routes>;
}
```

### 最佳实践

1. **在顶层组件初始化一次**
   ```jsx
   // App.jsx
   useEffect(() => {
     if (window.AnyWP) {
       AnyWP.ready('My App');
       AnyWP.setSPAMode(true);
     }
   }, []);
   ```

2. **使用 `waitFor` 选项**
   ```jsx
   AnyWP.onClick('.my-button', callback, { waitFor: true });
   ```

3. **避免在依赖数组中包含状态**
   ```jsx
   // ❌ 错误 - 每次 count 变化都会重新注册
   useEffect(() => {
     AnyWP.onClick('#btn', () => setCount(count + 1));
   }, [count]);

   // ✅ 正确 - 只注册一次，使用函数式更新
   useEffect(() => {
     AnyWP.onClick('#btn', () => setCount(c => c + 1), { waitFor: true });
   }, []);
   ```

---

## 💚 Vue 集成

### Vue 3 Composition API

```vue
<template>
  <div>
    <h1>计数器: {{ count }}</h1>
    <button class="increment-btn">增加</button>
    <button class="open-link">打开 Vue 文档</button>
  </div>
</template>

<script>
import { ref, onMounted, onUnmounted } from 'vue';

export default {
  setup() {
    const count = ref(0);

    onMounted(() => {
      if (!window.AnyWP) return;

      // 初始化
      AnyWP.enableDebug();
      AnyWP.ready('Vue 壁纸');

      // 注册点击区域
      AnyWP.onClick('.increment-btn', () => {
        count.value++;
      }, { waitFor: true });

      AnyWP.onClick('.open-link', () => {
        AnyWP.openURL('https://vuejs.org');
      }, { waitFor: true });
    });

    onUnmounted(() => {
      console.log('Component unmounting');
    });

    return { count };
  }
};
</script>
```

### Vue 2 Options API

```vue
<script>
export default {
  data() {
    return {
      count: 0
    };
  },
  
  mounted() {
    if (!window.AnyWP) return;

    AnyWP.ready('Vue 2 壁纸');
    
    AnyWP.onClick('.increment-btn', () => {
      this.count++;
    }, { waitFor: true });
  }
};
</script>
```

### Vue Router 路由切换

```javascript
// router/index.js
import { createRouter } from 'vue-router';

const router = createRouter({...});

router.afterEach(() => {
  // 路由切换后刷新边界
  if (window.AnyWP) {
    setTimeout(() => {
      AnyWP.refreshBounds();
    }, 500);
  }
});

export default router;
```

### 最佳实践

1. **在根组件初始化**
   ```vue
   // App.vue
   onMounted(() => {
     if (window.AnyWP) {
       AnyWP.ready('My App');
       AnyWP.setSPAMode(true);
     }
   });
   ```

2. **使用 `waitFor` 选项**
   ```javascript
   AnyWP.onClick('.my-button', callback, { waitFor: true });
   ```

3. **避免重复注册**
   ```javascript
   // ✅ 正确 - 只在 onMounted 中注册
   onMounted(() => {
     AnyWP.onClick('.btn', () => count.value++, { waitFor: true });
   });
   ```

---

## 🔄 SPA 单页应用支持

### 自动检测

SDK 会自动检测以下框架：
- React
- Vue
- Angular

检测到 SPA 后会自动启用以下功能：
- ✅ 路由变化监听（pushState/replaceState）
- ✅ DOM 变化监听（MutationObserver）
- ✅ 自动刷新点击边界
- ✅ 元素重新挂载检测

### 手动配置

如果自动检测失败，可手动启用：

```javascript
// 在应用初始化时
AnyWP.setSPAMode(true);
```

### 路由切换处理

#### 方式 1：自动处理（推荐）

SDK 会自动监听路由变化并刷新边界。只需使用 `waitFor` 选项：

```javascript
AnyWP.onClick('.my-button', callback, { waitFor: true });
```

#### 方式 2：手动刷新

在路由切换后手动调用：

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

### 动态内容处理

#### 自动模式

使用 `autoRefresh` 选项（默认启用）：

```javascript
AnyWP.onClick('.dynamic-element', callback, {
  waitFor: true,
  autoRefresh: true  // 自动监听元素大小/位置变化
});
```

#### 手动模式

内容更新后手动刷新：

```javascript
// 添加/删除元素后
addNewElement();
setTimeout(() => AnyWP.refreshBounds(), 100);
```

---

## 🐛 调试技巧

### 1. 启用调试模式

```javascript
// 方法 1：代码中启用
AnyWP.enableDebug();

// 方法 2：URL 参数
// http://localhost:3000?debug
```

调试模式会：
- 显示红色边框标记点击区域
- 输出详细日志到控制台
- 显示物理坐标和 CSS 坐标

### 2. 检查 SDK 是否加载

```javascript
if (window.AnyWP) {
  console.log('SDK 版本:', AnyWP.version);
  console.log('SPA 模式:', AnyWP._spaMode);
} else {
  console.error('AnyWP SDK 未加载！');
}
```

### 3. 验证点击区域

```javascript
// 注册时显示调试边框
AnyWP.onClick('#button', callback, { debug: true });
```

### 4. 查看已注册的处理器

```javascript
console.log('已注册处理器数量:', AnyWP._clickHandlers.length);
AnyWP._clickHandlers.forEach((h, i) => {
  console.log(`Handler ${i}:`, h.element, h.bounds);
});
```

### 5. 测试点击坐标

```javascript
// 手动触发点击测试
AnyWP._handleClick(500, 300);
```

---

## 📐 坐标系统说明

### 物理像素 vs CSS 像素

SDK 内部使用**物理像素**（考虑 DPI 缩放），但开发者无需关心，SDK 会自动转换。

```javascript
console.log('DPI 缩放:', AnyWP.dpiScale);        // 例如 1.5x
console.log('CSS 宽度:', window.innerWidth);     // 1920px
console.log('物理宽度:', AnyWP.screenWidth);      // 2880px (1920 * 1.5)
```

### 注意事项

1. **使用 CSS 定位和尺寸即可**，SDK 会自动处理缩放
2. **避免使用固定坐标**，始终通过选择器注册元素
3. **响应式设计**会自动工作（配合 `autoRefresh`）

---

## ⚡ 性能优化

### 1. 减少注册数量

```javascript
// ❌ 不好 - 为每个列表项注册
items.forEach(item => {
  AnyWP.onClick(`#item-${item.id}`, callback);
});

// ✅ 更好 - 注册父容器，通过事件传播处理
AnyWP.onClick('#item-list', (x, y) => {
  // 通过坐标判断点击了哪个子元素
});
```

### 2. 禁用不必要的自动刷新

```javascript
// 对于静态元素，禁用 autoRefresh
AnyWP.onClick('.static-button', callback, {
  autoRefresh: false
});
```

### 3. 延迟初始化

```javascript
// 等待关键内容渲染后再注册
setTimeout(() => {
  AnyWP.onClick('.late-element', callback);
}, 2000);
```

### 4. 及时清理

```javascript
// 组件卸载时清理
onUnmounted(() => {
  AnyWP.clearHandlers();
});
```

---

## 🌐 浏览器兼容性

| 功能 | 要求 |
|------|------|
| 基础功能 | WebView2 (Chromium Edge) |
| MutationObserver | ✅ 全支持 |
| ResizeObserver | ✅ 全支持 |
| ES6+ 语法 | ✅ 全支持 |

**注意**：SDK 在 WebView2 环境中运行，基于最新 Chromium 内核，支持所有现代 Web 特性。

---

## 📝 完整示例

查看 `examples/` 目录下的示例文件：

- `test_simple.html` - 基础 HTML 示例
- `test_react.html` - React 集成示例
- `test_vue.html` - Vue 集成示例
- `test_api.html` - 完整 API 演示

---

## ❓ 常见问题

### Q: 为什么点击没有反应？

1. 检查元素是否存在：`document.querySelector('#myButton')`
2. 启用调试模式查看边界：`AnyWP.onClick('#myButton', callback, { debug: true })`
3. 确认注册时机：SPA 使用 `{ waitFor: true }`

### Q: SPA 路由切换后点击失效？

使用 `waitFor` 选项或在路由切换后调用 `AnyWP.refreshBounds()`。

### Q: 如何在 iframe 中使用？

暂不支持 iframe 内的点击检测，请在顶层页面注册。

### Q: 能否检测拖拽、滚动等事件？

当前版本仅支持点击事件，后续版本将支持更多交互。

### Q: 开发时如何测试？

1. 在浏览器中预览（功能受限，主要测试 UI）
2. 使用 AnyWP Engine 示例应用加载网页
3. 启用调试模式验证点击区域

---

## 🔗 相关资源

- [快速开始](QUICK_START.md)
- [技术实现](TECHNICAL_NOTES.md)
- [API 桥接](API_BRIDGE.md)
- [Flutter 插件使用指南](PACKAGE_USAGE_GUIDE_CN.md)

---

**版本**: 4.0.0  
**更新日期**: 2025-11-03

