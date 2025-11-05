# 拖拽功能实现总结

## ✅ 功能已完成

互动壁纸现在支持：
- ✅ 元素拖拽
- ✅ 状态持久化（保存到 Windows Registry）
- ✅ 重启后自动恢复位置
- ✅ 基于鼠标钩子，支持透明窗口
- ✅ 壁纸保持在桌面图标下方

## 🏗️ 架构设计

### 核心原理

```
用户拖动鼠标
    ↓
C++ 鼠标钩子 (LowLevelMouseProc)
    ↓
转发事件到 WebView2 (mousedown, mousemove, mouseup)
    ↓
JavaScript SDK 处理拖拽逻辑
    ↓
保存状态到 Windows Registry
```

### 关键特性

1. **鼠标透明窗口**：壁纸窗口保持 `WS_EX_TRANSPARENT` 属性
2. **鼠标钩子转发**：C++ 捕获系统级鼠标事件并转发
3. **WebView2 接收**：通过 `ExecuteScript` 派发 `AnyWP:mouse` 事件
4. **JavaScript 处理**：SDK 基于物理像素坐标判断元素命中
5. **状态持久化**：位置保存到注册表，跨会话保留

## 📋 使用方法

### SDK 加载（必需）

在 HTML 文件的 `<head>` 中添加：

```html
<script src="../windows/anywp_sdk.js"></script>
```

### 基础拖拽

```javascript
// 使元素可拖拽
AnyWP.makeDraggable('#myElement', {
  persistKey: 'myElement_position',  // 位置自动保存
  onDragStart: function(pos) {
    console.log('开始拖拽', pos);
  },
  onDragEnd: function(pos) {
    console.log('拖拽结束', pos);
  }
});
```

### 带边界限制

```javascript
AnyWP.makeDraggable('#myElement', {
  persistKey: 'myElement_pos',
  bounds: {
    left: 0,
    top: 0,
    right: 1920,
    bottom: 1080
  }
});
```

### 状态管理

```javascript
// 保存自定义状态
AnyWP.saveState('settings', JSON.stringify({ theme: 'dark', volume: 0.8 }));

// 加载状态
AnyWP.loadState('settings', function(value) {
  if (value) {
    const settings = JSON.parse(value);
    console.log('主题:', settings.theme);
  }
});

// 清除所有状态
AnyWP.clearState();
```

## 🧪 测试页面

### test_drag_auto.html（推荐）
自动测试所有功能，显示测试报告

```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_drag_auto.html
```

### test_draggable.html
完整的拖拽演示，包含多个可拖拽元素

```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_draggable.html
```

### test_drag_final.html
调试版本，显示所有控制台输出

```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_drag_final.html
```

## 📊 Dart API

### 保存和加载状态

```dart
// 保存状态
await AnyWPEngine.saveState('myKey', 'myValue');

// 加载状态
String value = await AnyWPEngine.loadState('myKey');

// 清除所有状态
await AnyWPEngine.clearState();
```

## 💾 状态存储

### 存储位置
```
HKEY_CURRENT_USER\Software\AnyWPEngine\State
```

### 数据格式
键值对，值为 JSON 字符串：
- `box1_position` = `{"left":123,"top":456}`
- `settings` = `{"theme":"dark","volume":0.8}`

## 📝 文件清单

### 核心代码
- `windows/anywp_sdk.js` - JavaScript SDK (v4.2.0)
- `windows/anywp_engine_plugin.cpp` - C++ 插件（鼠标钩子、状态持久化）
- `lib/anywp_engine.dart` - Dart API

### 测试页面
- `examples/test_draggable.html` - 完整演示
- `examples/test_drag_auto.html` - 自动测试
- `examples/test_drag_final.html` - 调试版本

### 文档
- `docs/SDK_LOADING.md` - SDK 加载指南
- `docs/DRAG_ARCHITECTURE.md` - 架构设计
- `examples/README_DRAG_TEST.md` - 使用指南
- `examples/DEBUG_DRAG.md` - 调试指南
- `QUICK_TEST.md` - 快速测试说明

## ⚠️ 重要提示

1. **必须添加 `<script src>` 标签**才能使用拖拽功能
2. **保持鼠标透明启用**（默认设置）
3. **交互模式默认已启用**（SDK v4.2.0）
4. **拖拽基于鼠标钩子**，不是 DOM 事件

## 🎯 已验证功能

- ✅ 拖拽流畅无卡顿
- ✅ 位置保存到 Registry
- ✅ 重启后自动恢复
- ✅ 鼠标穿透（空白区域点击到达桌面）
- ✅ 多元素拖拽支持
- ✅ 边界限制功能
- ✅ 拖拽回调函数

## 🚀 版本信息

- **SDK 版本**: 4.2.0
- **发布日期**: 2025-11-05
- **主要特性**: 拖拽支持 + 状态持久化
- **兼容性**: Windows 10/11, WebView2

## 📞 技术支持

如有问题，请查看：
1. `docs/SDK_LOADING.md` - SDK 加载问题
2. `examples/DEBUG_DRAG.md` - 拖拽调试
3. `QUICK_TEST.md` - 快速测试步骤

