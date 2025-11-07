# 🌐 AnyWP Engine - Web 开发者文档导航

**你是 Web 开发者，想开发可作为桌面壁纸的网页内容？**

这里是你需要的所有文档！

---

## 🚀 快速开始（5分钟）

### 第一个互动壁纸

```html
<!DOCTYPE html>
<html>
<head>
  <title>我的壁纸</title>
</head>
<body>
  <button id="myButton">点击我</button>

  <script>
    // SDK 自动注入到 window.AnyWP
    if (window.AnyWP) {
      // 通知壁纸已就绪
      AnyWP.ready('我的壁纸');

      // 注册点击区域
      AnyWP.onClick('#myButton', (x, y) => {
        console.log('按钮被点击了！');
        AnyWP.openURL('https://example.com');
      });
    }
  </script>
</body>
</html>
```

**完整教程** → [WEB_DEVELOPER_GUIDE_CN.md - 第1节](WEB_DEVELOPER_GUIDE_CN.md#快速开始)

---

## 📚 核心文档（必读）

### ⭐ 完整 SDK 指南

**[WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md)** (688行)

**这是你的核心文档！包含所有你需要的内容：**

#### 📖 章节目录

1. **快速开始** - 5分钟上手
2. **核心 API** - 所有方法详解
   - `ready()`, `onClick()`, `openURL()`
   - `onVisibilityChange()` - 省电优化 🆕
   - `onMouse()`, `onKeyboard()`
   - `refreshBounds()`, `clearHandlers()`
   - `enableDebug()`, `setSPAMode()`

3. **React 集成** - 完整教程
   - useEffect Hook 使用
   - React Router 处理
   - 最佳实践
   - 避坑指南

4. **Vue 集成** - 完整教程
   - Vue 3 Composition API
   - Vue 2 Options API
   - Vue Router 处理
   - 最佳实践

5. **SPA 支持** - 单页应用
   - 自动检测（React/Vue/Angular）
   - 路由变化处理
   - 动态内容处理
   - 手动配置

6. **调试技巧**
   - 启用调试模式
   - 查看点击区域
   - 验证坐标
   - 常见问题

7. **性能优化**
   - 减少注册数量
   - 禁用不必要刷新
   - 延迟初始化
   - 及时清理

8. **常见问题 FAQ**

**立即阅读** → [WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md)

---

### 🌍 英文版本

**[WEB_DEVELOPER_GUIDE.md](WEB_DEVELOPER_GUIDE.md)** (642行)
- 与中文版内容相同
- 适合国际开发者

---

## 🔧 进阶文档（可选）

### 技术原理深入

**[API_BRIDGE.md](API_BRIDGE.md)** (721行)

**包含内容：**
- C++/JavaScript 桥接架构
- 完整的事件传递流程图（7步详解）
- 鼠标钩子实现原理
- 像素坐标系统详解
- 可见性 API 工作原理
- 安全机制说明

**适合：** 想深入理解底层实现的开发者

**何时阅读：**
- 遇到复杂问题需要理解原理
- 想优化性能需要了解底层
- 纯粹的技术好奇心

**阅读** → [API_BRIDGE.md](API_BRIDGE.md)

---

## 📝 实际示例代码

**examples/ 目录下的 HTML 文件：**

| 文件 | 内容 | 适合场景 |
|-----|------|---------|
| **test_simple.html** | 基础示例 | 新手入门 |
| **test_react.html** | React 集成 | React 开发者 |
| **test_vue.html** | Vue 集成 | Vue 开发者 |
| **test_visibility.html** | 可见性 API | 省电优化 🆕 |
| **test_api.html** | 完整 API 演示 | 功能测试 |
| **test_iframe_ads.html** | 复杂场景 | 广告支持 |

**使用方法：**
1. 启动 AnyWP Engine 示例应用
2. 输入文件路径，例如：
   ```
   file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_simple.html
   ```
3. 点击 "Start Wallpaper"
4. 观察效果和控制台日志

---

## 🎯 SDK API 快速参考

### 核心方法

```javascript
// 通知就绪
AnyWP.ready('My Wallpaper')

// 注册点击（基础）
AnyWP.onClick('#button', (x, y) => {
  console.log('Clicked!');
})

// 注册点击（SPA推荐）
AnyWP.onClick('.dynamic-btn', callback, {
  waitFor: true,       // 等待元素出现
  debug: true          // 显示调试边框
})

// 打开链接
AnyWP.openURL('https://example.com')

// 刷新边界（路由切换后）
AnyWP.refreshBounds()

// 清除处理器（组件卸载时）
AnyWP.clearHandlers()

// 监听可见性变化（省电）🆕
AnyWP.onVisibilityChange((visible) => {
  if (visible) {
    resumeAnimations();  // 恢复动画
  } else {
    pauseAnimations();   // 暂停以省电
  }
})
```

### SDK 属性

```javascript
AnyWP.version         // "4.1.0"
AnyWP.dpiScale        // 1.5 (DPI缩放)
AnyWP.screenWidth     // 2880 (物理像素)
AnyWP.screenHeight    // 1620
AnyWP.interactionEnabled  // true/false
```

---

## 💡 常见场景快速查找

### 我想...

**开发 React 壁纸**
→ [WEB_DEVELOPER_GUIDE_CN.md#react-集成](WEB_DEVELOPER_GUIDE_CN.md#⚛️-react-集成)

**开发 Vue 壁纸**
→ [WEB_DEVELOPER_GUIDE_CN.md#vue-集成](WEB_DEVELOPER_GUIDE_CN.md#💚-vue-集成)

**处理 SPA 路由**
→ [WEB_DEVELOPER_GUIDE_CN.md#spa-支持](WEB_DEVELOPER_GUIDE_CN.md#🔄-spa-单页应用支持)

**调试点击不工作**
→ [WEB_DEVELOPER_GUIDE_CN.md#调试技巧](WEB_DEVELOPER_GUIDE_CN.md#🐛-调试技巧)

**优化性能**
→ [WEB_DEVELOPER_GUIDE_CN.md#性能优化](WEB_DEVELOPER_GUIDE_CN.md#⚡-性能优化)

**省电优化（暂停视频）**
→ [WEB_DEVELOPER_GUIDE_CN.md#onvisibilitychange](WEB_DEVELOPER_GUIDE_CN.md#onvisibilitychange-callback-🆕)

**理解点击是如何工作的**
→ [API_BRIDGE.md#完整交互流程](API_BRIDGE.md#🎯-complete-interaction-flow)

---

## 🎓 学习检查清单

### 基础（必须掌握）
- [ ] 知道如何注册点击区域
- [ ] 知道如何打开链接
- [ ] 理解 DPI 缩放（SDK自动处理）
- [ ] 能启用调试模式查看边界

### 框架集成（如果使用框架）
- [ ] React: 能在 useEffect 中注册
- [ ] React: 能处理路由切换
- [ ] Vue: 能在 onMounted 中注册
- [ ] Vue: 能处理路由切换
- [ ] 知道何时使用 `waitFor` 选项

### 优化（推荐掌握）
- [ ] 知道如何监听可见性变化
- [ ] 能暂停/恢复自定义动画以省电
- [ ] 理解视频/音频自动管理
- [ ] 能减少注册数量优化性能
- [ ] 能正确清理处理器避免内存泄漏

### 高级（可选）
- [ ] 理解底层事件传递机制
- [ ] 理解物理像素和CSS像素转换
- [ ] 能处理复杂的动态内容
- [ ] 能调试和诊断问题

---

## 📊 SDK 版本

**当前版本：** 4.1.0

**主要功能：**
- ✅ 点击区域注册
- ✅ 链接打开
- ✅ SPA 框架支持（React/Vue/Angular）
- ✅ 自动边界刷新
- ✅ 调试模式
- ✅ **可见性 API**（v4.1.0 新增）
- ✅ **自动暂停视频/音频**（v4.1.0 新增）

**版本历史** → [SDK_CHANGELOG.md](SDK_CHANGELOG.md)

---

## 🆘 需要帮助？

### 查看示例代码
→ `examples/` 目录下的 HTML 文件

### 查看常见问题
→ [WEB_DEVELOPER_GUIDE_CN.md - FAQ](WEB_DEVELOPER_GUIDE_CN.md#❓-常见问题)

### 理解技术原理
→ [API_BRIDGE.md](API_BRIDGE.md)

### 查看更新历史
→ [SDK_CHANGELOG.md](SDK_CHANGELOG.md)

---

**返回文档中心** → [docs/README.md](README.md)

**查看 Flutter 开发者文档** → [FOR_FLUTTER_DEVELOPERS.md](FOR_FLUTTER_DEVELOPERS.md)

