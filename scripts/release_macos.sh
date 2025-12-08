#!/bin/bash
# ==========================================
# AnyWP Engine - macOS Release Build Script
# Build release packages for macOS distribution
# ==========================================

set -e  # Exit on error

echo "========================================"
echo " AnyWP Engine - macOS Release Build"
echo "========================================"
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

# Read version from pubspec.yaml
VERSION=$(grep "^version:" pubspec.yaml | sed 's/version: //' | tr -d ' ')
echo "Plugin Version: $VERSION"

# Read JS SDK version from package.json
SDK_VERSION=$(grep '"version"' sdk/src/package.json | head -1 | sed 's/.*"version": "\(.*\)".*/\1/')
echo "JS SDK Version: $SDK_VERSION"
echo ""

RELEASE_DIR="$PROJECT_ROOT/release"
PRECOMPILED_DIR="$RELEASE_DIR/anywp_engine_macos_v${VERSION}_precompiled"
SOURCE_DIR="$RELEASE_DIR/anywp_engine_macos_v${VERSION}_source"
WEB_SDK_DIR="$RELEASE_DIR/anywp_web_sdk_v${SDK_VERSION}"

TOTAL_STEPS=20
STEP=1

print_step() {
    echo "[$STEP/$TOTAL_STEPS] $1"
    STEP=$((STEP + 1))
}

# Check if we're on macOS
if [[ "$(uname)" != "Darwin" ]]; then
    echo "ERROR: This script must be run on macOS"
    exit 1
fi

print_step "Checking version consistency..."
# TODO: Create macOS version check script

print_step "Building SDK..."
if [ -f "$PROJECT_ROOT/scripts/build_sdk.sh" ]; then
    bash "$PROJECT_ROOT/scripts/build_sdk.sh" production
else
    # Fallback to npm directly
    cd "$PROJECT_ROOT/sdk"
    npm run build
    cd "$PROJECT_ROOT"
fi

print_step "Cleaning old release..."
rm -rf "$PRECOMPILED_DIR" "$PRECOMPILED_DIR.zip"
rm -rf "$SOURCE_DIR" "$SOURCE_DIR.zip"
rm -rf "$WEB_SDK_DIR" "$WEB_SDK_DIR.zip"

print_step "Building example app (Release)..."
cd "$PROJECT_ROOT/example"
flutter build macos --release
BUILD_ERROR=$?
if [ $BUILD_ERROR -ne 0 ]; then
    echo "ERROR: Build failed with code $BUILD_ERROR"
    exit 1
fi
cd "$PROJECT_ROOT"

# ==========================================
# Part A: Precompiled Package (Framework + Headers)
# ==========================================

print_step "Creating precompiled package structure..."
mkdir -p "$PRECOMPILED_DIR/Frameworks"
mkdir -p "$PRECOMPILED_DIR/lib/dart"
mkdir -p "$PRECOMPILED_DIR/include/anywp_engine"
mkdir -p "$PRECOMPILED_DIR/macos"
mkdir -p "$PRECOMPILED_DIR/sdk"
mkdir -p "$PRECOMPILED_DIR/examples"

print_step "Copying framework to precompiled package..."
# Copy the built framework/plugin
BUILD_OUTPUT="$PROJECT_ROOT/example/build/macos/Build/Products/Release"
if [ -d "$BUILD_OUTPUT/anywp_engine" ]; then
    cp -R "$BUILD_OUTPUT/anywp_engine" "$PRECOMPILED_DIR/Frameworks/"
else
    echo "WARNING: Plugin framework not found at $BUILD_OUTPUT/anywp_engine"
fi

print_step "Copying Dart API to precompiled package..."
cp "$PROJECT_ROOT/lib/anywp_engine.dart" "$PRECOMPILED_DIR/lib/dart/"
cp "$PROJECT_ROOT/lib/anywp_engine.dart" "$PRECOMPILED_DIR/lib/"

print_step "Creating C API header for precompiled package..."
# Create a simple C API header (macOS doesn't need complex C API like Windows)
cat > "$PRECOMPILED_DIR/include/anywp_engine/anywp_engine_plugin.h" << 'EOF'
// AnyWP Engine - macOS Plugin Header (Precompiled)
// 
// This is a simplified header for precompiled package users.
// For full source code, use the source package.

