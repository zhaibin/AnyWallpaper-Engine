# 🚀 AnyWP Engine 发布指南

本指南说明如何构建、打包和发布 AnyWP Engine 的预编译版本到 GitHub Releases。

---

## 📋 发布前检查清单

### 1. 代码准备

- [ ] 所有功能已完成并测试
- [ ] 代码通过 `flutter analyze` 检查
- [ ] 示例应用正常运行
- [ ] 测试 HTML 文件全部通过

### 2. 文档准备

- [ ] 更新 `CHANGELOG_CN.md` 记录新功能
- [ ] 更新 `pubspec.yaml` 中的版本号
- [ ] 更新 `README.md` 中的版本号（如有必要）
- [ ] 检查所有文档链接是否有效

### 3. 版本号

确保以下文件中的版本号一致：
- `pubspec.yaml` → `version: 1.1.0`
- `CHANGELOG_CN.md` → 添加新版本条目
- `scripts/build_release.bat` → `set "VERSION=1.1.0"`

---

## 🔨 构建预编译包

### 步骤 1：运行构建脚本

在项目根目录打开 PowerShell 或 CMD：

```bash
.\scripts\build_release.bat
```

**脚本会自动完成以下操作：**

1. ✅ 清理旧的构建
2. ✅ 运行 `flutter clean` 和 `flutter pub get`
3. ✅ 构建 Release 版本
4. ✅ 创建 Release 目录结构
5. ✅ 复制 DLL 和相关文件
6. ✅ 生成集成文档和 pubspec.yaml
7. ✅ 打包成 ZIP 文件

### 步骤 2：验证构建产物

构建完成后，检查以下文件：

```
release/
└── anywp_engine_v1.1.0/
    ├── bin/
    │   ├── anywp_engine_plugin.dll     ← 核心插件
    │   └── WebView2Loader.dll          ← WebView2 运行时
    ├── lib/
    │   ├── anywp_engine_plugin.lib     ← 静态库
    │   └── dart/                        ← Dart 源码
    ├── include/
    │   ├── anywp_engine_plugin.h       ← C++ 头文件
    │   └── anywp_engine_plugin_c_api.h
    ├── sdk/
    │   └── anywp_sdk.js                 ← JavaScript SDK
    ├── pubspec.yaml                     ← Flutter 包配置
    ├── PRECOMPILED_README.md            ← 快速开始指南
    ├── README.md                        ← 完整文档
    ├── LICENSE                          ← 许可证
    └── CHANGELOG_CN.md                  ← 更新日志
```

和打包的 ZIP：
```
release/
└── anywp_engine_v1.1.0.zip             ← 上传到 GitHub
```

### 步骤 3：本地测试

在本地测试预编译包是否可用：

```bash
# 创建测试项目
flutter create test_precompiled
cd test_precompiled

# 复制预编译包
xcopy ..\release\anywp_engine_v1.1.0 .\anywp_engine_v1.1.0 /E /I

# 更新 pubspec.yaml
# dependencies:
#   anywp_engine:
#     path: ./anywp_engine_v1.1.0

# 测试构建
flutter pub get
flutter build windows

# 运行测试
flutter run -d windows
```

如果构建和运行成功，说明预编译包可用。

---

## 📤 发布到 GitHub Releases

### 步骤 1：创建 Git Tag

```bash
# 确保所有改动已提交
git add .
git commit -m "chore: release v1.1.0"

# 创建标签
git tag v1.1.0

# 推送代码和标签
git push origin main
git push origin v1.1.0
```

### 步骤 2：创建 GitHub Release

1. **访问 Releases 页面**  
   https://github.com/zhaibin/AnyWallpaper-Engine/releases/new

2. **选择标签**  
   选择刚才创建的 `v1.1.0` 标签

3. **填写 Release 信息**

   **标题**：
   ```
   AnyWP Engine v1.1.0 - 预编译版本 + 拖拽支持与状态持久化
   ```

   **描述**：  
   复制 `docs/RELEASE_TEMPLATE.md` 的内容并根据实际情况修改

4. **上传文件**

   拖拽以下文件到 Assets 区域：
   ```
   release/anywp_engine_v1.1.0.zip
   ```

5. **发布**

   - ☑️ Set as the latest release
   - ☑️ Create a discussion for this release (可选)
   - 点击 **Publish release**

### 步骤 3：验证发布

1. 访问 Release 页面，确认 ZIP 文件可以下载
2. 下载并解压，验证文件完整性
3. 测试从 Release 下载的包是否可用

---

## 📝 Release 说明模板

参考 `docs/RELEASE_TEMPLATE.md`，包含以下关键信息：

### 必须包含

- ✅ 版本号和发布日期
- ✅ 主要功能更新
- ✅ Bug 修复
- ✅ 下载链接和安装指南
- ✅ 系统要求
- ✅ 完整文档链接

### 可选包含

- 📊 性能对比
- 🎓 使用示例
- 🐛 已知问题
- 🔄 升级指南
- 🙏 致谢

---

## 🔄 发布后工作

### 1. 更新文档

确保所有文档链接指向正确的版本：
- README.md
- QUICK_START.md
- PRECOMPILED_DLL_INTEGRATION.md

### 2. 社区通知

- 在 GitHub Discussions 发布公告
- 更新相关社区（如 Reddit、Discord）
- 发布博客文章（如有）

### 3. 监控反馈

- 关注 GitHub Issues
- 回应用户问题
- 收集功能建议

---

## 🐛 发布问题排查

### Q: build_release.bat 失败？

**检查步骤**：
```bash
# 1. 确认 Flutter 环境
flutter doctor

# 2. 确认 WebView2 SDK 已安装
dir windows\packages\Microsoft.Web.WebView2.1.0.2592.51

# 3. 手动构建测试
cd example
flutter clean
flutter pub get
flutter build windows --release
```

### Q: ZIP 文件缺少某些文件？

编辑 `scripts/build_release.bat`，检查复制命令：
```bat
copy "%BUILD_DIR%\..." "%RELEASE_DIR%\..." >nul
```

### Q: 用户报告 DLL 无法加载？

**可能原因**：
1. WebView2 Runtime 未安装（Windows 10）
2. 缺少依赖的 DLL（如 `WebView2Loader.dll`）
3. 架构不匹配（x86 vs x64）

**解决方案**：
- 确保 Release 包含 `WebView2Loader.dll`
- 在 Release 说明中明确系统要求
- 提供 WebView2 Runtime 下载链接

---

## 📊 版本号规范

遵循 [语义化版本](https://semver.org/lang/zh-CN/)：

```
MAJOR.MINOR.PATCH
```

**示例**：
- `1.0.0` → `1.0.1`（修复 Bug）
- `1.0.1` → `1.1.0`（新功能，向后兼容）
- `1.1.0` → `2.0.0`（破坏性更改）

---

## 🔐 安全性检查

发布前确保：

- [ ] 不包含敏感信息（密钥、密码）
- [ ] 不包含测试代码或调试符号
- [ ] 所有依赖来自可信源
- [ ] License 文件完整

---

## 📞 获取帮助

如果在发布过程中遇到问题：

1. 查看 [GitHub Actions 日志](https://github.com/zhaibin/AnyWallpaper-Engine/actions)
2. 提交 [Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
3. 联系维护者

---

**最后更新**：2025-11-05  
**版本**：1.0.0

