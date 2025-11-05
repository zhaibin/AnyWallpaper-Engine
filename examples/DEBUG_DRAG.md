# 拖拽功能调试指南

## 🐛 如果拖拽不工作，请按以下步骤检查

### 第1步：使用调试测试页面

1. 在 Flutter 应用中输入 URL：
   ```
   file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_drag_simple.html
   ```

2. 点击 "Start" 启动壁纸

3. 查看**右侧黑色调试面板**，应该看到：
   ```
   ✅ AnyWP SDK 已加载
   版本: 4.2.0
   DPI Scale: 1.0 (或其他值)
   交互启用: true
   ✅ makeDraggable 方法存在
   ✅ makeDraggable 调用成功
   ✅ 拖拽处理器已注册
   ```

### 第2步：检查鼠标事件

1. 在桌面上移动鼠标
2. 调试面板应该显示：
   ```
   🖱️ AnyWP:mouse - mousemove (x, y)
   ```
   每 10 个事件记录一次

3. 点击桌面任意位置
4. 应该看到：
   ```
   🖱️ AnyWP:mouse - mousedown (x, y)
   🖱️ AnyWP:mouse - mouseup (x, y)
   ```

### 第3步：测试拖拽

1. 点击并按住粉红色方块
2. 应该看到：
   ```
   🎯 拖拽开始: (x, y)
   ```

3. 移动鼠标（按住鼠标左键）
4. 方块应该跟随鼠标移动

5. 松开鼠标
6. 应该看到：
   ```
   🎯 拖拽结束: (x, y)
   ```

## 🔍 常见问题诊断

### 问题1: 看不到 "AnyWP SDK 已加载"

**原因**：anywp_sdk.js 未正确注入

**解决**：
1. 检查 C++ 插件是否正确编译
2. 查看 Flutter 应用的控制台输出
3. 确认 `InjectAnyWallpaperSDK()` 被调用

### 问题2: 看不到 "🖱️ AnyWP:mouse" 事件

**原因**：鼠标钩子未工作或事件未转发

**解决**：
1. 检查 C++ 插件的 `SetupMouseHook()` 是否成功
2. 检查 `LowLevelMouseProc()` 是否被调用
3. 检查控制台是否有错误信息
4. 确认 `WM_MOUSEMOVE` 转发已启用（line 1636）

### 问题3: 看到鼠标事件但拖拽不工作

**原因**：可能的情况
- `interactionEnabled = false`
- 坐标转换错误
- 元素位置错误

**解决**：
1. 确认 SDK 初始化时显示 "Interaction Enabled: true"
2. 检查 DPI Scale 是否正确
3. 在调试面板中查看 "mousedown not over element" 消息
4. 检查元素的实际位置和大小

### 问题4: 拖拽开始但无法移动

**原因**：mousemove 事件未处理

**解决**：
1. 确认看到 mousemove 事件（每10个记录一次）
2. 检查 `_dragState` 是否正确设置
3. 查看控制台是否有 JavaScript 错误

## 📋 正常工作的日志应该是这样的

```
===== 调试日志 =====
[1] 页面加载完成
[2] DPI: 1
[3] 开始检查 AnyWP SDK...
[4] ✅ AnyWP SDK 已加载
[5] 版本: 4.2.0
[6] DPI Scale: 1
[7] Screen: 1920x1080
[8] 交互启用: true
[9] ✅ makeDraggable 方法存在
[10] 开始设置拖拽...
[11] 交互模式已启用
[12] ✅ makeDraggable 调用成功
[13] ✅ 拖拽处理器已注册
[14] 🖱️ AnyWP:mouse - mousemove (856, 234)
[15] 🖱️ AnyWP:mouse - mousemove (901, 267)
[16] 🖱️ AnyWP:mouse - mousedown (156, 145)
[17] 🎯 拖拽开始: (100, 100)
[18] 🖱️ AnyWP:mouse - mouseup (234, 189)
[19] 🎯 拖拽结束: (178, 89)
```

## 🛠️ 高级调试

### 查看 C++ 日志

在 PowerShell 中运行：
```powershell
cd example\build\windows\x64\Debug\runner
.\example.exe
```

查看控制台输出，应该看到：
```
[AnyWP] Plugin initialized
[AnyWP] [API] SDK script loaded (xxxx bytes)
[AnyWP] [API] SDK injected successfully
[Native] Dispatching events at (x,y)
```

### 查看 JavaScript 控制台

如果可以打开 WebView2 的开发者工具（需要特殊配置），查看：
1. Console 选项卡的错误信息
2. Network 选项卡确认资源加载
3. Elements 选项卡查看元素位置

### 检查 Registry

查看状态是否保存：
```
HKEY_CURRENT_USER\Software\AnyWPEngine\State
```

应该看到 `test_box_position` 键（如果拖拽成功）

## 💡 提示

- 使用 `test_drag_simple.html` 进行调试，它有详细的日志输出
- 完成调试后，使用 `test_draggable.html` 进行正常使用
- 如果问题依然存在，提供调试面板的日志截图

## 📞 报告问题

如果按照以上步骤仍无法工作，请提供：
1. 调试面板的完整日志
2. Flutter 应用控制台输出
3. C++ 应用控制台输出（如果可用）
4. 屏幕录制或截图

这将帮助快速定位问题！

