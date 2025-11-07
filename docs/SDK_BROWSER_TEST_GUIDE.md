# Web SDK 浏览器测试指南

**版本**: v4.2.0 (模块化架构)  
**测试页面**: `examples/test_sdk_browser.html`  
**更新日期**: 2025-11-07

---

## 🚀 快速开始

### 打开测试页面

```bash
# 方法1: Chrome
chrome examples/test_sdk_browser.html

# 方法2: 默认浏览器
start examples/test_sdk_browser.html

# 方法3: PowerShell
cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine
$path = Resolve-Path "examples\test_sdk_browser.html"
Start-Process "chrome.exe" "file:///$($path -replace '\\','/')"
```

---

## 📋 测试内容

### 1. SDK 加载测试 ✅

**测试项**:
- SDK 对象存在 (`window.AnyWP`)
- SDK 版本信息 (`v4.2.0`)
- DPI 缩放值
- 屏幕尺寸

**预期结果**:
```
✅ SDK 对象存在: window.AnyWP
✅ SDK 版本: 4.2.0
✅ DPI 缩放: 1x (或你的实际DPI)
✅ 屏幕尺寸: 1920x1080 (或你的实际尺寸)
```

---

### 2. API 可用性测试 ✅

**测试项** (11个API):
```javascript
- onClick()           // 点击处理
- makeDraggable()     // 拖拽功能
- removeDraggable()   // 移除拖拽
- resetPosition()     // 重置位置
- saveState()         // 保存状态
- loadState()         // 加载状态
- clearState()        // 清空状态
- onMouse()           // 鼠标事件
- onKeyboard()        // 键盘事件
- refreshBounds()     // 刷新边界
- clearHandlers()     // 清空处理器
```

**预期结果**:
```
✅ onClick: function
✅ makeDraggable: function
✅ removeDraggable: function
... (所有API应显示为function)
```

---

### 3. 模块结构测试 ✅

**测试项**:
- 内部状态管理 (`_clickHandlers`, `_draggableElements`, `_persistedState`)
- 交互模式状态
- SPA 模式检测

**预期结果**:
```
✅ 内部状态管理: 已初始化
✅ 交互模式: 已启用
✅ SPA 模式: 已检测 或 未检测 (取决于框架)
```

---

### 4. 拖拽功能测试 🎯

#### 自动测试

点击 **"测试拖拽"** 按钮后:

**预期行为**:
1. 三个彩色方块变为可拖拽状态
2. 日志显示: "✅ 拖拽功能设置完成 - 3 个元素可拖动"
3. 状态面板显示: "✅ 拖拽功能已启用"

#### 手动测试

**操作步骤**:
1. 用鼠标点击并按住任意彩色方块
2. 拖动到新位置
3. 松开鼠标

**预期日志**:
```
📦 box1 开始拖拽: (150, 450)
✅ box1 拖拽结束: (350, 550)
```

**预期效果**:
- 方块跟随鼠标移动
- 拖动时方块有阴影效果 (`dragging` 类)
- 松开后方块固定在新位置
- 位置自动保存到 localStorage

#### 拖拽元素

| 元素 | 颜色 | 初始位置 |
|------|------|---------|
| 方块 #1 | Pink渐变 | (100, 400) |
| 方块 #2 | Blue渐变 | (300, 400) |
| 方块 #3 | Green渐变 | (500, 400) |

---

### 5. 存储功能测试 💾

#### 操作

点击 **"测试存储"** 按钮

**测试流程**:
1. 生成随机测试数据 (`{time, value}`)
2. 调用 `AnyWP.saveState('test_key', data)`
3. 延迟500ms后调用 `AnyWP.loadState('test_key', callback)`
4. 验证加载的数据是否与保存的一致

**预期日志**:
```
💾 测试存储功能...
✅ 数据已保存: {"time":"2025-11-07T...","value":0.12345}
✅ 数据加载成功: {"time":"2025-11-07T...","value":0.12345}
```

**注意**: 
- 在浏览器环境下，使用 `localStorage` 作为后端
- 在实际 Flutter 应用中，会使用 Windows Registry

---

## 🎨 页面功能

### 1. 实时日志面板

- 黑色背景的终端风格日志
- 颜色编码:
  - 🔵 蓝色: 信息日志
  - 🟢 绿色: 成功日志
  - 🟠 橙色: 警告日志
  - 🔴 红色: 错误日志
- 自动滚动到最新日志
- 可通过 **"清空日志"** 按钮清空

### 2. 测试卡片

