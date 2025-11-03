# React/Vue SPA 集成测试说明

## 🐛 问题已修复

**根本原因**：`InjectAnyWallpaperSDK()` 函数从未被调用

**修复内容**：
1. ✅ 修复 SDK 文件路径（v4.0.0）
2. ✅ 在 WebView2 初始化时调用 `InjectAnyWallpaperSDK()`
3. ✅ 默认 URL 改为 `test_react.html`

---

## 📋 测试步骤

### 方法 1：使用默认 URL（推荐）

1. **完全关闭应用**（如果正在运行）
   ```powershell
   taskkill /F /IM anywallpaper_engine_example.exe
   ```

2. **启动应用**
   ```bash
   cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine\example
   .\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
   ```

3. **点击 "Start Wallpaper" 按钮**
   - 应自动加载 `test_react.html`

### 方法 2：手动输入 URL

1. **启动应用**

2. **在 URL 输入框中输入**：
   ```
   file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_react.html
   ```
   
3. **点击 "Start Wallpaper" 按钮**

---

## ✅ 预期结果

### 1. 状态栏显示（页面顶部）

```
🚀 AnyWP SDK v4.0.0          ← 不再是 vN/A
⚛️ React v18.3.1
🔄 SPA Mode: ✅ Enabled      ← 不再是 Disabled
📊 DPI Scale: 2x (或你的实际缩放比例)
```

### 2. 点击功能正常

- ✅ 点击 **"+ Increment"** → 计数器加 1
- ✅ 点击 **"- Decrement"** → 计数器减 1
- ✅ 点击 **"↻ Reset"** → 计数器归零
- ✅ 点击卡片（GitHub、React Docs、AnyWP Docs）→ 在浏览器打开

### 3. Event Log 有输出（页面底部黑色区域）

```
[时间] AnyWP SDK initialized (v4.0.0)
[时间] SPA Mode: Enabled
[时间] Increment button clicked at (x,y)
[时间] Counter: 1
[时间] Card 1 clicked - Opening GitHub
```

### 4. 调试边框（红色边框）

每个可点击区域周围应该有红色边框和阴影。

---

## 🐛 如果仍然显示 vN/A

### 步骤 1：检查日志

```powershell
# 查找最新日志
$log = Get-ChildItem "E:\Projects\AnyWallpaper\AnyWallpaper-Engine" -Filter "*.log" | 
       Sort-Object LastWriteTime -Descending | Select-Object -First 1

# 查找 SDK 注入信息
Get-Content $log.FullName | Select-String "Inject|SDK|anywp_sdk"
```

**应该看到**：
```
[AnyWP] [API] Injecting AnyWallpaper SDK...
[AnyWP] [API] Loading AnyWallpaper SDK script...
[AnyWP] [API] SDK script loaded (XXXXX bytes)
[AnyWP] [API] SDK injected successfully, ID: xxxxx
```

### 步骤 2：验证 SDK 文件

```powershell
# 检查 SDK 文件是否存在
$sdkPath = "E:\Projects\AnyWallpaper\AnyWallpaper-Engine\example\build\windows\x64\runner\Debug\data\flutter_assets\packages\anywp_engine\windows\anywp_sdk.js"

if (Test-Path $sdkPath) {
    Write-Host "✅ SDK file EXISTS"
    Get-Content $sdkPath -Head 2
} else {
    Write-Host "❌ SDK file NOT FOUND"
}
```

**应该显示**：
```
✅ SDK file EXISTS
// AnyWP Engine SDK v4.0.0 - JavaScript Bridge
// Auto-injected into WebView2
```

### 步骤 3：完全重新编译

```bash
cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine\example

# 1. 停止应用
taskkill /F /IM anywallpaper_engine_example.exe

# 2. 完全清理
Remove-Item -Recurse -Force build
flutter clean

# 3. 重新编译
flutter build windows --debug

# 4. 验证 SDK 文件
Get-Content "build\windows\x64\runner\Debug\data\flutter_assets\packages\anywp_engine\windows\anywp_sdk.js" -Head 1

# 5. 运行
.\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
```

