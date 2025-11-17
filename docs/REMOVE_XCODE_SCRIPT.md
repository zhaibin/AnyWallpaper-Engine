# 移除 Xcode 中的嵌入脚本

## 问题原因

我们之前添加的嵌入 FlutterMacOS 框架的 Run Script Phase 干扰了 Flutter 的正常构建流程，导致：
- `App.framework` 和 `flutter_assets` 没有被正确构建和嵌入
- Flutter 找不到应用包（期望 `example.app` 但实际是 `Runner.app`）

## 解决方案：移除自定义脚本

### 步骤

1. **打开 Xcode 项目**
   ```bash
   open example/macos/Runner.xcworkspace
   ```

2. **选择 Runner Target**
   - 在左侧项目导航器中，点击 `Runner` 项目
   - 在 TARGETS 列表中选择 `Runner`

3. **进入 Build Phases**
   - 点击顶部的 `Build Phases` 标签

4. **找到并删除自定义脚本**
   - 查找名为 `"Run Script"` 或 `"Embed Flutter Framework"` 的 Phase
   - 这个脚本包含以下内容：
     ```bash
     #!/bin/bash
     set -e
     
     if [ "$CONFIGURATION" == "Debug" ]; then
         FRAMEWORK_DIR="$FLUTTER_ROOT/bin/cache/artifacts/engine/darwin-x64/FlutterMacOS.xcframework/macos-arm64_x86_64"
     else
         FRAMEWORK_DIR="$FLUTTER_ROOT/bin/cache/artifacts/engine/darwin-x64-release/FlutterMacOS.xcframework/macos-arm64_x86_64"
     fi
     
     if [ -d "$FRAMEWORK_DIR/FlutterMacOS.framework" ]; then
         mkdir -p "$TARGET_BUILD_DIR/$FRAMEWORKS_FOLDER_PATH"
         rsync -av --delete "$FRAMEWORK_DIR/FlutterMacOS.framework" "$TARGET_BUILD_DIR/$FRAMEWORKS_FOLDER_PATH/"
         echo "✅ FlutterMacOS.framework embedded"
     fi
     ```
   - 点击该 Phase 左侧的 `-` 按钮删除它

5. **保存并关闭 Xcode**
   - `Cmd + S` 保存
   - `Cmd + Q` 退出 Xcode

## 为什么要移除？

Flutter 的构建系统已经内置了框架嵌入逻辑：
- `flutter run` 会自动处理所有框架的嵌入
- `App.framework` 和 `flutter_assets` 应该由 Flutter 的构建脚本自动生成
- 我们的自定义脚本反而干扰了这个流程

## 正确的构建方式

**移除脚本后，只需使用标准 Flutter 命令：**

```bash
# 开发调试
cd example
flutter run -d macos

# 生产构建
flutter build macos --release
```

Flutter 会自动：
✅ 构建 `App.framework`（包含 Dart 代码）
✅ 嵌入 `flutter_assets`（包含资源文件）
✅ 嵌入 `FlutterMacOS.framework`（Flutter 引擎）
✅ 嵌入所有插件框架（`anywp_engine.framework` 等）

## 验证

移除脚本并重新构建后，检查应用包内容：

```bash
ls -la build/macos/Build/Products/Debug/Runner.app/Contents/Frameworks/
```

应该看到：
- ✅ `FlutterMacOS.framework`
- ✅ `App.framework`
- ✅ `anywp_engine.framework`
- ✅ 其他插件框架

```bash
ls -la build/macos/Build/Products/Debug/Runner.app/Contents/Frameworks/App.framework/Resources/
```

应该看到：
- ✅ `flutter_assets/` 目录

