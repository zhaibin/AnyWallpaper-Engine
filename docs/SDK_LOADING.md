# SDK 加载方式说明

## 📌 推荐方式：使用 `<script>` 标签

### 基本用法

在你的 HTML 文件的 `<head>` 部分添加：

```html
<script src="../windows/anywp_sdk.js"></script>
```

### 完整示例

```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>我的互动壁纸</title>
  
  <!-- 加载 AnyWP SDK -->
  <script src="../windows/anywp_sdk.js"></script>
  
  <style>
    /* 你的样式 */
  </style>
</head>
<body>
  <!-- 你的内容 -->
  
  <script>
    // SDK 自动初始化，直接使用
    console.log('AnyWP SDK v' + window.AnyWP.version);
    
    // 使用拖拽功能
    AnyWP.makeDraggable('#myElement', {
      persistKey: 'myElement_position',
      onDragStart: function(pos) {
        console.log('开始拖拽', pos);
      }
    });
  </script>
</body>
</html>
```

## 🛠️ 不同路径方式

### 相对路径（推荐）

适用于本地 file:// 协议的 HTML 文件：

```html
<script src="../windows/anywp_sdk.js"></script>
```

### 绝对路径

如果相对路径有问题，使用绝对路径：

```html
<script src="file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/windows/anywp_sdk.js"></script>
```

### 项目内相对路径

如果你的 HTML 在 examples 目录：

```html
<script src="../windows/anywp_sdk.js"></script>
```

## ❓ 为什么不用 C++ 注入？

### 尝试过的方案

我们尝试过让 C++ 自动注入 SDK，但遇到以下问题：
- ❌ 文件路径在不同环境下不一致
- ❌ C++ 字符串长度限制（SDK 超过 30KB）
- ❌ 编码问题（UTF-8, Unicode, BOM）
- ❌ 注入时机问题（页面加载顺序）

### `<script src>` 的优势

- ✅ **简单可靠**：浏览器原生机制
- ✅ **性能更好**：浏览器优化加载
- ✅ **调试方便**：开发者工具可以看到
- ✅ **灵活**：可以加载不同版本
- ✅ **已验证**：test_iframe_ads.html 等页面都用这个方式

## 📋 SDK 功能

加载 SDK 后可使用以下功能：

### 拖拽功能（v4.2.0+）

```javascript
AnyWP.makeDraggable('#element', {
  persistKey: 'element_pos',  // 位置自动保存
  bounds: { left: 0, top: 0, right: 1920, bottom: 1080 },
  onDragStart: (pos) => console.log('开始', pos),
  onDrag: (pos) => console.log('拖拽中', pos),
  onDragEnd: (pos) => console.log('结束', pos)
});
```

### 状态持久化

```javascript
// 保存
AnyWP.saveState('myKey', JSON.stringify({ theme: 'dark' }));

// 加载
AnyWP.loadState('myKey', function(value) {
  if (value) {
    const data = JSON.parse(value);
    console.log('主题:', data.theme);
  }
});

// 清除
AnyWP.clearState();
```

### 点击处理

```javascript
AnyWP.onClick('#button', function(x, y) {
  console.log('按钮被点击', x, y);
}, { debug: true });
```

### 鼠标和键盘事件

```javascript
AnyWP.onMouse(function(event) {
  console.log('鼠标事件:', event.type, event.x, event.y);
});

AnyWP.onKeyboard(function(event) {
  console.log('键盘事件:', event.key);
});
```

## 🔧 故障排除

### SDK 未加载

如果 `window.AnyWP` 不存在：

1. **检查 `<script>` 标签**是否正确添加
2. **检查路径**是否正确（相对于 HTML 文件）
3. **查看浏览器控制台**是否有加载错误
4. **确认 anywp_sdk.js 文件**存在于 `windows/` 目录

### 版本检查

```javascript
if (window.AnyWP) {
  console.log('SDK 版本:', window.AnyWP.version);
  
  if (window.AnyWP.version.startsWith('4.2')) {
    console.log('✓ 支持拖拽功能');
  } else {
    console.log('× 版本过旧，请更新');
  }
} else {
  console.error('× SDK 未加载');
}
```

## 📚 参考示例

查看以下文件了解如何使用：

- `examples/test_draggable.html` - 拖拽功能完整演示
- `examples/test_drag_auto.html` - 自动测试页面
- `examples/test_drag_final.html` - 调试版本
- `examples/test_iframe_ads.html` - 基础用法

## 💡 最佳实践

1. **总是在 HTML 中添加** `<script src>` 标签
2. **不要依赖** C++ 自动注入
3. **检查 SDK 版本**确保功能可用
4. **使用相对路径**便于移植
5. **查看示例文件**学习最佳实践

