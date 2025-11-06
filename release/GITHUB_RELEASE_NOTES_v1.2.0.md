# AnyWP Engine v1.2.0 - 应用级存储隔离 + 测试UI优化

**发布日期**: 2025-11-06  
**版本**: 1.2.0 (Changelog: 4.4.0)

---

## 🎉 发布亮点

### 1. 🗂️ 应用级存储隔离机制
**彻底解决多应用数据残留问题**

- ✅ 每个应用使用独立存储目录
- ✅ 卸载干净无残留
- ✅ 多应用完全隔离，互不干扰
- ✅ 向后兼容

```dart
// 设置应用唯一标识
await AnyWPEngine.setApplicationName('MyAwesomeApp');

// 获取存储路径
final path = await AnyWPEngine.getStoragePath();
// 输出: C:\Users\...\AppData\Local\AnyWPEngine\MyAwesomeApp
```

### 2. 🎨 测试界面大升级
**效率提升 12 倍**

- ✅ 8 个快捷测试按钮，一键加载测试页面
- ✅ 表情图标标识，直观识别
- ✅ 自动换行布局，响应式设计
- ✅ 保留自定义 URL 输入框

**快捷测试页面**:
- 🎨 Simple - 基础壁纸测试
- 🖱️ Draggable - 拖拽演示（鼠标钩子）
- ⚙️ API Test - 完整 API 测试
- 👆 Click Test - 点击检测测试
- 👁️ Visibility - 可见性/省电测试
- ⚛️ React / 💚 Vue - SPA 框架测试
- 📺 iFrame Ads - 广告检测测试

---

## ✨ 新增功能

### 应用级存储隔离

**问题**: 旧版本所有应用共享同一存储目录，卸载单个应用无法清理其专属数据

**解决方案**: 每个应用使用独立子目录

**存储路径**:
```
%LOCALAPPDATA%\AnyWPEngine\
├── AppA\
│   └── state.json    # 应用 A 的数据
├── AppB\
│   └── state.json    # 应用 B 的数据
└── Default\
    └── state.json    # 未设置应用名的默认数据
```

### 新增 API

#### `setApplicationName(String name)`
设置应用唯一标识以隔离存储。应在壁纸初始化前调用。

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 设置应用标识
  await AnyWPEngine.setApplicationName('MyCompany_MyApp');
  
  runApp(MyApp());
}
```

#### `getStoragePath()`
获取当前应用的存储路径。

```dart
final path = await AnyWPEngine.getStoragePath();
print('数据存储在: $path');
```

### 卸载清理

**手动清理**:
```powershell
Remove-Item -Recurse "$env:LOCALAPPDATA\AnyWPEngine\MyApp"
```

**集成到卸载程序** (Windows Installer):
```bat
REM uninstall.bat
rmdir /s /q "%LOCALAPPDATA%\AnyWPEngine\MyApp"
```

---

## 🔄 重构改进

### 存储系统演进

| 版本 | 存储方式 | 问题 | 状态 |
|------|---------|------|------|
| v1.0 | 注册表 | 卸载残留垃圾 | ❌ |
| v1.1 | JSON 文件 | 多应用共享目录 | ⚠️ |
| **v1.2** | **应用隔离 JSON** | **✅ 完美解决** | ✅ |

### 技术改进

- ✅ 修改 `GetAppDataPath()` 支持应用名称参数
- ✅ 更新状态文件读写函数传递应用名称
- ✅ 添加应用名称清理和验证逻辑
- ✅ 切换应用时自动清空内存缓存

---

## 🎨 UI 改进

### 测试效率对比

| 方式 | 操作步骤 | 耗时 |
|-----|---------|------|
| 优化前 | 查看文件名 → 手动输入完整路径 → 启动 | ~60 秒 |
| **优化后** | **点击快捷按钮 → 启动** | **~5 秒** |

**效率提升**: 🚀 **12倍**

### 界面布局

```
🚀 Quick Test Pages:
[🎨 Simple] [🖱️ Draggable] [⚙️ API Test] [👆 Click Test]
[👁️ Visibility] [⚛️ React] [💚 Vue] [📺 iFrame Ads]

🔗 Custom URL:
[_________________________________________________]

[▶ Start]  [⏹ Stop]
```

---

## 📚 文档更新

### 文档优化

- **README.md** - 整合存储隔离完整指南
  - 配置说明和 API 参考
  - 多应用隔离优势详解
  - 卸载清理最佳实践（批处理、PowerShell、NSIS 等多种方案）
  - 从旧版本迁移说明
- **所有核心文档同步更新** - API 参考、开发者指南等

---

## 📊 测试结果

| 测试项 | 结果 |
|-------|------|
| 功能测试 | ✅ 100% 通过 (17/17) |
| 编译测试 | ✅ 无错误无警告 |
| 运行测试 | ✅ 稳定运行，内存占用合理 |
| 隔离测试 | ✅ 多应用数据完全隔离 |

---

## 🚀 使用示例

### 基本集成

```dart
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 1. 设置应用唯一标识（推荐）
  await AnyWPEngine.setApplicationName('MyAwesomeApp');
  
  // 2. 查看存储路径（可选）
  final storagePath = await AnyWPEngine.getStoragePath();
  print('数据存储在: $storagePath');
  
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: ElevatedButton(
          onPressed: () async {
            // 初始化壁纸
            await AnyWPEngine.initializeWallpaper(
              url: 'https://example.com/wallpaper.html',
              enableMouseTransparent: true,
            );
          },
          child: Text('Start Wallpaper'),
        ),
      ),
    );
  }
}
```

### 卸载清理脚本

```bat
@echo off
REM 集成到你的卸载程序

echo Cleaning up application data...

REM 删除应用专属数据目录
rmdir /s /q "%LOCALAPPDATA%\AnyWPEngine\MyAwesomeApp"

echo Application data cleaned successfully!
```

---

## 💡 升级建议

### 从 v1.1.x 升级

**代码更改**（可选，推荐）:
```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 新增：设置应用名称以启用存储隔离
  await AnyWPEngine.setApplicationName('YourAppName');
  
  runApp(MyApp());
}
```

**向后兼容**: 如果不调用 `setApplicationName()`，将使用默认值 `"Default"`，行为与 v1.1.x 相同。

---

## 🔗 相关资源

- 📖 [完整更新日志](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md)
- 📚 [存储隔离指南](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/STORAGE_ISOLATION.md)
- 🚀 [快速开始](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/QUICK_START.md)
- 💻 [API 参考](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/DEVELOPER_API_REFERENCE.md)

---

## 📦 安装方式

### 方式 1: pub.dev (推荐)
```yaml
dependencies:
  anywp_engine: ^1.2.0
```

### 方式 2: 预编译 DLL
1. 下载 `anywp_engine_v1.2.0.zip`
2. 解压到项目目录
3. 参考 `PRECOMPILED_README.md`

### 方式 3: Git 仓库
```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/zhaibin/AnyWallpaper-Engine.git
      ref: v1.2.0
```

---

## 🙏 致谢

感谢所有使用 AnyWP Engine 的开发者！

如果您觉得这个项目有用，请给我们一个 ⭐ Star！

---

## 📝 完整更新日志

查看 [CHANGELOG_CN.md](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md) 了解所有版本的详细变更。

---

**作者**: AnyWP Engine Team  
**许可**: MIT License  
**支持**: [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues)

