# 🎯 拖拽功能快速测试

## 测试页面已准备就绪

应用已编译并启动，请按以下步骤测试：

### 第1步：加载测试页面

在 Flutter 应用中输入以下 URL：

```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_drag_auto.html
```

### 第2步：启动壁纸

1. 保持 "Enable Mouse Transparency" 勾选（默认）
2. 点击 **"Start"** 按钮
3. 等待 3 秒

### 第3步：查看自动测试报告

页面中央会显示大大的测试报告，应该看到：

```
✓ 所有检查通过
SDK 版本: 4.2.0
makeDraggable: EXISTS
拖拽处理器注册: SUCCESS
```

### 第4步：测试拖拽

拖动页面下方的**黄色边框方块**：
- 方块应该跟随鼠标移动
- 松开后位置自动保存
- 停止并重新启动壁纸，方块会恢复到上次位置

## 🎨 其他测试页面

### test_draggable.html - 完整演示
```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_draggable.html
```
包含多个可拖拽元素，带按钮控制

### test_drag_final.html - 调试版本
```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_drag_final.html
```
右侧显示所有 console.log 输出，便于调试

## ✅ 预期结果

- ✅ 拖拽流畅
- ✅ 位置保存到 Registry
- ✅ 重启后恢复位置
- ✅ 鼠标在空白处点击穿透到桌面
- ✅ 壁纸在桌面图标下方