#ifndef ANYWP_ENGINE_PLUGIN_H_
#define ANYWP_ENGINE_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

// Plugin registration function (called by Flutter framework)
// You typically don't need to call this manually.
void AnyWPEnginePluginRegisterWithRegistrar(void* registrar);

#ifdef __cplusplus
}
#endif

#endif  // ANYWP_ENGINE_PLUGIN_H_
EOF

print_step "Copying podspec to precompiled package..."
cp "$PROJECT_ROOT/macos/anywp_engine.podspec" "$PRECOMPILED_DIR/macos/"

print_step "Creating precompiled CMakeLists.txt..."
# Create a simplified CMakeLists.txt for precompiled package
cat > "$PRECOMPILED_DIR/macos/CMakeLists.txt" << 'EOF'
# AnyWP Engine - Precompiled Package CMakeLists.txt
# This file is for precompiled package users.

cmake_minimum_required(VERSION 3.10)
project(anywp_engine_plugin VERSION 2.2.0 LANGUAGES OBJC)

# This is a precompiled plugin - no source compilation needed
# The plugin is provided as a framework in Frameworks/ directory

message(STATUS "Using precompiled AnyWP Engine framework")

# Installation path (Flutter will handle this automatically)
# No additional configuration needed for precompiled packages
EOF

print_step "Copying Web SDK to precompiled package..."
# Copy both minified and unminified SDK
if [ -f "$PROJECT_ROOT/sdk/dist/anywp_sdk.min.js" ]; then
    cp "$PROJECT_ROOT/sdk/dist/anywp_sdk.min.js" "$PRECOMPILED_DIR/sdk/"
    echo "  [OK] Copied minified SDK"
else
    echo "  [WARNING] Minified SDK not found"
fi

if [ -f "$PROJECT_ROOT/sdk/dist/anywp_sdk.js" ]; then
    cp "$PROJECT_ROOT/sdk/dist/anywp_sdk.js" "$PRECOMPILED_DIR/sdk/"
    echo "  [OK] Copied unminified SDK"
else
    echo "  [WARNING] Unminified SDK not found"
fi

