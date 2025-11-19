# 预编译包自动化测试指南

## 📋 概述

为了防止发布包含缺陷的预编译包（如v2.4.1初次发布时DLL缺少Auto Recovery功能的问题），我们开发了一套三层自动化测试系统。

## 🎯 测试层级

### 第1层: 基础完整性测试 (快速, <5秒)

验证包的基本结构和文件完整性：

- ✅ 包目录存在
- ✅ 所有必需文件存在（DLL、LIB、Dart、头文件、文档等）
- ✅ DLL文件大小合理（500KB - 5MB）
- ✅ 版本号一致性（pubspec.yaml）
- ✅ DLL编译时间合理（不是旧版本）

**适用场景**: 快速验证打包是否成功

### 第2层: DLL功能验证 (中速, <30秒)

验证DLL和API的可用性：

- ✅ DLL导出符号检查（需要Visual Studio）
- ✅ CHANGELOG包含当前版本说明
- ✅ Dart API完整性检查（核心API存在）

**适用场景**: 发布前的快速验证

### 第3层: 完整功能测试 (完整, <2分钟)

运行集成测试验证实际功能：

- ✅ 所有API调用测试
- ✅ 日志输出验证
- ✅ 实际功能验证（壁纸创建、Auto Recovery等）

**适用场景**: 重要版本发布前的全面验证

## 🚀 使用方法

### 基础测试（推荐日常使用）

```powershell
.\scripts\test_precompiled_package.ps1 -Version "2.4.1" -TestLevel Basic
```

**输出示例**:
```
通过: 15 | 失败: 0 | 跳过: 0
✓ 所有测试通过！
```

### 快速测试（推荐发布前使用）

```powershell
.\scripts\test_precompiled_package.ps1 -Version "2.4.1" -TestLevel Quick
```

**输出示例**:
```
通过: 21 | 失败: 0 | 跳过: 1
✓ 所有测试通过！
```

### 完整测试（推荐重要版本发布）

```powershell
.\scripts\test_precompiled_package.ps1 -Version "2.4.1" -TestLevel Full
```

### 自定义包路径

```powershell
.\scripts\test_precompiled_package.ps1 -Version "2.4.1" -PackagePath "E:\custom\path\anywp_engine_v2.4.1_precompiled"
```

## 📦 集成到发布流程

在 `release.bat` 脚本的末尾添加自动测试：

```batch
echo [Step N] Testing precompiled package...
powershell -ExecutionPolicy Bypass -File scripts\test_precompiled_package.ps1 -Version %PLUGIN_VERSION% -TestLevel Quick
if errorlevel 1 (
    echo [ERROR] Package testing failed!
    exit /b 1
)
```

## 🔍 测试检查清单

### v2.4.1 测试项

| 测试项 | 类型 | 说明 |
|--------|------|------|
| 包目录存在 | 基础 | 验证预编译包目录 |
| DLL文件 | 基础 | `bin\anywp_engine_plugin.dll` + `WebView2Loader.dll` |
| LIB文件 | 基础 | `lib\anywp_engine_plugin.lib` |
| Dart文件 | 基础 | `lib\anywp_engine.dart` + `lib\dart\anywp_engine.dart` |
| C API头文件 | 基础 | `include\anywp_engine\anywp_engine_plugin_c_api.h` |
| CMakeLists | 基础 | `windows\CMakeLists.txt` |
| 文档 | 基础 | `README.md`, `CHANGELOG_CN.md`, `LICENSE` |
| DLL大小 | 基础 | 500KB - 5MB（v2.4.1约1075KB）|
| 版本一致性 | 基础 | pubspec.yaml版本号匹配 |
| DLL编译时间 | 基础 | 不超过7天（建议24小时内）|
| DLL导出符号 | 功能 | Flutter插件注册函数 |
| CHANGELOG | 功能 | 包含当前版本说明 |
| initializeWallpaperOnMonitor | 功能 | 核心API |
| enableAutoRecovery | 功能 | Auto Recovery功能 |
| isAutoRecoveryEnabled | 功能 | Auto Recovery状态查询 |
| saveCurrentWallpaperConfiguration | 功能 | 手动保存配置 |
| sendMessage | 功能 | WebMessage通信 |

## ⚠️ 常见问题

### 问题1: dumpbin工具未找到

**症状**:
```
[~] dumpbin工具未找到，跳过导出符号检查
[i] （需要安装 Visual Studio 或 Build Tools）
```

**解决方案**:
1. 安装 Visual Studio 2022（推荐）
2. 或安装 Visual Studio Build Tools
3. 测试会自动跳过，不影响其他测试

### 问题2: DLL编译时间过旧

**症状**:
```
[✗] DLL编译时间过旧 (30 天前)，可能是旧版本
```

**原因**: 复制了旧的DLL文件或缓存未清理

**解决方案**:
```powershell
cd example
flutter clean
flutter build windows --release
```

### 问题3: API缺失

**症状**:
```
[✗] API缺失: saveCurrentWallpaperConfiguration
```

**原因**: Dart API文件未同步更新

**解决方案**:
1. 确认 `lib/anywp_engine.dart` 包含该API
2. 重新运行 `release.bat`

## 📊 测试报告

测试完成后会显示详细报告：

```
========================================
 测试总结
========================================
测试级别: Quick
通过: 21
失败: 0
跳过: 1

✓ 所有测试通过！
预编译包可以安全使用
```

## 🔧 高级用法

### 创建集成测试项目

如果需要运行完整的功能测试（第3层），需要先创建集成测试项目：

```powershell
.\scripts\create_integration_test.ps1
```

该脚本会创建一个专门的Flutter测试应用，用于：
- 调用所有AnyWP Engine API
- 验证日志输出
- 测试实际功能（壁纸创建、Auto Recovery等）

### 自定义测试项

修改 `scripts/test_precompiled_package.ps1` 中的 `$requiredAPIs` 数组，添加新的必需API：

```powershell
$requiredAPIs = @(
    "initializeWallpaperOnMonitor",
    "yourNewAPI"  # 添加新API
)
```

## 📝 最佳实践

1. **每次发布前运行Quick测试**: 确保基本功能完整
2. **重要版本发布前运行Full测试**: 全面验证功能
3. **将测试集成到CI/CD**: 自动化质量保证
4. **保存测试日志**: 便于问题追溯

## 🎯 测试覆盖率目标

- **基础完整性**: 100%（所有必需文件）
- **API完整性**: 100%（所有公共API）
- **功能验证**: ≥90%（核心功能）

## 📚 相关文档

- [发布流程](../README.md#发版流程)
- [集成指南](INTEGRATION_GUIDE.md)
- [开发者API参考](DEVELOPER_API_REFERENCE.md)

---

**最后更新**: 2025-11-19  
**测试工具版本**: 1.0.0  
**支持的引擎版本**: 2.4.0+



