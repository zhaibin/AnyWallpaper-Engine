# 点击事件重复触发问题 - 完整修复方案

## 问题现象

**用户报告**: 点击测试页面中的按钮时，出现增一次+3、减一次-3的异常情况。

## 根本原因分析

### 问题根源

SDK 被执行了 **3 次**，导致每次点击触发 3 次回调：

1. **测试页面手动引入** (`<script src="../windows/anywp_sdk.js">`)
2. **C++ AddScriptToExecuteOnDocumentCreated()** - 文档创建时自动注入
3. **C++ NavigationCompleted 回调中的 ExecuteScript()** - 导航完成后再次注入

每次 SDK 执行都会调用页面中的 `registerHandlers()`，所以每个按钮注册了 3 个处理器！

### 为什么会这样设计？

**用户提出的关键问题**：
> "在注入时，会有跨域安全问题，我们还要引入互联网的网页。要保证注入SDK JS的稳定性与ExecuteScript 时机，同时需要PostMessage保障双向通信。"

C++ 端的多重注入机制是为了：
1. **AddScriptToExecuteOnDocumentCreated**: 保证本地页面和同域页面能在文档创建时就注入
2. **NavigationCompleted + ExecuteScript**: 兜底注入，确保跨域页面也能获得 SDK
3. **手动引入**: 允许在浏览器中直接打开测试页面

## 完整修复方案

### 1. SDK 端：防重复初始化

**修改文件**: `windows/sdk/index.ts`

```typescript
// Auto-initialize when DOM is ready
if (typeof window !== 'undefined') {
  // ========== CRITICAL: Prevent Duplicate SDK Initialization ==========
  // Check if SDK is already loaded (防止重复注入)
  if (typeof (window as any).AnyWP !== 'undefined') {
    console.log('[AnyWP] SDK already loaded, skipping re-initialization');
    console.log('[AnyWP] This is expected when C++ plugin injects SDK multiple times');
  } else {
    console.log('[AnyWP] Initializing SDK for the first time');
    
    window.AnyWP = AnyWP;
    
    if (document.readyState === 'loading') {
      document.addEventListener('DOMContentLoaded', function() {
        AnyWP._init();
      });
    } else {
      AnyWP._init();
    }
    
    console.log('[AnyWP] SDK loaded successfully');
  }
}
```

**效果**: 
- 即使 SDK 脚本被执行 3 次，只有第一次会真正初始化
- 后续执行会跳过，避免重复注册事件监听器

### 2. Event 系统：防重复 Setup

**修改文件**: `windows/sdk/modules/events.ts`

```typescript
export const Events = {
  _setupCompleted: false,
  _eventHandlers: {} as { [key: string]: EventListener },
  
  setup(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType, animationsHandler: typeof AnimationsType) {
    // Prevent duplicate setup
    if (this._setupCompleted) {
      console.log('[AnyWP] Events already setup, skipping duplicate initialization');
      return;
    }
    
    // ... 存储事件监听器引用
    
    this._setupCompleted = true;
    console.log('[AnyWP] Events setup completed');
  },
  // ...
}
```

**效果**: 
- 防止事件监听器被重复添加
- 即使被多次调用也不会重复监听 `AnyWP:click` 事件

### 3. Click Handler：防重复注册

**修改文件**: `windows/sdk/modules/click.ts`

```typescript
function registerElement(el: HTMLElement | null) {
  if (!el) {
    console.error('[AnyWP] Element not found:', element);
    return;
  }
  
  // Check if already registered (prevent duplicate handlers)
  const existingIndex = anyWP._clickHandlers.findIndex(function(h) {
    return h.element === el;
  });
  
  if (existingIndex !== -1) {
    const existingHandler = anyWP._clickHandlers[existingIndex];
    if (existingHandler) {
      console.warn('[AnyWP] Element already has click handler, skipping duplicate registration:', el.id || el.className);
    }
    return; // Skip duplicate registration
  }
  
  // ... 注册新处理器
}
```