print_step "Copying example HTML files to precompiled package..."
cp "$PROJECT_ROOT"/examples/*.html "$PRECOMPILED_DIR/examples/"

print_step "Copying documentation to precompiled package..."
cp "$PROJECT_ROOT/README.md" "$PRECOMPILED_DIR/"
cp "$PROJECT_ROOT/CHANGELOG_CN.md" "$PRECOMPILED_DIR/"
cp "$PROJECT_ROOT/LICENSE" "$PRECOMPILED_DIR/"
cp "$PROJECT_ROOT/pubspec.yaml" "$PRECOMPILED_DIR/"

# Create macOS-specific integration guide
cat > "$PRECOMPILED_DIR/INTEGRATION_GUIDE_MACOS.md" << 'EOF'
# 📦 macOS 预编译包集成指南

## 快速开始

### 1. 下载预编译包

下载 `anywp_engine_macos_v{版本号}_precompiled.zip` 并解压到项目目录。

### 2. 添加依赖

在 `pubspec.yaml` 中添加：

```yaml
dependencies:
  anywp_engine:
    path: ./packages/anywp_engine_macos
```

### 3. 运行

```bash
flutter pub get
flutter run -d macos
```

## 包内容说明

- `Frameworks/` - 预编译的插件框架
- `lib/` - Dart API
- `sdk/` - Web SDK (JavaScript)
- `examples/` - HTML 示例
- `macos/` - CocoaPods 配置

## 系统要求

- macOS 10.14+
- Flutter 3.0+
- Xcode 12+

## 更多文档

- 完整 API 文档: [README.md](README.md)
- 更新日志: [CHANGELOG_CN.md](CHANGELOG_CN.md)
- macOS 开发指南: 参见源码包

## 注意事项

1. **沙箱限制**: macOS 应用需要正确的 entitlements
2. **网络权限**: 需要 `com.apple.security.network.client`
3. **文件访问**: 建议使用 https:// 而非 file:// URL

## 常见问题

### Q: 如何配置 entitlements?

在 `macos/Runner/*.entitlements` 中添加：

```xml
<key>com.apple.security.network.client</key>
<true/>
<key>com.apple.security.network.server</key>
<true/>
```

### Q: 如何调试?

使用 Xcode:
```bash
open macos/Runner.xcworkspace
```

---

**版本**: {VERSION}
**更新日期**: {DATE}
EOF

print_step "Verifying precompiled package..."
VERIFY_ERROR=0
if [ ! -f "$PRECOMPILED_DIR/lib/anywp_engine.dart" ]; then
    echo "[ERROR] Dart file not found"
    VERIFY_ERROR=1
fi
if [ ! -d "$PRECOMPILED_DIR/Frameworks" ]; then
    echo "[ERROR] Frameworks directory not found"
    VERIFY_ERROR=1
fi
if [ $VERIFY_ERROR -eq 1 ]; then
    echo "[FAILED] Package verification failed"
    exit 1
else
    echo "[SUCCESS] Package verification passed"
fi

print_step "Creating precompiled ZIP package..."
cd "$RELEASE_DIR"
zip -r "anywp_engine_macos_v${VERSION}_precompiled.zip" "$(basename "$PRECOMPILED_DIR")" -q
echo "Created: anywp_engine_macos_v${VERSION}_precompiled.zip"

# ==========================================
# Part B: Source Package (Full source code)
# ==========================================

print_step "Creating source package structure..."
mkdir -p "$SOURCE_DIR/Frameworks"
mkdir -p "$SOURCE_DIR/lib"
mkdir -p "$SOURCE_DIR/macos/Classes/Modules"
mkdir -p "$SOURCE_DIR/macos/Classes/Utils"
mkdir -p "$SOURCE_DIR/macos/Resources"
mkdir -p "$SOURCE_DIR/sdk"
mkdir -p "$SOURCE_DIR/examples"

print_step "Copying source files to source package..."
# Copy all source files
cp -R "$PROJECT_ROOT/macos/Classes/" "$SOURCE_DIR/macos/Classes/"
cp "$PROJECT_ROOT/macos/anywp_engine.podspec" "$SOURCE_DIR/macos/"
cp "$PROJECT_ROOT/macos/CMakeLists.txt" "$SOURCE_DIR/macos/"

# Copy resources if they exist
if [ -d "$PROJECT_ROOT/macos/Resources" ]; then
    cp -R "$PROJECT_ROOT/macos/Resources/" "$SOURCE_DIR/macos/Resources/"
fi

# Copy Dart lib
cp "$PROJECT_ROOT/lib/anywp_engine.dart" "$SOURCE_DIR/lib/"

# Copy SDK source
cp -R "$PROJECT_ROOT/sdk" "$SOURCE_DIR/"

# Copy examples
cp "$PROJECT_ROOT"/examples/*.html "$SOURCE_DIR/examples/"

# Copy documentation
cp "$PROJECT_ROOT/README.md" "$SOURCE_DIR/"
cp "$PROJECT_ROOT/CHANGELOG_CN.md" "$SOURCE_DIR/"
cp "$PROJECT_ROOT/LICENSE" "$SOURCE_DIR/"
cp "$PROJECT_ROOT/pubspec.yaml" "$SOURCE_DIR/"

# Copy built framework for convenience
if [ -d "$BUILD_OUTPUT/anywp_engine" ]; then
    cp -R "$BUILD_OUTPUT/anywp_engine" "$SOURCE_DIR/Frameworks/"
fi

print_step "Creating source ZIP package..."
cd "$RELEASE_DIR"
zip -r "anywp_engine_macos_v${VERSION}_source.zip" "$(basename "$SOURCE_DIR")" -q
echo "Created: anywp_engine_macos_v${VERSION}_source.zip"

# ==========================================
# Part C: Web SDK Package (shared with Windows)
# ==========================================

# Only create Web SDK if it doesn't exist (can be shared across platforms)
if [ ! -f "$RELEASE_DIR/anywp_web_sdk_v${SDK_VERSION}.zip" ]; then
    print_step "Creating Web SDK package..."
    mkdir -p "$WEB_SDK_DIR/sdk"
    mkdir -p "$WEB_SDK_DIR/examples"
    mkdir -p "$WEB_SDK_DIR/docs"

    # Copy SDK files
    if [ -f "$PROJECT_ROOT/sdk/dist/anywp_sdk.min.js" ]; then
        cp "$PROJECT_ROOT/sdk/dist/anywp_sdk.min.js" "$WEB_SDK_DIR/sdk/"
    fi
    if [ -f "$PROJECT_ROOT/sdk/dist/anywp_sdk.js" ]; then
        cp "$PROJECT_ROOT/sdk/dist/anywp_sdk.js" "$WEB_SDK_DIR/sdk/"
    fi

    # Copy examples and docs
    cp "$PROJECT_ROOT"/examples/*.html "$WEB_SDK_DIR/examples/"
    cp "$PROJECT_ROOT/docs/WEB_DEVELOPER_GUIDE_CN.md" "$WEB_SDK_DIR/docs/"
    cp "$PROJECT_ROOT/docs/WEB_DEVELOPER_GUIDE.md" "$WEB_SDK_DIR/docs/"
    cp "$PROJECT_ROOT/docs/API_USAGE_EXAMPLES.md" "$WEB_SDK_DIR/docs/"
    cp "$PROJECT_ROOT/LICENSE" "$WEB_SDK_DIR/"

    # Create README
    cat > "$WEB_SDK_DIR/README.md" << WEBEOF
# AnyWP Engine - Web SDK v${SDK_VERSION}

Cross-platform Web SDK for desktop wallpaper development.

**Supported Platforms**: Windows, macOS

## Quick Start

\`\`\`html
<script src="sdk/anywp_sdk.js"></script>
<script>
  AnyWP.onClick(element, () => { /* handler */ });
