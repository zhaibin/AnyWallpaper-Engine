# AnyWP Engine - 发版流程快速参考

> **版本管理策略 (v2.4.1+)**: 双版本号独立管理

## 版本号体系

本项目包含两个独立产品，各有独立版本号：

| 产品 | 版本文件 | 用途 |
|------|---------|------|
| **Flutter Plugin (引擎)** | `pubspec.yaml` | Windows 桌面壁纸引擎 |
| **Web SDK (JavaScript)** | `windows/sdk/package.json` | HTML 壁纸开发 SDK |

**重要**: 两个版本号可以不同，根据各自的更新内容独立递增。

## 发版流程（5步）

### Step 1: 更新版本号

根据更新内容，选择需要更新的版本号：

#### 场景 A：仅更新引擎功能（C++/Dart）
```bash
# 1. 更新引擎版本号
编辑 pubspec.yaml: version: 2.x.x

# 2. 更新文档
编辑 CHANGELOG_CN.md: 添加 [2.x.x] 条目（标注 Engine）
编辑 .cursorrules: 更新底部版本号
编辑 docs/PRECOMPILED_DLL_INTEGRATION.md: 更新示例版本号
```

#### 场景 B：仅更新 SDK 功能（TypeScript/JS）
```bash
# 1. 更新 SDK 版本号
编辑 windows/sdk/package.json: version: 2.x.x

# 2. 更新文档
编辑 CHANGELOG_CN.md: 添加 [2.x.x] 条目（标注 Web SDK）
```

#### 场景 C：同时更新引擎和 SDK
```bash
# 1. 分别更新两个版本号
编辑 pubspec.yaml: version: 2.x.x
编辑 windows/sdk/package.json: version: 2.x.x

# 2. 更新文档
编辑 CHANGELOG_CN.md: 添加条目，区分 Engine 和 SDK 更新
编辑 .cursorrules: 更新底部版本号
编辑 docs/PRECOMPILED_DLL_INTEGRATION.md: 更新示例版本号
```

### Step 2: 检查文档更新

确保以下文档已更新：

**Flutter 开发者文档**（引擎更新时）:
- [ ] `lib/anywp_engine.dart` - API 文档注释
- [ ] `docs/FOR_FLUTTER_DEVELOPERS.md`
- [ ] `docs/DEVELOPER_API_REFERENCE.md`
- [ ] `docs/PRECOMPILED_DLL_INTEGRATION.md`

**Web 开发者文档**（SDK 更新时）:
- [ ] `docs/WEB_DEVELOPER_GUIDE_CN.md`
- [ ] `docs/WEB_DEVELOPER_GUIDE.md`
- [ ] `docs/API_USAGE_EXAMPLES.md`

**通用文档**:
- [ ] `README.md`
- [ ] `CHANGELOG_CN.md`

### Step 3: 发版前检查

```bash
.\scripts\pre_release_check.bat
```

**检查内容**（13项）:
- 引擎版本一致性
- SDK 版本验证（独立版本）
- **文档一致性检查** ⭐ 新增
- Lint、测试、构建检查

### Step 4: 构建发布包

```bash
.\scripts\release.bat
```

**输出文件**（注意版本号）:
- `anywp_engine_v{引擎版本}_precompiled.zip`
- `anywp_engine_v{引擎版本}_source.zip`
- `anywp_web_sdk_v{SDK版本}.zip` ⭐ 使用 SDK 版本

### Step 5: 发布到 GitHub

#### 方式 1：自动化脚本（推荐）
```bash
.\scripts\release_git.bat {引擎版本号}
```

#### 方式 2：手动发布
```bash
# 1. 提交代码
git add release/ CHANGELOG_CN.md pubspec.yaml docs/ README.md .cursorrules
git commit -F release\commit_msg_v{引擎版本号}.txt
git push origin main

# 2. 创建 Tag
git tag -a v{引擎版本号} -m "AnyWP Engine v{引擎版本号}"
git push origin v{引擎版本号}

# 3. 创建 GitHub Release
# 访问: https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
# Tag: v{引擎版本号}
# Title: AnyWP Engine v{引擎版本号}
# Description: 在顶部添加版本信息
```

**Release Notes 模板**:
```markdown
**Flutter Plugin Version**: v{引擎版本号}
**Web SDK Version**: v{SDK版本号}

> **Note**: Flutter Plugin and Web SDK have independent version numbers.

[复制 release/GITHUB_RELEASE_NOTES_v{引擎版本号}.md 的内容]
```

**上传文件**:
- `anywp_engine_v{引擎版本号}_precompiled.zip` - Flutter 开发者
- `anywp_engine_v{引擎版本号}_source.zip` - 高级用户
- `anywp_web_sdk_v{SDK版本号}.zip` - Web 壁纸开发者

## CHANGELOG 格式示例

### 示例 1：仅引擎更新
```markdown
## [2.5.0] - 2025-11-20

### 新增功能
- 添加自动恢复机制
- 优化 WorkerW 创建流程

### 修复
- 修复 Explorer 重启后壁纸丢失问题
```

### 示例 2：仅 SDK 更新
```markdown
## [2.4.2] - 2025-11-20 (Web SDK)

### SDK 更新
- 新增 `setVolume()` API
- 改进错误处理机制
- 修复拖拽事件冲突

**注意**: 此版本仅更新 Web SDK，引擎版本保持 v2.4.1
```

### 示例 3：同时更新
```markdown
## [2.5.0] - 2025-11-20

### 引擎更新 (Engine v2.5.0)
- 添加音频控制支持
- 新增 `setVolume` Dart API

### SDK 更新 (SDK v2.5.0)
- 新增 `AnyWP.setVolume()` JavaScript API
- 与引擎音频控制配合使用

**版本信息**: Engine v2.5.0 + SDK v2.5.0
```

## 常见问题

### Q1: 什么时候需要更新引擎版本？
**A**: 当修改了 C++、Dart 代码，或者更改了插件的核心功能时。

### Q2: 什么时候需要更新 SDK 版本？
**A**: 当修改了 TypeScript/JavaScript SDK 代码，添加、修改或删除了 Web API 时。

### Q3: 可以只更新其中一个版本吗？
**A**: 可以！两个版本号是独立的。如果只改了引擎，就只更新 `pubspec.yaml`；如果只改了 SDK，就只更新 `windows/sdk/package.json`。

### Q4: GitHub Release 的 Tag 用哪个版本号？
**A**: 使用引擎版本号（`pubspec.yaml`），因为 Release 主要针对 Flutter 开发者。在 Release Notes 中明确标注 SDK 版本。

### Q5: 发版前检查失败怎么办？
**A**: 仔细阅读错误信息，修复所有错误后重新运行 `pre_release_check.bat`。脚本会阻止发版直到所有检查通过。

## 脚本参考

| 脚本 | 用途 |
|------|------|
| `pre_release_check.bat` | 发版前全面检查（13项） |
| `release.bat` | 构建发布包（3个 ZIP） |
| `release_git.bat {版本号}` | 自动 Git 提交和推送 |
| `verify_precompiled.bat {版本号}` | 验证发布包完整性 |
| `check_version_consistency.ps1` | 版本一致性检查 |
| `check_docs_consistency.ps1` | 文档一致性检查 ⭐ 新增 |

## 更新日志

- **v2.4.1** (2025-11-20): 引入双版本号独立管理，添加文档一致性检查
- **v2.3.0**: 版本号统一管理，CMake 自动生成
- **v2.2.0**: 引入自动化发版脚本

---

**完整规范**: 参见 `.cursorrules` 文件