四个测试模块卡片，展示测试结果:
- 📦 SDK 加载
- 🔧 API 可用性
- 📐 模块架构
- ⚡ 功能测试

### 3. 交互按钮

- **测试拖拽**: 启用三个方块的拖拽功能
- **测试存储**: 测试状态保存/加载
- **清空日志**: 清空日志面板

---

## ✅ 测试检查清单

### 基础测试

- [ ] SDK 加载成功
- [ ] 版本号显示为 v4.2.0
- [ ] DPI 和屏幕尺寸正确显示
- [ ] 所有11个API都可用 (function)
- [ ] 内部状态正确初始化

### 拖拽测试

- [ ] 点击"测试拖拽"按钮成功
- [ ] 可以拖动 Pink 方块
- [ ] 可以拖动 Blue 方块
- [ ] 可以拖动 Green 方块
- [ ] 拖动时有视觉反馈 (阴影)
- [ ] 松开后方块固定位置
- [ ] 日志显示拖拽开始/结束坐标

### 存储测试

- [ ] 点击"测试存储"按钮成功
- [ ] 日志显示"数据已保存"
- [ ] 日志显示"数据加载成功"
- [ ] 加载的数据与保存的一致

### 持久化测试

- [ ] 拖动方块到新位置
- [ ] 刷新浏览器页面 (F5)
- [ ] 方块恢复到上次拖动的位置

---

## 🐛 故障排查

### 问题1: SDK 未加载

**症状**: 
- 显示 "❌ SDK 对象存在: 未找到"
- 日志显示 "❌ SDK 加载失败"

**解决**:
1. 检查 `windows/anywp_sdk.js` 文件是否存在
2. 打开浏览器控制台 (F12) 查看错误信息
3. 确认文件路径正确 (`../windows/anywp_sdk.js`)

### 问题2: 拖拽不工作

**症状**:
- 点击方块无反应
- 日志没有拖拽相关消息

**解决**:
1. 先点击"测试拖拽"按钮
2. 确认日志显示 "✅ 拖拽功能设置完成"
3. 在浏览器中，需要按住鼠标左键拖动

**注意**: 
- 浏览器测试使用标准 DOM 事件
- 实际 Flutter 应用使用鼠标钩子 (更强大)

### 问题3: 存储不工作

**症状**:
- 点击"测试存储"后无反应
- 或显示 "⚠️ 数据加载失败"

**解决**:
1. 检查浏览器是否禁用了 localStorage
2. 打开控制台查看 localStorage: `localStorage.getItem('AnyWP_test_key')`
3. 清空 localStorage 重试: `localStorage.clear()`

---

## 🔧 高级测试

### 1. 性能测试

```javascript
// 在浏览器控制台执行
console.time('SDK Init');
// 刷新页面
console.timeEnd('SDK Init');
// 应该 < 100ms
```

### 2. 内存测试

```javascript
// 在浏览器控制台执行
console.memory.usedJSHeapSize / 1024 / 1024
// SDK 应该 < 5MB
```

### 3. API 压力测试

```javascript
// 大量拖拽操作
for (let i = 0; i < 100; i++) {
  AnyWP.makeDraggable('#box1', {
    persistKey: 'test_' + i
  });
}
// 应该不卡顿
```

---

## 📊 测试报告模板

```markdown
## SDK v4.2.0 测试报告

**测试日期**: YYYY-MM-DD
**测试环境**: Chrome/Edge/Firefox
**操作系统**: Windows 11

### 测试结果

- ✅ SDK 加载: 通过
- ✅ API 可用性: 11/11 通过
- ✅ 模块结构: 通过
- ✅ 拖拽功能: 通过
- ✅ 存储功能: 通过

### 性能指标

- 加载时间: 65ms
- 内存占用: 3.2MB
- 拖拽响应: 流畅

### 问题

- 无

### 结论

✅ 所有测试通过，可以投入生产使用
```

---

## 📚 相关文档

- [SDK 重构报告](./SDK_REFACTORING_REPORT.md)
- [生产构建测试](./SDK_PRODUCTION_BUILD_TEST.md)
- [Web 开发者指南](./WEB_DEVELOPER_GUIDE_CN.md)
- [API 使用示例](./API_USAGE_EXAMPLES.md)

---

## 🎯 下一步

测试通过后，可以：

1. **集成到实际项目**: 将 SDK 集成到你的 Flutter 应用
2. **性能优化**: 使用压缩版 `anywp_sdk.min.js`
3. **功能扩展**: 基于模块化架构添加新功能
4. **发布应用**: 打包并发布你的桌面壁纸应用

---

**快乐测试！** 🎉

