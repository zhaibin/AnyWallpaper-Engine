# macOS 调试指南

本指南介绍如何在 macOS 平台上调试 AnyWP Engine 插件。

---

## 环境要求

### 必需软件
- **macOS**: 10.14 (Mojave) 或更高
- **Xcode**: 13.0+ (推荐最新版本)
- **Flutter**: 3.0+ (已配置 macOS 支持)
- **CocoaPods**: 1.11+ (用于依赖管理)

### 检查环境

```bash
# 检查 Flutter macOS 支持
flutter doctor

# 确保输出包含：
# [✓] Xcode - develop for iOS and macOS
```

---

## 快速开始

### 方法 1: 使用 Xcode 调试（推荐）

**优点**: 完整的调试功能、断点、变量查看、内存分析

#### 步骤 1: 生成 Xcode 项目

```bash
cd example
flutter build macos --debug
```

#### 步骤 2: 打开 Xcode 项目

```bash
# 打开 Runner 项目
open macos/Runner.xcworkspace

# 如果没有 .xcworkspace，使用 .xcodeproj
open macos/Runner.xcodeproj
```

#### 步骤 3: 配置 Scheme

1. 在 Xcode 顶部菜单栏，点击 **Product → Scheme → Edit Scheme...**
2. 选择左侧的 **Run**
3. **Info** 标签页:
   - Build Configuration: **Debug**
   - Executable: **Runner.app**
4. **Arguments** 标签页（可选）:
   - 添加启动参数，如 `--verbose`
5. **Options** 标签页:
   - 确保 "Debug executable" 已勾选

#### 步骤 4: 设置断点

在 Objective-C 代码中设置断点：

```objective-c
// macos/Classes/AnyWPEnginePlugin.m

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
    // 在这里设置断点 🔴
    NSLog(@"Method called: %@", call.method);
    
    if ([@"initializeWallpaper" isEqualToString:call.method]) {
        // 断点会在这里暂停 🔴
        NSString *url = call.arguments[@"url"];
        // ...
    }
}
```

#### 步骤 5: 启动调试

1. 选择目标设备: **My Mac**
2. 点击运行按钮 ▶️ 或按 `Cmd + R`
3. 应用启动后，断点会自动触发

#### 步骤 6: 调试技巧

**查看变量**:
- 鼠标悬停在变量上查看值
- 在底部 **Debug Area** 的 **Variables View** 中查看所有变量

**控制台输出**:
- 在底部 **Debug Area** 的 **Console** 中查看 `NSLog` 输出

**LLDB 命令**:
```lldb
# 打印变量
(lldb) po url
(lldb) po call.arguments

# 继续执行
(lldb) c

# 单步执行
(lldb) n

# 进入函数
(lldb) s
```

---

### 方法 2: 使用 Flutter 命令行调试

**优点**: 快速启动、适合简单调试

#### 步骤 1: 运行应用

```bash
cd example
flutter run -d macos --debug
```

#### 步骤 2: 查看输出

Flutter 会自动显示：
- Dart 代码的日志输出
- 原生代码的 `NSLog` 输出
- 错误和警告信息

#### 步骤 3: 热重载

```bash
# 在命令行中按 'r' 进行热重载
r

# 按 'R' 进行完整重启
R

# 按 'q' 退出
q
```

---

### 方法 3: 使用 VS Code 调试

**优点**: 统一的开发环境、Dart 代码调试方便

#### 配置 launch.json

在项目根目录创建 `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Flutter (macOS)",
      "request": "launch",
      "type": "dart",
      "program": "example/lib/main.dart",
      "cwd": "example",
      "args": [
        "-d",
        "macos"
      ]
    }
  ]
}
```

#### 启动调试

1. 打开 `example/lib/main.dart`
2. 设置 Dart 断点（点击行号左侧）
3. 按 `F5` 或点击 **Run → Start Debugging**
4. 选择 **Flutter (macOS)** 配置

---

## 日志查看

### 方法 1: 使用自定义 Logger

在 Objective-C 代码中:

```objective-c
#import "../Utils/Logger.h"

[AWPLogger log:@"Normal message"];
[AWPLogger warn:@"Warning message"];
[AWPLogger error:@"Error message"];
```

### 方法 2: 系统控制台

**Console.app** (系统应用):

1. 打开 **应用程序 → 实用工具 → 控制台**
2. 在左侧选择你的 Mac
3. 在搜索框输入: `anywallpaper` 或 `AnyWP`
4. 实时查看所有日志

**命令行查看日志**:

```bash
# 实时查看系统日志
log stream --predicate 'process == "anywallpaper_engine_example"' --level debug

# 查看最近的日志
log show --predicate 'process == "anywallpaper_engine_example"' --last 5m

# 查看特定级别的日志
log show --predicate 'eventMessage contains "AnyWP"' --last 1h --info
```

