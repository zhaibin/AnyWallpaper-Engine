# macOS 集成故障排查指南

本文档记录了 AnyWP Engine macOS 集成过程中常见的问题和解决方案。

---

## 📋 目录

1. [dyld: Library not loaded](#1-dyld-library-not-loaded)
2. [code object is not signed](#2-code-object-is-not-signed)
3. [版本号不一致](#3-版本号不一致)
4. [网络权限问题](#4-网络权限问题)
5. [Framework 结构问题](#5-framework-结构问题)

---

## 1. dyld: Library not loaded

### 问题描述

```
dyld: Library not loaded: @rpath/anywp_engine.framework/Versions/A/anywp_engine
  Referenced from: /Applications/YourApp.app/Contents/MacOS/YourApp
  Reason: image not found
```

### 原因

Framework 未正确嵌入到应用的 `Contents/Frameworks/` 目录中。

### 解决方案

#### 方案 1: 使用源码集成（推荐）

如果你从源码集成，CocoaPods 会自动处理。确保正确运行：

```bash
cd macos
pod install
```

#### 方案 2: 使用预编译包

如果使用预编译包，需要手动复制框架：

**Step 1**: 在 Xcode 中添加 Build Phase

1. 打开 `macos/Runner.xcworkspace`
2. 选择 `Runner` target
3. 选择 `Build Phases` 标签
4. 点击 `+` -> `New Run Script Phase`
5. 将脚本移到 `Embed Frameworks` 之前
6. 添加以下脚本：

```bash
#!/bin/bash
# Copy AnyWP Engine Framework

FRAMEWORK_SOURCE="${SRCROOT}/../plugins/anywp_engine/Frameworks/anywp_engine/anywp_engine.framework"
FRAMEWORK_DEST="${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}"

if [ -d "${FRAMEWORK_SOURCE}" ]; then
    echo "Copying AnyWP Engine framework..."
    mkdir -p "${FRAMEWORK_DEST}"
    cp -R "${FRAMEWORK_SOURCE}" "${FRAMEWORK_DEST}/"
    
    # Sign the framework
    echo "Signing framework..."
    codesign --force --deep --sign - "${FRAMEWORK_DEST}/anywp_engine.framework"
    
    echo "✅ AnyWP Engine framework embedded and signed"
else
    echo "⚠️  WARNING: AnyWP Engine framework not found at ${FRAMEWORK_SOURCE}"
fi
```

**Step 2**: 清理并重新构建

```bash
cd example
flutter clean
flutter pub get
flutter build macos --release
```

---

## 2. code object is not signed

### 问题描述

```
code object is not signed at all
In subcomponent: /Applications/YourApp.app/Contents/Frameworks/anywp_engine.framework
```

### 原因

Framework 没有代码签名，macOS 安全机制拒绝加载。

### 解决方案

#### 自动签名（推荐）

在构建脚本中添加签名步骤（见上面的 Run Script Phase）。

#### 手动签名

```bash
# 进入框架目录
cd path/to/anywp_engine.framework

# 使用 ad-hoc 签名
codesign --force --deep --sign - .

# 验证签名
codesign --verify --verbose .

# 查看签名信息
codesign --display --verbose .
```

#### 验证签名结果

```bash
# 应该看到类似输出
codesign --display --verbose anywp_engine.framework

# 输出示例：
# Executable=/path/to/anywp_engine.framework/Versions/A/anywp_engine
# Identifier=anywp-engine
# Format=Mach-O thin (arm64)
# CodeDirectory v=20500 size=... flags=0x2(adhoc) ...
# Signature=adhoc
# Info.plist=not bound
# ...
```

---

## 3. 版本号不一致

### 问题描述

不同位置的版本号不一致：
- `pubspec.yaml`: 2.6.2
- `Info.plist`: 2.6.0
- `getVersion()`: 2.2.0

### 原因

多个地方硬编码了版本号，没有统一管理。

### 解决方案

**v2.6.2+ 已修复**

从 v2.6.2 开始，所有版本号已统一：

- `pubspec.yaml`: 2.6.2
- `podspec`: 2.6.2
- `getVersion()`: 2.6.2
- `getSDKVersion()`: 2.5.0 (JS SDK 独立版本)

#### 验证版本号

```bash
# 检查二进制中的版本号
strings anywp_engine.framework/Versions/A/anywp_engine | grep -E "^[0-9]+\.[0-9]+\.[0-9]+$"

# 应该输出: 2.6.2
```

#### 在 Dart 中检查版本

```dart
final version = await AnyWPEngine.getVersion();
print('Plugin version: $version'); // 应该是 2.6.2

final sdkVersion = await AnyWPEngine.getSDKVersion();
print('SDK version: $sdkVersion'); // 应该是 2.5.0
```

---

## 4. 网络权限问题

### 问题描述

HTTP 请求失败，无法加载远程内容。

### 解决方案

#### Step 1: 添加网络权限到 Entitlements

编辑 `macos/Runner/DebugProfile.entitlements` 和 `macos/Runner/Release.entitlements`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <!-- 允许网络客户端连接 -->
    <key>com.apple.security.network.client</key>
    <true/>
    
    <!-- 如果需要启动本地服务器，还需要添加 -->
    <key>com.apple.security.network.server</key>
    <true/>
    
    <!-- 其他权限... -->
</dict>
</plist>
```

#### Step 2: 清理并重新构建

```bash
flutter clean
flutter build macos --release
```

---

## 5. Framework 结构问题

### 问题描述

```
bundle format is ambiguous (could be "deep" or "shallow")
```

### 原因

Framework 内部符号链接结构不正确。

### 标准 Framework 结构

```
anywp_engine.framework/
├── Versions/
│   ├── A/
│   │   ├── anywp_engine (二进制)
│   │   ├── Headers/
│   │   ├── Modules/
│   │   └── Resources/
│   └── Current -> A (符号链接)
├── anywp_engine -> Versions/Current/anywp_engine (符号链接)
├── Headers -> Versions/Current/Headers (符号链接)
├── Modules -> Versions/Current/Modules (符号链接)
└── Resources -> Versions/Current/Resources (符号链接)
```

### 解决方案

#### 自动修复（v2.6.2+）

从 v2.6.2 开始，发布脚本会自动修复框架结构。

#### 手动修复

使用提供的修复脚本：

```bash
cd anywp-engine
bash scripts/fix_framework_structure.sh path/to/anywp_engine.framework
```

#### 验证结构

```bash
# 检查符号链接
ls -la anywp_engine.framework/

# 应该看到类似输出：
# lrwxr-xr-x  anywp_engine -> Versions/Current/anywp_engine
# lrwxr-xr-x  Headers -> Versions/Current/Headers
# lrwxr-xr-x  Resources -> Versions/Current/Resources
# drwxr-xr-x  Versions/
```

---

## 🔍 调试技巧

### 1. 检查框架是否正确嵌入

```bash
# 列出应用中的所有框架
ls -la YourApp.app/Contents/Frameworks/

# 应该看到 anywp_engine.framework
```

### 2. 检查框架依赖

```bash
# 查看应用依赖的库
otool -L YourApp.app/Contents/MacOS/YourApp | grep anywp

# 应该看到:
# @rpath/anywp_engine.framework/Versions/A/anywp_engine
```

### 3. 检查 RPATH

```bash
# 查看应用的 rpath 设置
otool -l YourApp.app/Contents/MacOS/YourApp | grep -A 2 LC_RPATH

# 应该包含:
# @executable_path/../Frameworks
```

### 4. 运行时日志

在应用启动时检查日志：

```bash
# 在 Terminal 中运行应用以查看详细日志
./YourApp.app/Contents/MacOS/YourApp
```

---

## 📚 相关文档

- [macOS 预编译包集成指南](./PRECOMPILED_MACOS_INTEGRATION.md)
- [开发者 API 参考](./DEVELOPER_API_REFERENCE.md)
- [跨平台集成指南](./CROSS_PLATFORM_INTEGRATION.md)

---

## 🆘 仍然有问题？

如果以上解决方案都不起作用：

1. 确保使用的是最新版本 (v2.6.2+)
2. 完全清理项目：
   ```bash
   flutter clean
   cd macos
   pod deintegrate
   pod install
   cd ..
   flutter build macos --release
   ```
3. 在 [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues) 中报告问题

---

**最后更新**: 2025-12-08  
**版本**: v2.6.2