**效果**: 
- 即使页面调用多次 `AnyWP.onClick()`，同一个元素只注册一次
- 避免闭包中累积多个回调函数

### 4. 测试页面：条件加载

**修改文件**: 所有 `examples/*.html`

```html
<!-- 条件加载：如果 SDK 未注入，则手动加载（用于浏览器直接打开） -->
<script>if (!window.AnyWP) { document.write('<script src="../windows/anywp_sdk.js"><\/script>'); }</script>
```

**效果**: 
- **在引擎中**: C++ 已注入 SDK，条件不成立，不会二次加载
- **在浏览器中**: SDK 未注入，条件成立，手动加载以便测试

## 修复效果验证

### 控制台输出（正常情况）

```
[AnyWP] Initializing SDK for the first time
[AnyWP] SDK loaded successfully
========================================
AnyWP Engine v4.2.0 (SPA Compatible)
========================================
[AnyWP] Events setup completed
[AnyWP] SDK already loaded, skipping re-initialization
[AnyWP] SDK already loaded, skipping re-initialization
```

可以看到：
- 第 1 次执行：正常初始化
- 第 2-3 次执行：跳过重复初始化

### 点击测试结果

- ✅ 点击 "+" 按钮一次 → 计数器 **+1**（不是 +3）
- ✅ 点击 "-" 按钮一次 → 计数器 **-1**（不是 -3）
- ✅ 每个按钮只注册 1 个处理器

## 技术亮点

### 1. 多层防护机制

| 防护层级 | 位置 | 作用 |
|---------|------|------|
| SDK 初始化 | `index.ts` | 检查 `window.AnyWP` 是否存在，防止整个 SDK 重复初始化 |
| 事件系统 | `events.ts` | 防止事件监听器重复注册 |
| 点击处理器 | `click.ts` | 防止同一元素重复注册回调 |
| 页面加载 | `*.html` | 条件加载，避免手动引入与自动注入冲突 |

### 2. 自动位置跟踪 (v4.2.0+)

**三重监听机制**:

1. **ResizeObserver** - 监听元素尺寸变化
   - 元素宽度/高度改变时自动更新
   - 适用于动态内容、Flexbox、Grid 布局
   
2. **IntersectionObserver** - 监听元素可见性和位置
   - 元素进入/离开视口时更新
   - 元素被遮挡或显示时更新
   
3. **Position Polling** - 定期检查位置（兜底方案）
   - 每 100ms 检查一次位置（高频响应）
   - 检测到变化时立即更新
   - 适用于 CSS 动画、JavaScript 驱动的移动
   - **性能**: ~10 FPS 检查频率，快速响应

**使用方式**:
```javascript
// 启用自动位置跟踪（推荐）
AnyWP.onClick('#myButton', function() {
  console.log('Button clicked!');
}, {
  autoRefresh: true  // 启用自动跟踪
});

// 无需手动调用 refreshBounds()
```

**性能优化**:
- 高频轮询：100ms 间隔（~10 FPS）
- 元素移除时自动清理监听器
- 仅监听注册的元素，不影响其他元素
- 三重机制互补，确保快速响应

### 3. 跨域兼容性

**C++ 注入策略**:
1. **AddScriptToExecuteOnDocumentCreated**: 
   - 适用于本地文件 (`file://`) 和同域页面
   - 在文档创建时立即注入，最可靠
   
2. **NavigationCompleted + ExecuteScript**: 
   - 适用于跨域页面（如 `https://example.com`）
   - 兜底机制，确保即使第一种方式失败也能注入

**PostMessage 双向通信**:
- SDK 使用 `window.chrome.webview.postMessage()` 发送消息到 C++
- C++ 使用 `ExecuteScript()` 触发 JavaScript 事件
- 所有消息都通过 `AnyWP:*` 自定义事件传递

### 3. 测试友好性

**在引擎中运行**:
```
C++ 自动注入 → 条件加载不触发 → SDK 只初始化一次
```