### 方法 3: 文件日志

Logger 默认也会输出到文件（如果已配置）:

```bash
# 查看日志文件
tail -f ~/Library/Logs/AnyWPEngine/debug.log
```

---

## 调试常见问题

### 1. WebView 内容调试

**启用 Web Inspector**:

在 `WallpaperManager.m` 中启用开发者工具:

```objective-c
// 在创建 WKWebView 之前
WKPreferences *preferences = [[WKPreferences alloc] init];
#ifdef DEBUG
preferences.valueForKey:@"developerExtrasEnabled"] = @YES;
#endif
config.preferences = preferences;
```

**打开 Web Inspector**:

1. 运行应用并加载壁纸
2. 打开 **Safari → Develop → [Your Mac Name] → [Wallpaper Window]**
3. 使用 Safari 开发者工具调试 JavaScript

### 2. WKWebView 渲染问题

**检查窗口层级**:

```objective-c
// 打印窗口信息
NSLog(@"Window level: %ld", (long)window.level);
NSLog(@"Window order: %ld", (long)window.orderedIndex);

// 检查是否在桌面层级
if (window.level != kCGDesktopWindowLevel) {
    NSLog(@"❌ Window level incorrect!");
}
```

**检查 WebView 是否加载**:

```objective-c
// 添加导航代理
- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    [AWPLogger log:@"✅ WebView finished loading"];
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error {
    [AWPLogger error:[NSString stringWithFormat:@"❌ WebView load failed: %@", error.localizedDescription]];
}
```

### 3. 消息通信调试

**Flutter → JavaScript 消息**:

在 `MessageBridge.m` 中添加详细日志:

```objective-c
- (BOOL)sendMessageToWebView:(WKWebView *)webView message:(NSString *)message {
    NSLog(@"📤 Sending message to WebView");
    NSLog(@"   Message: %@", message);
    
    // 执行脚本
    [webView evaluateJavaScript:script completionHandler:^(id result, NSError *error) {
        if (error) {
            NSLog(@"❌ Script execution failed: %@", error);
        } else {
            NSLog(@"✅ Script executed successfully");
            NSLog(@"   Result: %@", result);
        }
    }];
    
    return YES;
}
```

**JavaScript → Flutter 消息**:

在 `MessageBridge.m` 的 `didReceiveScriptMessage` 中:

```objective-c
- (void)userContentController:(WKUserContentController *)userContentController
      didReceiveScriptMessage:(WKScriptMessage *)message {
    
    NSLog(@"📥 Received message from JavaScript");
    NSLog(@"   Name: %@", message.name);
    NSLog(@"   Body: %@", message.body);
    NSLog(@"   Body type: %@", NSStringFromClass([message.body class]));
    
    // 处理消息...
}
```

### 4. 内存泄漏检测

**使用 Instruments**:

1. 在 Xcode 中，选择 **Product → Profile** (或 `Cmd + I`)
2. 选择 **Leaks** 模板
3. 点击录制按钮
4. 操作应用，查看内存泄漏

**使用 Xcode Memory Graph**:

1. 运行应用
2. 在 Xcode 底部工具栏，点击 **Debug Memory Graph** 按钮
3. 查看对象引用关系
4. 查找循环引用

### 5. 性能分析

**CPU 性能**:

```bash
# 使用 sample 工具
sample anywallpaper_engine_example 10 -file cpu_profile.txt

# 查看结果
open cpu_profile.txt
```

**内存使用**:

```bash
# 监控内存
while true; do
  ps aux | grep anywallpaper_engine_example | grep -v grep
  sleep 1
done
```

---

## 调试技巧

### 1. 符号化崩溃日志

如果应用崩溃，查看崩溃日志:

```bash
# 崩溃日志位置
~/Library/Logs/DiagnosticReports/

# 查看最新的崩溃日志
ls -lt ~/Library/Logs/DiagnosticReports/ | head -5
```

### 2. 启用详细日志

在代码中启用详细日志级别:

```objective-c
// Logger.m
- (instancetype)init {
    self = [super init];
    if (self) {
#ifdef DEBUG
        self.logLevel = AWPLogLevelDebug;  // 调试模式：输出所有日志
#else
        self.logLevel = AWPLogLevelInfo;    // 发布模式：只输出重要日志
#endif
    }
    return self;
}
```

### 3. 条件断点

在 Xcode 中设置条件断点:

1. 设置断点
2. 右键断点 → **Edit Breakpoint...**
3. 添加条件，例如: `monitorIndex == 1`
4. 添加动作，例如: `Log Message: Monitor @monitorIndex@ selected`

### 4. 异常断点

捕获所有 Objective-C 异常:

