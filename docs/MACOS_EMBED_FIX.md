# macOS FlutterMacOS 框架嵌入问题修复指南

## 问题描述

在 Xcode 中直接构建和运行 macOS 应用时，会出现以下错误：

```
dyld: Library not loaded: @rpath/FlutterMacOS.framework/Versions/A/FlutterMacOS
Reason: tried: '.../Runner.app/Contents/Frameworks/FlutterMacOS.framework/Versions/A/FlutterMacOS' (no such file)
```

**原因**：Flutter 的 macOS 构建系统没有正确配置 Xcode 项目，导致 FlutterMacOS 框架未被嵌入到应用包中。

## 解决方案

### 方案 1：使用 flutter run（推荐用于开发）

```bash
cd example
flutter run -d macos
```

这是最简单的方式，Flutter 工具会自动处理框架嵌入和热重载。

### 方案 2：手动复制框架（临时解决）

每次在 Xcode 中构建后运行此脚本：

```bash
# 找到 DerivedData 中的构建路径
DERIVED_DATA=$(find ~/Library/Developer/Xcode/DerivedData -name "Runner-*" -type d | head -1)

# 复制 FlutterMacOS 框架
cp -R ~/Dev/flutter/bin/cache/artifacts/engine/darwin-x64/FlutterMacOS.xcframework/macos-arm64_x86_64/FlutterMacOS.framework \
  "$DERIVED_DATA/Build/Products/Debug/Runner.app/Contents/Frameworks/"
```

或使用我们提供的自动化脚本：

```bash
cd anywp-engine
./scripts/embed_flutter_framework.sh
```

### 方案 3：添加 Xcode Build Phase（永久修复）

在 Xcode 中为 Runner target 添加自动嵌入脚本：

#### 步骤 1：打开 Xcode 项目

```bash
cd example/macos
open Runner.xcworkspace
```

#### 步骤 2：添加 Build Phase

1. 在左侧导航栏选择 **Runner** 项目
2. 选择 **Runner** target
3. 切换到 **Build Phases** 标签
4. 点击左上角的 **+** 按钮
5. 选择 **New Run Script Phase**
6. 将新建的 Run Script 拖动到 **Embed Frameworks** 之后
7. 展开 Run Script，在脚本框中粘贴：

```bash
#!/bin/bash
set -e

echo "🔧 Embedding FlutterMacOS framework..."

# 根据构建配置选择框架路径
if [ "$CONFIGURATION" == "Debug" ]; then
    FLUTTER_FRAMEWORK_DIR="$FLUTTER_ROOT/bin/cache/artifacts/engine/darwin-x64/FlutterMacOS.xcframework/macos-arm64_x86_64"
else
    FLUTTER_FRAMEWORK_DIR="$FLUTTER_ROOT/bin/cache/artifacts/engine/darwin-x64-release/FlutterMacOS.xcframework/macos-arm64_x86_64"
fi

FLUTTER_FRAMEWORK="$FLUTTER_FRAMEWORK_DIR/FlutterMacOS.framework"
DEST_FRAMEWORKS_DIR="$TARGET_BUILD_DIR/$FRAMEWORKS_FOLDER_PATH"

# 检查并复制框架
if [ -d "$FLUTTER_FRAMEWORK" ]; then
    mkdir -p "$DEST_FRAMEWORKS_DIR"
    rsync -av --delete "$FLUTTER_FRAMEWORK" "$DEST_FRAMEWORKS_DIR/"
    echo "✅ FlutterMacOS.framework embedded"
else
    echo "❌ FlutterMacOS.framework not found at: $FLUTTER_FRAMEWORK"
    exit 1
fi
```

8. 设置脚本名称为 `Embed Flutter Framework`
9. 确保勾选 **Run script only when installing**（可选）

#### 步骤 3：重新构建

1. 清理构建：**Product > Clean Build Folder** (`Cmd + Shift + K`)
2. 重新运行：**Product > Run** (`Cmd + R`)

## 验证修复

构建成功后，检查框架是否已嵌入：

```bash
# 对于 flutter build 方式
ls -la example/build/macos/Build/Products/Debug/Runner.app/Contents/Frameworks/FlutterMacOS.framework

# 对于 Xcode 构建方式
DERIVED_DATA=$(find ~/Library/Developer/Xcode/DerivedData -name "Runner-*" -type d | head -1)
ls -la "$DERIVED_DATA/Build/Products/Debug/Runner.app/Contents/Frameworks/FlutterMacOS.framework"
```

如果看到框架目录和文件，说明嵌入成功。

## 常见问题

### Q: 为什么 flutter build 可以工作，但 Xcode 不行？

A: `flutter build macos` 使用 Flutter 工具链的自定义构建脚本，会自动处理框架嵌入。而直接在 Xcode 中构建时，这些脚本不会被执行。

### Q: 每次清理构建都需要重新复制框架吗？

A: 如果使用方案 2（手动复制），是的。使用方案 3（添加 Build Phase）可以自动化这个过程。

### Q: Release 构建需要不同的框架路径吗？

A: 是的。Debug 使用 `darwin-x64`，Release 使用 `darwin-x64-release`。我们的脚本会自动根据配置选择正确的路径。

### Q: 可以在 Xcode 中使用热重载吗？

A: 不能。热重载是 Flutter 工具链的功能，只能通过 `flutter run` 使用。在 Xcode 中只能传统的停止-修改-重新运行。

## 建议的开发工作流

1. **日常开发**：使用 `flutter run -d macos` 进行开发和调试，享受热重载
2. **Native 调试**：需要调试原生代码时，在 Xcode 中运行（已添加 Build Phase 自动嵌入框架）
3. **发布构建**：使用 `flutter build macos --release` 构建最终产品

## 相关文档

- [MACOS_BUILD_FIX.md](./MACOS_BUILD_FIX.md) - macOS 编译问题修复指南
- [MACOS_DEBUG_GUIDE.md](./MACOS_DEBUG_GUIDE.md) - macOS 调试指南
- [MACOS_DEVELOPER_GUIDE.md](./MACOS_DEVELOPER_GUIDE.md) - macOS 开发指南