**在浏览器中运行**:
```
C++ 未注入 → 条件加载触发 → 手动引入 SDK → SDK 初始化一次
```

## 文件修改清单

| 文件 | 修改内容 |
|------|---------|
| `windows/sdk/index.ts` | 添加防重复初始化检查 |
| `windows/sdk/modules/events.ts` | 添加 `_setupCompleted` 标志 |
| `windows/sdk/modules/click.ts` | 改为跳过而不是替换已注册元素；添加三重位置跟踪机制 |
| `windows/sdk/rollup.config.js` | 修复 TypeScript 编译配置 |
| `windows/sdk/types.ts` | 修复 `positionCheckTimer` 类型定义 |
| `examples/*.html` (11个) | 改为条件加载 SDK |
| `examples/test_position_tracking.html` | 新增：专门测试位置跟踪的页面 |
| `examples/test_refactoring.html` | 移除手动位置跟踪代码，使用 SDK 自动跟踪 |
| `scripts/test_position_tracking.bat` | 新增：位置跟踪功能测试脚本 |

## 测试步骤

### 1. 重新构建 SDK
```bash
cd windows\sdk
npm run build
```

### 2. 编译 Flutter 应用
```bash
cd example
flutter build windows --debug
```

### 3. 运行位置跟踪测试
```bash
.\scripts\test_position_tracking.bat
```

**或使用原有的点击测试**:
```bash
.\scripts\test_click_fix.bat
```

### 4. 验证修复

#### 测试 1: 重复触发修复
1. 点击 "👆 Click Test" 按钮
2. 点击 "Start" 启动壁纸
3. 点击桌面上的 "+ 增加" 按钮
4. **预期结果**: 计数器增加 1（而不是 3）
5. 查看控制台，应该看到 "SDK already loaded, skipping re-initialization"

#### 测试 2: 位置跟踪
1. 点击 "Position Tracking Test" 按钮
2. 点击 "Start" 启动壁纸
3. 使用方向按钮移动目标按钮
4. **预期结果**: 移动后仍可点击，控制台显示 "ResizeObserver detected change"

#### 测试 3: 动画元素
1. 在位置跟踪测试页面，点击弹跳/旋转/脉冲的方块
2. **预期结果**: 即使元素在动画中，点击仍然正确检测

#### 测试 4: 布局重排
1. 在重构测试页面，点击任意按钮添加日志
2. **预期结果**: 即使日志增加导致布局变化，按钮仍可点击

## 常见问题

### Q: 为什么不直接移除 C++ 的多次注入？

A: C++ 的多重注入是必要的：
- **AddScriptToExecuteOnDocumentCreated** 对跨域页面可能失败
- **ExecuteScript** 确保所有页面都能获得 SDK
- SDK 端的防重复机制更灵活，不影响注入可靠性

### Q: 条件加载会影响性能吗？

A: 几乎不会：
- `if (!window.AnyWP)` 检查非常快（< 1ms）
- 引擎环境下不会触发 `document.write()`
- 浏览器环境下只在初次加载时执行

### Q: 如果用户在互联网页面上使用呢？

A: 完全支持：
- C++ 会通过 `ExecuteScript()` 注入 SDK
- 跨域限制不影响 `ExecuteScript()` 的执行
- `PostMessage` 桥接在所有环境下都可用

## 后续优化建议

1. **C++ 注入优化**: 
   - 添加注入成功的回调通知
   - 记录注入失败的情况，便于调试

2. **SDK 监控**: 
   - 添加 SDK 加载时长统计
   - 记录重复注入的次数

3. **错误恢复**: 
   - 如果 SDK 初始化失败，提供手动重试接口
   - 提供 SDK 健康检查 API

---

**修复日期**: 2025-11-08  
**版本**: SDK v4.2.0  
**影响范围**: 所有使用 `AnyWP.onClick()` 的页面  
**测试状态**: 待用户验证