1. 在 Xcode 左侧导航器，选择 **Breakpoint Navigator** (⌘ + 8)
2. 点击左下角的 **+** → **Exception Breakpoint...**
3. Exception: **Objective-C**
4. Break: **On Throw**

### 5. 使用 OSLog

使用系统日志框架:

```objective-c
#import <os/log.h>

os_log_t logger = os_log_create("com.anywp.engine", "wallpaper");

os_log_info(logger, "Initializing wallpaper: %{public}@", url);
os_log_error(logger, "Failed to create window: %{public}@", error);
```

---

## 测试检查清单

在 macOS 上调试时，请确保测试以下功能:

### 基础功能
- [ ] 应用启动成功
- [ ] 壁纸窗口创建成功
- [ ] WebView 加载 URL 成功
- [ ] 壁纸显示在桌面图标后面

### 多显示器
- [ ] 枚举所有显示器
- [ ] 在主显示器上创建壁纸
- [ ] 在次显示器上创建壁纸
- [ ] 显示器拔插处理

### 消息通信
- [ ] Flutter → JavaScript 消息发送
- [ ] JavaScript → Flutter 消息接收
- [ ] 消息格式正确（JSON）
- [ ] 错误处理正确

### 电源管理
- [ ] 屏幕锁定时暂停壁纸
- [ ] 屏幕解锁时恢复壁纸
- [ ] 进入睡眠时处理

### 内存和性能
- [ ] 无内存泄漏
- [ ] CPU 使用率正常
- [ ] 内存使用稳定
- [ ] 长时间运行无崩溃

---

## 常用调试命令

```bash
# === 构建和运行 ===

# 清理项目
flutter clean

# 获取依赖
flutter pub get

# 运行（调试模式）
flutter run -d macos --debug

# 运行（详细输出）
flutter run -d macos --debug --verbose

# 构建（调试模式）
flutter build macos --debug

# 构建（发布模式）
flutter build macos --release

# === 日志查看 ===

# 实时查看系统日志（过滤进程）
log stream --process anywallpaper_engine_example --level debug

# 查看最近 10 分钟的日志
log show --predicate 'process == "anywallpaper_engine_example"' --last 10m

# 查看包含特定文本的日志
log show --predicate 'eventMessage contains "AnyWP"' --info

# === 进程管理 ===

# 查看进程
ps aux | grep anywallpaper

# 终止进程
killall anywallpaper_engine_example

# 监控 CPU 和内存
top -pid $(pgrep anywallpaper_engine_example)

# === Xcode 工具 ===

# 清理 Xcode 缓存
rm -rf ~/Library/Developer/Xcode/DerivedData/*

# 重置 Xcode
defaults delete com.apple.dt.Xcode

# === CocoaPods ===

# 安装依赖
cd macos && pod install

# 更新依赖
cd macos && pod update

# 清理 CocoaPods 缓存
pod cache clean --all
```

---

## 故障排查

### 问题 1: "No development team selected"

**解决方案**:
1. 打开 Xcode
2. 选择 **Runner** 项目
3. 在 **Signing & Capabilities** 标签页
4. 选择你的 **Team**（Apple ID）

### 问题 2: WebView 不显示内容

**检查清单**:
- [ ] URL 是否正确
- [ ] 网络连接是否正常
- [ ] Info.plist 是否允许网络访问
- [ ] WebView 是否添加到窗口
- [ ] 窗口是否可见

**解决方案**:
```objective-c
// 检查 WebView 是否加载
[webView.configuration.userContentController addScriptMessageHandler:self name:@"test"];

// 在 JavaScript 中测试
window.webkit.messageHandlers.test.postMessage("Hello from WebView!");
```

### 问题 3: 消息无法发送

**检查清单**:
- [ ] MessageBridge 是否正确初始化
- [ ] WallpaperManager 引用是否设置
- [ ] WebView 是否已加载完成
- [ ] 消息格式是否正确

**解决方案**:
添加详细日志，参考上面的"消息通信调试"部分。

---

## 相关资源

### 官方文档
- [Flutter macOS 开发](https://docs.flutter.dev/development/platform-integration/macos/building)
- [WKWebView 文档](https://developer.apple.com/documentation/webkit/wkwebview)
- [AppKit 文档](https://developer.apple.com/documentation/appkit)

### 调试工具
- [Xcode](https://developer.apple.com/xcode/)
- [Instruments](https://developer.apple.com/xcode/features/)
- [Console.app](https://support.apple.com/guide/console/welcome/mac)

### 项目文档
- [多平台架构设计](./MULTIPLATFORM_ARCHITECTURE.md)
- [macOS 开发指南](./MACOS_DEVELOPER_GUIDE.md)
- [macOS 支持总结](./MULTIPLATFORM_ARCHITECTURE.md)

---

**版本**: v2.2.0  
**更新日期**: 2025-11-17  
**维护者**: AnyWP Team

