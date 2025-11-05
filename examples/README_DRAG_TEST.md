# 拖拽功能测试指南

## ✨ 架构亮点

**核心特性**：基于 C++ 鼠标钩子实现拖拽，**保持鼠标透明窗口**的同时支持交互！

- ✅ 壁纸在桌面图标**下方**（WorkerW 层）
- ✅ 鼠标透明窗口（WS_EX_TRANSPARENT）保持启用
- ✅ C++ 鼠标钩子捕获全局鼠标事件
- ✅ 事件转发到 WebView2（mousedown, mousemove, mouseup）
- ✅ 不在元素上时，点击穿透到桌面图标

## 🎯 快速开始

### 第1步：启动壁纸
1. 在 Flutter 应用中输入 URL：`file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_draggable.html`
2. **保持 "Enable Mouse Transparency" 勾选**（默认）
3. 点击 **"Start"** 按钮
4. 等待壁纸加载完成

### 第2步：测试拖拽
1. **交互模式默认已启用**，无需额外操作
2. 直接用鼠标拖动以下元素：
   - 粉红色方块 "拖动我 #1"
   - 数据面板卡片
   - 音乐播放器卡片
3. 松开鼠标，位置会自动保存到 Windows Registry

### 第3步：测试鼠标穿透
1. 在**空白区域**（不在拖拽元素上）点击
2. 点击会**穿透到桌面**，可以正常操作桌面图标
3. 这证明了壁纸真正在图标下方！

### 第4步：测试状态持久化
1. 拖动元素到新位置
2. 点击 **"Stop"** 停止壁纸
3. 再次点击 **"Start"** 重新启动
4. **元素会自动恢复到上次拖拽的位置！** ✨

## 🔧 功能特性

### ✅ 已实现
- ✅ 元素拖拽（支持任意 DOM 元素）
- ✅ 拖拽边界限制
- ✅ 位置自动保存到 Windows Registry
- ✅ 重启后自动恢复位置
- ✅ 拖拽回调函数（onDragStart, onDrag, onDragEnd）
- ✅ 实时状态显示

### 📊 测试元素
1. **粉红色方块** - 基础拖拽测试
2. **数据面板** - 带内容的卡片拖拽
3. **音乐播放器** - 复杂元素拖拽

## ⚠️ 常见问题

### Q: 拖拽不工作？
**A:** 检查以下几点：
1. 页面右下角状态显示是否为 "✅ SDK 已加载 - 交互已启用"？
2. 是否看到 "🖱️ 鼠标钩子: 已激活"？
3. 浏览器控制台是否有错误？
4. 确认正在使用最新的 anywp_sdk.js

### Q: 鼠标透明是否需要禁用？
**A:** **不需要！** 这正是这个架构的亮点。拖拽功能通过鼠标钩子实现，即使窗口透明也能工作。

### Q: 位置没有保存？
**A:** 检查：
1. Windows Registry 权限是否正常
2. 控制台是否显示 "[AnyWP] [State] Saved to registry"
3. 拖拽后是否看到 "💾 保存状态: 已保存位置" 提示

### Q: 如何清除保存的位置？
**A:** 点击 "清除所有状态" 按钮，或者手动删除注册表项：
```
HKEY_CURRENT_USER\Software\AnyWPEngine\State
```

## 📝 技术细节

### 架构设计

#### 鼠标事件流程
```
用户拖动鼠标
    ↓
C++ 鼠标钩子捕获 (LowLevelMouseProc)
    ↓
检查是否被应用窗口遮挡
    ↓
转发到 WebView2 (SendClickToWebView)
    ↓
派发 CustomEvent ('AnyWP:mouse')
    ↓
JavaScript SDK 处理拖拽逻辑
    ↓
更新元素位置 + 保存状态
```

#### 关键代码位置
- **C++ 鼠标钩子**: `windows/anywp_engine_plugin.cpp` - `LowLevelMouseProc()`
- **事件转发**: `SendClickToWebView()` 
- **JavaScript 拖拽**: `windows/anywp_sdk.js` - `makeDraggable()`
- **状态持久化**: C++ `SaveState()` / JavaScript `_saveElementPosition()`

### 状态保存位置
- **路径**: `HKEY_CURRENT_USER\Software\AnyWPEngine\State`
- **格式**: JSON 字符串（例如：`{"left": 100, "top": 200}`）
- **键名**: 元素 ID + `_position`（例如：`box1_position`）

### API 使用示例
```javascript
// 使元素可拖拽并自动保存位置
AnyWP.makeDraggable('#myElement', {
  persistKey: 'myElement_position',  // 持久化标识
  bounds: { left: 0, top: 0, right: 1920, bottom: 1080 },
  onDragStart: function(pos) {
    console.log('开始拖拽', pos);
  },
  onDragEnd: function(pos) {
    console.log('拖拽结束', pos);
  }
});

// 手动保存状态
AnyWP.saveState('myKey', JSON.stringify({ x: 100, y: 200 }));

// 加载状态
AnyWP.loadState('myKey', function(value) {
  if (value) {
    const data = JSON.parse(value);
    console.log('位置:', data.x, data.y);
  }
});
```

## 🚀 下一步

你可以：
1. 修改 `test_draggable.html` 添加更多可拖拽元素
2. 实现自己的交互式壁纸
3. 使用状态持久化保存用户的任何设置（主题、音量等）

## 💡 提示

- ✅ 拖拽功能**支持鼠标透明窗口**（通过鼠标钩子实现）
- ✅ 壁纸真正在桌面图标下方（WorkerW 层）
- ✅ 状态保存在 Windows Registry，跨应用重启持久化
- ✅ 支持保存任意 JSON 数据，不仅限于位置
- ✅ 可以为每个显示器设置不同的壁纸内容
- ✅ mousemove 事件优化：只在拖拽时处理以保持性能

