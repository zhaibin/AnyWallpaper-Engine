# 版本管理策略

本文档说明 AnyWP Engine 的版本号管理策略。

---

## 📋 版本号来源

### 单一真实来源 (Single Source of Truth)

**`pubspec.yaml`** 是版本号的唯一真实来源。所有其他位置的版本号都从这里同步。

```yaml
# pubspec.yaml
version: 2.6.2
```

---

## 🔄 版本号同步机制

### 1. podspec 版本同步

**脚本**: `scripts/sync_version.sh`

从 `pubspec.yaml` 读取版本号并自动更新到 `macos/anywp_engine.podspec`。

**使用方法**:
```bash
bash scripts/sync_version.sh
```

**输出示例**:
```
Syncing version: 2.6.2
  Source: pubspec.yaml
  Target: macos/anywp_engine.podspec
✅ Version synced successfully: 2.6.2
```

### 2. 原生代码版本读取

**文件**: `macos/Classes/AnyWPEnginePlugin.m`

从 Bundle 的 Info.plist 动态读取版本号，而非硬编码。

```objc
- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
    if ([method isEqualToString:@"getVersion"]) {
        // Read version from Info.plist dynamically
        NSBundle *bundle = [NSBundle bundleForClass:[self class]];
        NSString *version = [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
        if (!version) {
            version = @"Unknown";
        }
        result(version);
    }
}
```

### 3. 发布脚本集成

**文件**: `scripts/release_macos.sh`

发布脚本会自动调用 `sync_version.sh` 确保版本一致：

```bash
print_step "Syncing version numbers..."
bash "$PROJECT_ROOT/scripts/sync_version.sh"
```

---

## 📝 更新版本号流程

### Step 1: 更新 pubspec.yaml

```yaml
# pubspec.yaml
name: anywp_engine
version: 2.6.3  # 更新这里
```

### Step 2: 同步版本号

```bash
# 方式 1: 手动同步
bash scripts/sync_version.sh

# 方式 2: 发布脚本会自动同步
bash scripts/release_macos.sh
```

### Step 3: 验证同步结果

```bash
# 检查 podspec
grep "s.version" macos/anywp_engine.podspec
# 输出: s.version = '2.6.3'

# 检查 pubspec
grep "^version:" pubspec.yaml
# 输出: version: 2.6.3
```

---

## 🎯 版本号位置总览

| 位置 | 类型 | 更新方式 |
|------|------|----------|
| `pubspec.yaml` | 主版本号 | ✍️ 手动更新 |
| `macos/anywp_engine.podspec` | CocoaPods | 🔄 自动同步 |
| `Info.plist` | Bundle | 🔄 Flutter 自动生成 |
| `getVersion()` | 运行时 | 📖 动态读取 Info.plist |
| `.cursorrules` | 文档记录 | ✍️ 手动更新（可选）|

---

## 🔍 版本号验证

### 验证所有版本号一致

```bash
# 1. 检查 pubspec
PUBSPEC_VERSION=$(grep "^version:" pubspec.yaml | sed 's/version: //' | tr -d ' ')
echo "pubspec.yaml: $PUBSPEC_VERSION"

# 2. 检查 podspec
PODSPEC_VERSION=$(grep "s.version" macos/anywp_engine.podspec | sed "s/.*'\([^']*\)'.*/\1/")
echo "podspec: $PODSPEC_VERSION"

# 3. 检查是否一致
if [ "$PUBSPEC_VERSION" = "$PODSPEC_VERSION" ]; then
    echo "✅ Versions are consistent"
else
    echo "❌ Version mismatch!"
fi
```

### 运行时验证

```dart
// 在 Flutter 应用中
final version = await AnyWPEngine.getVersion();
print('Runtime version: $version');
// 应该输出: Runtime version: 2.6.2
```

---

## ⚠️ 注意事项

### 1. SDK 版本独立

JavaScript SDK 有独立的版本号，在 `sdk/src/package.json` 中管理：

```json
{
  "version": "2.5.0"
}
```

通过 `getSDKVersion()` 获取：

```dart
final sdkVersion = await AnyWPEngine.getSDKVersion();
print('SDK version: $sdkVersion');
// 输出: SDK version: 2.5.0
```

### 2. 不要硬编码版本号

❌ **错误示例**:
```objc
result(@"2.6.2"); // 硬编码
```

✅ **正确示例**:
```objc
NSBundle *bundle = [NSBundle bundleForClass:[self class]];
NSString *version = [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
result(version); // 动态读取
```

### 3. 更新 CHANGELOG

每次更新版本号后，记得更新 `CHANGELOG_CN.md`：

```markdown
## [2.6.3] - 2025-12-08

### 新增功能
- ...
```

---

## 🛠️ 故障排查

### 问题: 版本号不一致

**症状**: `getVersion()` 返回的版本与 `pubspec.yaml` 不一致

**解决方案**:
```bash
# 1. 同步版本号
bash scripts/sync_version.sh

# 2. 清理并重新构建
flutter clean
flutter pub get
flutter build macos
```

### 问题: 运行时返回 "Unknown"

**症状**: `getVersion()` 返回 "Unknown"

**原因**: Info.plist 中没有 `CFBundleShortVersionString` 字段

**解决方案**:
确保 Flutter 正确生成了 Info.plist。检查：
```bash
cat example/build/macos/Build/Products/Release/anywp_engine/Info.plist
```

---

## 📚 相关文档

- [发布流程](../scripts/release_macos.sh)
- [故障排查](./MACOS_TROUBLESHOOTING.md)
- [开发者指南](./FOR_FLUTTER_DEVELOPERS.md)

---

**最后更新**: 2025-12-08  
**版本**: v2.6.2