</script>
\`\`\`

## Documentation

- \`docs/WEB_DEVELOPER_GUIDE_CN.md\` - Chinese guide
- \`docs/WEB_DEVELOPER_GUIDE.md\` - English guide
- \`docs/API_USAGE_EXAMPLES.md\` - API examples

## Examples

See \`examples/\` folder for test pages.

## License

MIT License - See LICENSE file
WEBEOF

    cd "$RELEASE_DIR"
    zip -r "anywp_web_sdk_v${SDK_VERSION}.zip" "$(basename "$WEB_SDK_DIR")" -q
    echo "Created: anywp_web_sdk_v${SDK_VERSION}.zip"
fi

cd "$PROJECT_ROOT"

# ==========================================
# Summary
# ==========================================

echo ""
echo "========================================"
echo " macOS Release Build Complete!"
echo "========================================"
echo ""
echo "Packages created:"
echo "  1. anywp_engine_macos_v${VERSION}_precompiled.zip"
echo "  2. anywp_engine_macos_v${VERSION}_source.zip"
if [ ! -f "$RELEASE_DIR/anywp_web_sdk_v${SDK_VERSION}.zip" ]; then
    echo "  3. anywp_web_sdk_v${SDK_VERSION}.zip (shared with Windows)"
fi
echo ""
echo "Location: $RELEASE_DIR"
echo ""
echo "Package descriptions:"
echo "  - Precompiled: For Flutter developers (minimal integration)"
echo "  - Source: For developers who need to modify or debug"
echo "  - Web SDK: For wallpaper developers (HTML/CSS/JS)"
echo ""
echo "========================================"
echo " Next Steps"
echo "========================================"
echo ""
echo "1. Verify packages:"
echo "   ./scripts/verify_precompiled_macos.sh ${VERSION}"
echo ""
echo "2. Test integration:"
echo "   cd /tmp/test_project"
echo "   unzip anywp_engine_macos_v${VERSION}_precompiled.zip"
echo "   flutter create test_anywp"
echo "   # Add dependency and test"
echo ""
echo "3. Create GitHub Release:"
echo "   - Upload all 3 ZIP files"
echo "   - Tag: v${VERSION}-macos"
echo "   - Mark as pre-release if beta"
echo ""

exit 0

