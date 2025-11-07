# 自动化发版指南

## 📋 概述

v1.3.1+ 提供完整的自动化发版流程，包括：
- ✅ 自动构建 Flutter Plugin 包（~16 MB）
- ✅ 自动构建 Web SDK 独立包（~56 KB）
- ✅ 自动更新版本号
- ✅ 自动提交代码和创建 Git Tag
- ✅ 自动验证关键文件

---

## 🚀 快速开始

### 方法 1：完全自动化（推荐）⭐

使用 `auto_release.ps1` 脚本执行完整的发版流程：

```powershell
# 在项目根目录执行
.\scripts\auto_release.ps1 -Version "1.3.2" -ReleaseTitle "新功能描述"
```

**脚本会自动完成**：
1. 更新版本号（pubspec.yaml、build_release_v2.bat、.cursorrules）
2. 编译 Release 版本（~30秒）
3. 打包 Flutter Plugin + Web SDK
4. 验证所有关键文件
5. Git 提交并推送
6. 创建并推送 Git Tag

**完成后手动操作**：
1. 访问 GitHub 创建 Release
2. 上传两个 ZIP 文件
3. 复制 Release Notes 内容

---

### 方法 2：手动分步执行

如果需要更多控制，可以分步执行：

#### Step 1: 更新版本号（3个文件）
```yaml
# pubspec.yaml
version: 1.x.x

# scripts/build_release_v2.bat
set "VERSION=1.x.x"

# .cursorrules (底部)
**版本**: 1.x.x
```

#### Step 2: 更新 CHANGELOG_CN.md
添加新版本的更新日志。

#### Step 3: 运行构建脚本
```bash
.\scripts\build_release_v2.bat
```

生成文件：
- `release/anywp_engine_v{版本号}.zip` (Flutter Plugin)
- `release/anywp_web_sdk_v{版本号}.zip` (Web SDK)

#### Step 4: 创建 Release Notes
复制 `docs/RELEASE_TEMPLATE.md` → `release/GITHUB_RELEASE_NOTES_v{版本号}.md`

#### Step 5: Git 提交
```bash
git add .
git commit -m "release: 发布 v1.x.x - 功能描述"
git push origin main
```

#### Step 6: 创建 Git Tag
```bash
git tag -a v1.x.x -m "AnyWP Engine v1.x.x - 功能描述"
git push origin v1.x.x
```

#### Step 7: 创建 GitHub Release
1. 访问: https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
2. 选择 Tag: v1.x.x
3. Title: `AnyWP Engine v1.x.x - 功能描述`
4. Description: 复制 Release Notes 内容
5. **上传两个 ZIP 文件** ⭐
6. 勾选 "Set as the latest release"
7. 点击 "Publish release"

---

## 🔧 auto_release.ps1 参数说明

### 必需参数

| 参数 | 说明 | 示例 |
|------|------|------|
| `-Version` | 版本号 | `"1.3.2"` |
| `-ReleaseTitle` | 发布标题 | `"显示器热插拔优化"` |

### 可选参数

| 参数 | 说明 | 使用场景 |
|------|------|----------|
| `-SkipBuild` | 跳过构建步骤 | 已手动构建，只需提交 |
| `-SkipGitPush` | 不推送到 GitHub | 仅本地测试 |
| `-DryRun` | 模拟运行，不实际执行 | 测试脚本逻辑 |

### 使用示例

```powershell
# 完整发版
.\scripts\auto_release.ps1 -Version "1.3.2" -ReleaseTitle "优化性能"

# 仅测试（不实际执行）
.\scripts\auto_release.ps1 `
    -Version "1.3.2" `
    -ReleaseTitle "测试" `
    -DryRun

# 跳过构建（已手动构建）
.\scripts\auto_release.ps1 `
    -Version "1.3.2" `
    -ReleaseTitle "修复Bug" `
    -SkipBuild

# 不推送到 GitHub（本地验证）
.\scripts\auto_release.ps1 `
    -Version "1.3.2" `
    -ReleaseTitle "实验性功能" `
    -SkipGitPush