---

## 🔍 调试技巧

### 1. 查看 WebView2 控制台（如果需要）

在 C++ 代码中临时添加：
```cpp
// 在 ConfigurePermissions() 之后
webview_->OpenDevToolsWindow();  // 打开开发者工具
```

### 2. 检查是否加载了正确的 URL

查看日志中的：
```
[AnyWP] Navigating to: file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_react.html
```

如果看到的是 `test_simple.html`，说明 URL 没有更新。

### 3. 强制刷新

在应用中：
1. 点击 "Stop Wallpaper"
2. 确认 URL 是 `test_react.html`
3. 点击 "Start Wallpaper"

---

## 📝 测试 Vue 集成

测试完 React 后，可以测试 Vue：

```
file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_vue.html
```

**预期结果**：
- 💚 Vue v3.x
- 🔄 SPA Mode: ✅ Enabled
- ✅ 标签页切换正常（Home、Counter、Todos）
- ✅ Todo List 添加/删除功能正常
- ✅ Counter 按钮点击正常

---

## 💡 常见问题

### Q: 为什么我看到的是 test_simple.html？

**A**: 应用可能缓存了旧的 URL。解决方法：
1. 停止壁纸
2. 手动输入 `test_react.html` 的完整路径
3. 重新启动壁纸

### Q: SDK 版本显示 v3.1.0 而不是 v4.0.0？

**A**: 使用了嵌入式 SDK（说明文件加载失败）。检查：
```powershell
# 检查文件是否存在
Test-Path "...\data\flutter_assets\packages\anywp_engine\windows\anywp_sdk.js"
```

如果不存在，重新编译：
```bash
flutter clean
flutter build windows --debug
```

### Q: 点击没有反应？

**A**: 检查以下几点：
1. SDK 是否正确注入（检查版本号）
2. 是否启用了 SPA Mode
3. Event Log 是否有输出
4. 控制台是否有错误

---

## 📊 完整测试清单

- [ ] SDK 版本显示 **v4.0.0**（不是 vN/A）
- [ ] SPA Mode 显示 **✅ Enabled**（不是 Disabled）
- [ ] React 版本显示 **v18.3.1**
- [ ] Increment 按钮点击有效
- [ ] Decrement 按钮点击有效
- [ ] Reset 按钮点击有效
- [ ] 三个卡片点击打开浏览器
- [ ] Event Log 有输出
- [ ] 红色调试边框显示
- [ ] 控制台无错误

---

## 🎉 成功标志

当你看到以下界面时，说明集成成功：

```
┌─────────────────────────────────────────────┐
│ 🎨 AnyWP React Integration                  │
│ Interactive Desktop Wallpaper with React    │
├─────────────────────────────────────────────┤
│ 🚀 AnyWP SDK v4.0.0                        │
│ ⚛️ React v18.3.1                            │
│ 🔄 SPA Mode: ✅ Enabled                     │
│ 📊 DPI Scale: 2x                            │
├─────────────────────────────────────────────┤
│ [GitHub]  [React Docs]  [AnyWP Docs]       │
├─────────────────────────────────────────────┤
│ Interactive Counter                          │
│            42                                │
│  [+ Increment] [- Decrement] [↻ Reset]     │
├─────────────────────────────────────────────┤
│ 📋 Event Log:                               │
│ [时间] AnyWP SDK initialized (v4.0.0)      │
│ [时间] SPA Mode: Enabled                    │
│ [时间] Increment clicked at (x,y)          │
└─────────────────────────────────────────────┘
```

---

**祝测试顺利！🚀**

如有问题，请提供：
1. 日志输出（特别是 SDK 相关的）
2. 屏幕截图
3. 控制台错误信息

