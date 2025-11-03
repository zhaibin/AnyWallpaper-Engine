# anywp_sdk.js - AnyWP Engine JavaScript SDK

## 文件信息

**文件名**: `anywp_sdk.js`  
**版本**: 4.0.0  
**大小**: ~18KB  
**用途**: WebView2 自动注入的 JavaScript 桥接库  

---

## 自动注入机制

此 SDK 会通过以下方式自动注入到 WebView2：

1. **预注入**: `AddScriptToExecuteOnDocumentCreated()`
2. **导航完成注入**: `NavigationCompleted` 回调中手动执行
3. **HTML 引用**: 网页可直接引用（推荐用于 React/Vue）

```html
<!-- 推荐：在 React/Vue 应用中显式引用 -->
<script src="../windows/anywp_sdk.js"></script>
```

---

## SPA 支持

SDK 自动检测并支持以下框架：

### 自动检测
- ✅ React (检测 `window.React` 或 `#root`)
- ✅ Vue (检测 `window.Vue` 或 `[data-v-]`)
- ✅ Angular (检测 `window.angular` 或 `[ng-version]`)

### 自动功能
检测到 SPA 后自动启用：
- 路由变化监听（pushState/replaceState）
- DOM 变化监听（MutationObserver）
- 元素自动重新挂载
- ResizeObserver 跟踪

---

## 核心功能

### 点击处理 (`onClick`)

注册点击区域，支持：
- 选择器或 DOM 元素
- 元素等待（`waitFor`）
- 自动刷新边界（`autoRefresh`）
- 调试模式（`debug`）

### URL 打开 (`openURL`)

在默认浏览器中打开链接。

### 边界管理

- `refreshBounds()`: 手动刷新所有边界
- `clearHandlers()`: 清除所有处理器

### 调试工具

- `enableDebug()`: 启用调试模式
- 红色边框显示点击区域
- 详细控制台日志

---

## 事件通信

### Native → JavaScript

SDK 监听以下自定义事件：

```javascript
// 点击事件（由 C++ 鼠标钩子触发）
window.addEventListener('AnyWP:click', function(event) {
  // event.detail = { x, y }
});

// 交互模式变化
window.addEventListener('AnyWP:interactionMode', function(event) {
  // event.detail = { enabled }
});
```

### JavaScript → Native

通过 `postMessage` 发送：

```javascript
window.chrome.webview.postMessage({
  type: 'ready',
  name: 'My Wallpaper'
});

window.chrome.webview.postMessage({
  type: 'openURL',
  url: 'https://example.com'
});

window.chrome.webview.postMessage({
  type: 'IFRAME_DATA',
  iframes: [...]
});
```

---

## 性能优化

### ResizeObserver

自动跟踪元素变化，无需轮询。

```javascript
// 自动创建 ResizeObserver
AnyWP.onClick('#btn', callback, { autoRefresh: true });
```

### MutationObserver

监听 DOM 变化，智能刷新。

### 防抖和节流

路由切换后延迟 500ms 刷新，避免频繁计算。

---

## 兼容性

### 浏览器

运行在 WebView2（Chromium Edge）环境中，支持所有现代 Web 特性：
- ✅ ES6+
- ✅ MutationObserver
- ✅ ResizeObserver
- ✅ CustomEvent
- ✅ Promise
- ✅ async/await

### 框架

- ✅ React 16+
- ✅ Vue 2/3
- ✅ Angular 2+
- ✅ 原生 HTML/JS
- ✅ 任何其他 Web 框架

---

## 调试技巧

### 1. 检查 SDK 加载

```javascript
if (window.AnyWP) {
  console.log('SDK 版本:', AnyWP.version);
  console.log('SPA 模式:', AnyWP._spaMode);
} else {
  console.error('SDK 未加载！');
}
```

### 2. 查看注册的处理器

```javascript
console.log('已注册:', AnyWP._clickHandlers.length, '个处理器');
```

### 3. 显示调试边框

```javascript
AnyWP.onClick('#btn', callback, { debug: true });
```

### 4. 手动触发点击

```javascript
AnyWP._handleClick(500, 300);
```

---

## 常见问题

### Q: SDK 显示 vN/A？

**A**: SDK 未正确注入。检查：
1. HTML 中是否引用了 SDK
2. 控制台是否有加载错误
3. 尝试刷新壁纸

### Q: 点击没反应？

**A**: 检查：
1. 鼠标穿透是否开启（Mouse Transparent = ON）
2. 元素是否已注册（启用 debug 模式）
3. 控制台是否有错误

### Q: SPA 路由切换后失效？

**A**: 使用 `waitFor` 选项或手动 `refreshBounds()`

### Q: iframe 能否点击？

**A**: 可以。SDK 支持 iframe 广告点击检测。

---

## 内部实现

### 初始化流程

```
1. IIFE 自执行
2. 创建 window.AnyWP 对象
3. _init() 初始化
4. _detectSPA() 检测框架
5. _setupEventListeners() 监听事件
6. _setupSPAMonitoring() (如果是 SPA)
```

### 点击处理流程

```
用户点击 → 鼠标钩子捕获 → C++ 转发 → 
dispatchEvent('AnyWP:click') → _handleClick() → 
遍历 _clickHandlers → 匹配边界 → 触发回调
```

### SPA 监听流程

```
检测到 SPA → override history API → 监听路由 → 
DOM 变化 → 刷新边界 → 重新绑定元素
```

---

## 更新日志

详见 [CHANGELOG_CN.md](../CHANGELOG_CN.md)

---

## 相关文件

- `anywp_engine_plugin.cpp`: C++ 插件实现
- `anywp_engine.dart`: Flutter API
- `examples/*.html`: 示例页面

---

**维护**: AnyWP Engine Team  
**最后更新**: 2025-11-03