```

---

## 📦 发布包说明

### Flutter Plugin 包 (anywp_engine_v{版本号}.zip)

**目标用户**：Flutter 开发者

**内容**：
- `bin/` - 运行时 DLL（anywp_engine_plugin.dll + WebView2Loader.dll）
- `lib/` - Dart 源码 + 链接库（.lib）
- `include/` - C++ 头文件
- `windows/` - C++ 源码 + CMake 配置 + WebView2 NuGet 包
- `sdk/` - JavaScript SDK
- 文档、示例、辅助脚本

**大小**：~16 MB

---

### Web SDK 包 (anywp_web_sdk_v{版本号}.zip)

**目标用户**：Web 前端开发者

**内容**：
- `sdk/anywp_sdk.js` - JavaScript SDK
- `examples/` - 8 个示例 HTML 文件
- `docs/` - 3 份开发指南（中英文 + API 示例）
- `README.md` - 快速开始指南
- `LICENSE`

**大小**：~56 KB

**优势**：
- ✅ 轻量级，无需下载完整插件
- ✅ 专注 Web 开发，无需了解 Flutter
- ✅ 包含完整示例和文档

---

## ✅ 发版检查清单

### 构建前检查
- [ ] 所有功能已开发完成并测试通过
- [ ] CHANGELOG_CN.md 已更新
- [ ] README.md 已更新（如有新功能）
- [ ] 文档已同步更新

### 构建检查
- [ ] Flutter Plugin 包生成成功（~16 MB）
- [ ] Web SDK 包生成成功（~56 KB）
- [ ] 验证关键文件存在：
  - [ ] `bin/anywp_engine_plugin.dll`
  - [ ] `lib/anywp_engine_plugin.lib`
  - [ ] `lib/anywp_engine.dart`
  - [ ] `windows/src/anywp_engine_plugin.cpp`
  - [ ] Web SDK 包内容完整

### Git 检查
- [ ] 代码已提交并推送到 main
- [ ] Git Tag v{版本号} 已创建并推送
- [ ] GitHub 网页可见最新提交和 Tag

### GitHub Release 检查
- [ ] Release 页面创建成功
- [ ] **两个 ZIP 文件都已上传** ⭐
- [ ] Release Notes 内容完整
- [ ] 标记为 "latest release"
- [ ] 文件可正常下载

---

## 🐛 常见问题

### Q1: 构建失败，找不到 DLL 文件

**原因**：Flutter Release 编译失败或被中断

**解决**：
```bash
cd example
flutter clean
flutter build windows --release
```

### Q2: Web SDK 包缺失

**原因**：build_release_v2.bat 步骤 17/17 失败

**解决**：
```powershell
# 单独运行 Web SDK 构建脚本
.\scripts\build_web_sdk.ps1 -Version "1.x.x"
```

### Q3: Git 提交时出现乱码

**原因**：PowerShell 默认编码问题

**解决**：使用 `auto_release.ps1` 脚本，或手动通过文件提交：
```bash
echo "release: 发布 v1.x.x - 功能描述" > commit_msg.txt
git commit -F commit_msg.txt
del commit_msg.txt
```

### Q4: GitHub Release 忘记上传 Web SDK 包

**解决**：
1. 编辑 Release（点击 Release 页面的 "Edit" 按钮）
2. 上传缺失的 `anywp_web_sdk_v{版本号}.zip`
3. 更新 Release 说明，提及两个包
4. 保存更改

---

## 📚 相关文档

- **完整发版流程**: `.cursorrules` 文件（搜索"测试、上线、发版流程"）
- **Web SDK 构建**: `scripts/build_web_sdk.ps1`
- **主构建脚本**: `scripts/build_release_v2.bat`
- **Release 模板**: `docs/RELEASE_TEMPLATE.md`

---

## 🎯 下次发版快速参考

```powershell
# 1. 一键发版
.\scripts\auto_release.ps1 -Version "1.x.x" -ReleaseTitle "功能描述"

# 2. 创建 Release Notes
Copy-Item docs\RELEASE_TEMPLATE.md release\GITHUB_RELEASE_NOTES_v1.x.x.md

# 3. GitHub 创建 Release
# - 访问 https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
# - 上传两个 ZIP 文件
# - 发布

# 完成！🎉
```

