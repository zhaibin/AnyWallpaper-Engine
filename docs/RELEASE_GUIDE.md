# AnyWP Engine - 发版指南

> **⚠️ 官方发版文档 - 发版时必须遵循本文档的流程和要求**
> 
> **版本管理策略 (v2.4.1+)**: 双版本号独立管理  
> **最后更新**: 2025-11-20  
> **维护者**: 开发团队

## 📋 目录

- [版本号体系](#版本号体系)
- [发版流程（7步）](#发版流程)
- [CHANGELOG 格式规范](#changelog-格式规范)
- [Git 提交规范](#git-提交规范)
- [常见问题](#常见问题)
- [脚本参考](#脚本参考)

---

## 版本号体系

### 双产品架构

本项目包含两个独立产品，各有独立版本号：

| 产品 | 版本文件 | 用途 | 示例 |
|------|---------|------|------|
| **Flutter Plugin (引擎)** | `pubspec.yaml` | Windows 桌面壁纸引擎 | `version: 2.4.1` |
| **Web SDK (JavaScript)** | `sdk/src/package.json` | HTML 壁纸开发 SDK | `"version": "2.4.0"` |

### 版本号规则

**重要原则**: 
- ✅ 两个版本号**可以不同**，根据各自的更新内容独立递增
- ✅ 如果只更新引擎功能（C++/Dart），只需更新 `pubspec.yaml`
- ✅ 如果只更新 Web SDK 功能（TypeScript/JS），只需更新 `sdk/src/package.json`
- ✅ 如果两者都更新，需要分别更新两个版本号

### 自动生成的版本

以下版本号由构建系统自动生成，**无需手动修改**：
- ✅ `windows/version.h` - 由 CMake 从 `pubspec.yaml` 自动生成
- ✅ C++ 代码中的 `kPluginVersion` - 从 `version.h` 引用

---

## 发版流程

### Step 1: 版本号更新

根据更新内容，选择对应的更新场景：

#### 场景 A：仅更新引擎功能（C++/Dart）

**版本号更新**:
- [ ] `pubspec.yaml`: `version: x.x.x` ⭐ **引擎版本唯一来源**

**文档更新**:
- [ ] `CHANGELOG_CN.md`: 添加新版本条目（标注 Engine vX.X.X）
- [ ] `.cursorrules`: 更新底部版本号
- [ ] `docs/PRECOMPILED_DLL_INTEGRATION.md`: 更新示例版本号

#### 场景 B：仅更新 SDK 功能（TypeScript/JS）

**版本号更新**:
- [ ] `sdk/src/package.json`: `version: x.x.x` ⭐ **SDK 版本唯一来源**

**文档更新**:
- [ ] `CHANGELOG_CN.md`: 添加新版本条目（标注 SDK vX.X.X）

#### 场景 C：同时更新引擎和 SDK

**版本号更新**:
- [ ] `pubspec.yaml`: `version: x.x.x`
- [ ] `sdk/src/package.json`: `version: x.x.x`

**文档更新**:
- [ ] `CHANGELOG_CN.md`: 添加条目，区分 Engine 和 SDK 更新
- [ ] `.cursorrules`: 更新底部版本号
- [ ] `docs/PRECOMPILED_DLL_INTEGRATION.md`: 更新示例版本号

---

### Step 2: 文档更新（⚠️ 重要：保持一致性）

根据更新类型，确保相关文档已同步更新：

#### Flutter 开发者文档（引擎更新时必须检查）

- [ ] `lib/anywp_engine.dart` - 公共 API 文档注释
- [ ] `docs/FOR_FLUTTER_DEVELOPERS.md` - API 列表和使用说明
- [ ] `docs/DEVELOPER_API_REFERENCE.md` - 完整 API 参考
- [ ] `docs/PRECOMPILED_DLL_INTEGRATION.md` - 集成指南（包含版本号）
- [ ] `README.md` - Features 和 Usage 章节

#### Web 开发者文档（SDK 更新时必须检查）

- [ ] `docs/WEB_DEVELOPER_GUIDE_CN.md` - 中文 Web 开发指南
- [ ] `docs/WEB_DEVELOPER_GUIDE.md` - 英文 Web 开发指南
- [ ] `docs/API_USAGE_EXAMPLES.md` - API 使用示例
- [ ] `sdk/src/` - TypeScript 源码注释

#### 通用文档

- [ ] `README.md` - 更新 Features 和使用示例
- [ ] `CHANGELOG_CN.md` - 添加更新日志（区分 Engine 和 SDK 版本）

#### 文档一致性要求

- ✅ API 文档必须与代码实现保持同步
- ✅ 集成文档中的版本号必须正确
- ✅ 中英文文档内容应保持一致
- ✅ 示例代码必须可运行

---

### Step 3: 发版前全面检查（⚠️ 必须执行）

```bash
.\scripts\pre_release_check.bat
```

#### 检查项目（13项全面检查）

1. ✅ 引擎版本一致性（pubspec.yaml, version.h, CHANGELOG, .cursorrules）
2. ✅ Web SDK 版本验证（独立版本号）
3. ✅ **文档一致性检查**（Flutter 和 Web 开发者文档）
4. ✅ Windows version.h 文件
5. ✅ CHANGELOG_CN.md 版本条目
6. ✅ .cursorrules 版本号
7. ✅ Flutter Lint 检查
8. ✅ WebView2 SDK 就绪
9. ✅ Web SDK 已构建
10. ✅ 文档文件完整性
11. ✅ Git 工作区状态
12. ✅ Release 目录冲突检查
13. ✅ Web SDK 单元测试

#### 新增检查项（v2.4.1+）

- ✅ **文档一致性自动检查**: 验证 Flutter 和 Web 开发者文档是否存在
- ✅ **双版本号显示**: 明确显示 Engine 和 SDK 的独立版本号
- ✅ **文档更新提醒**: 自动列出需要检查的文档清单

**⚠️ 重要**: 如果检查失败，脚本会阻止发版，必须修复所有错误后才能继续。

---

### Step 4: 编译测试

```bash
cd example
flutter clean
flutter pub get
flutter build windows --debug
flutter build windows --release
.\scripts\test_full.bat
```

#### 检查清单

- [ ] Debug/Release 构建成功
- [ ] 无 Linter 警告：`flutter analyze`
- [ ] 功能测试通过（至少 2 个测试页面）
- [ ] 验证 .lib 文件已生成

---

### Step 5: 构建发布包

```bash
.\scripts\release.bat
```

**注意**: `release.bat` 会自动运行版本一致性检查，无需手动执行 `check_version_consistency.ps1`

**推荐**: 构建完成后立即运行自动化测试：
```bash
.\scripts\test_precompiled_package.ps1 -Version {版本号} -TestLevel Quick
```

#### 构建后检查

**基本检查**:
- [ ] 构建成功（ERROR_COUNT = 0）
- [ ] DLL/LIB 文件生成
- [ ] 生成 3 个 ZIP 文件（⚠️ 注意版本号）：
  - `anywp_engine_v{引擎版本号}_precompiled.zip` - 预编译包（使用引擎版本）
  - `anywp_engine_v{引擎版本号}_source.zip` - 完整源码包（使用引擎版本）
  - `anywp_web_sdk_v{SDK版本号}.zip` - Web SDK 包（使用 SDK 版本）
- [ ] `release/GITHUB_RELEASE_NOTES_v{引擎版本号}.md` 自动生成
- [ ] `release/commit_msg_v{引擎版本号}.txt` 自动生成

**版本号说明**:
- **引擎包**（precompiled/source）使用 `pubspec.yaml` 中的版本号
- **Web SDK 包**使用 `sdk/src/package.json` 中的版本号
- 发布说明主要基于引擎版本，但会标注 SDK 版本

#### 预编译包验证（必须全部存在）

- [ ] `bin/anywp_engine_plugin.dll`
- [ ] `bin/WebView2Loader.dll`
- [ ] `lib/anywp_engine_plugin.lib` ⚠️ **最易遗漏**
- [ ] `lib/dart/anywp_engine.dart`
- [ ] `include/anywp_engine/anywp_engine_plugin_c_api.h` ⚠️ **纯C API头文件**
- [ ] `windows/CMakeLists.txt`

#### 源码包验证（额外包含）

- [ ] `windows/anywp_engine_plugin.cpp/h` - 完整 C++ 源码
- [ ] `windows/modules/` + `windows/utils/` - 所有模块和工具类
- [ ] `sdk/src/` - TypeScript SDK 源码
- [ ] `windows/packages/` - WebView2 依赖包

#### Web SDK 包验证

- [ ] `sdk/anywp_sdk.js` + `sdk/anywp_sdk.min.js`
- [ ] `docs/WEB_DEVELOPER_GUIDE_CN.md`
- [ ] `docs/WEB_DEVELOPER_GUIDE.md`
- [ ] `docs/API_USAGE_EXAMPLES.md`
- [ ] `examples/` 目录
- [ ] `README.md`

**验证命令**: 
```bash
.\scripts\verify_precompiled.bat {引擎版本号} {SDK版本号}
```
自动验证预编译包、源码包与 Web SDK 包的完整性。

---

### Step 6: Git 提交与推送

#### 方式一：自动化脚本（推荐）

```bash
.\scripts\release_git.bat {引擎版本号}
```

**自动化功能**：
- ✅ 自动检查提交模板文件是否存在
- ✅ 自动添加需要提交的文件
- ✅ 使用生成的提交模板进行提交
- ✅ 自动创建并推送 Tag
- ✅ 支持 `--no-push` 参数（仅本地提交，不推送）

#### 方式二：手动执行

```bash
# 1. 添加文件
git add release/ CHANGELOG_CN.md pubspec.yaml docs/ README.md .cursorrules

# 2. 提交（使用文件方式避免中文乱码）
git commit -F release/commit_msg_v{引擎版本号}.txt

# 3. 推送到远程
git push origin main

# 4. 创建并推送 Tag
git tag -a v{引擎版本号} -m "AnyWP Engine v{引擎版本号}"
git push origin v{引擎版本号}
```

**⚠️ 重要**: 中文提交必须使用文件方式（`-F`），禁止使用 `git commit -m "中文信息"`，会导致乱码。

---

### Step 7: GitHub Release

#### 双版本号发布策略

- **Release Tag**: 使用引擎版本号 `v{引擎版本号}`
- **Release Title**: `AnyWP Engine v{引擎版本号}`（主版本）
- **Release Notes**: 在描述中明确标注 SDK 版本号

#### 发布步骤

1. **访问**: `https://github.com/zhaibin/AnyWallpaper-Engine/releases/new`

2. **填写信息**:
   - **Tag**: `v{引擎版本号}` (例如: v2.4.1)
   - **Title**: `AnyWP Engine v{引擎版本号}`

3. **Release Notes**: 复制 `release/GITHUB_RELEASE_NOTES_v{引擎版本号}.md`
   
   ⚠️ **在顶部添加版本信息**：
   ```markdown
   **Flutter Plugin Version**: v{引擎版本号}
   **Web SDK Version**: v{SDK版本号}

   > **Note**: Flutter Plugin and Web SDK have independent version numbers.

   [原有内容...]
   ```

4. **上传文件**（3个ZIP文件）:
   - `anywp_engine_v{引擎版本号}_precompiled.zip` - **推荐给 Flutter 开发者**
   - `anywp_engine_v{引擎版本号}_source.zip` - 完整源码（高级用户）
   - `anywp_web_sdk_v{SDK版本号}.zip` - **Web 壁纸开发者**（注意使用 SDK 版本号）

5. **勾选**: "Set as the latest release"

#### 包说明

| 包名 | 用途 | 目标用户 |
|------|------|---------|
| **Precompiled** | 最简单的集成方式，只包含必要的 DLL、LIB 和纯 C API 头文件，无需 WebView2 开发环境 | Flutter 开发者 |
| **Source** | 包含完整源码、模块、工具类和 WebView2 依赖，适合需要自定义修改的开发者 | 高级开发者 |
| **Web SDK** | 独立的 JavaScript SDK，供 HTML 壁纸开发者使用（独立版本号） | Web 壁纸开发者 |

#### 版本号示例

```
Release: v2.4.1
Title: AnyWP Engine v2.4.1
Files:
  - anywp_engine_v2.4.1_precompiled.zip   (引擎版本)
  - anywp_engine_v2.4.1_source.zip        (引擎版本)
  - anywp_web_sdk_v2.4.0.zip              (SDK版本 - 可能不同)
```

---

## CHANGELOG 格式规范

### 版本标注规则

- 如果只更新引擎，条目标题使用 `[引擎版本号]`
- 如果只更新 SDK，条目标题使用 `[SDK 版本号]` 并标注 `Web SDK`
- 如果同时更新，在条目中明确区分 Engine 和 SDK 更新

### 示例 1：仅引擎更新

```markdown
## [2.5.0] - 2025-11-20

### 新增功能
- 添加自动恢复机制
- 优化 WorkerW 创建流程

### 修复
- 修复 Explorer 重启后壁纸丢失问题

### 性能优化
- 减少内存占用
- 提升启动速度
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

### 示例 3：同时更新（推荐分开标注）

```markdown
## [2.5.0] - 2025-11-20

### 引擎更新 (Engine v2.5.0)
- 添加音频控制支持
- 新增 `setVolume` Dart API
- 优化资源管理

### SDK 更新 (SDK v2.5.0)
- 新增 `AnyWP.setVolume()` JavaScript API
- 与引擎音频控制配合使用
- 改进类型定义

**版本信息**: Engine v2.5.0 + SDK v2.5.0
```

---

## Git 提交规范

### 提交信息格式

```
<type>(<scope>): <subject>

<body>
```

### Type 类型

| Type | 说明 | 示例 |
|------|------|------|
| `feat` | 新功能 | feat(engine): 添加自动恢复机制 |
| `fix` | 修复 Bug | fix(sdk): 修复拖拽事件冲突 |
| `docs` | 文档更新 | docs: 更新发版指南 |
| `refactor` | 代码重构 | refactor(modules): 优化模块结构 |
| `chore` | 构建/工具变动 | chore: 更新依赖版本 |
| `release` | 发版相关 | release: v2.4.1 |

### 中文提交（必须使用文件方式）

```bash
# 创建提交信息文件
echo "feat: 添加新功能" > commit_msg.txt
echo "" >> commit_msg.txt
echo "详细说明" >> commit_msg.txt

# 使用文件提交
git commit -F commit_msg.txt

# 清理临时文件
del commit_msg.txt
```

**❌ 禁止**: `git commit -m "中文信息"` - 会导致乱码

---

## 常见问题

### Q1: 什么时候需要更新引擎版本？

**A**: 当修改了以下内容时需要更新引擎版本：
- C++ 代码（`windows/` 目录）
- Dart 代码（`lib/` 目录）
- 插件的核心功能
- 平台接口变更

### Q2: 什么时候需要更新 SDK 版本？

**A**: 当修改了以下内容时需要更新 SDK 版本：
- TypeScript/JavaScript SDK 代码（`sdk/src/` 目录）
- Web API 的添加、修改或删除
- SDK 的类型定义
- JavaScript 接口变更

### Q3: 可以只更新其中一个版本吗？

**A**: 可以！两个版本号是独立的：
- 只改了引擎 → 只更新 `pubspec.yaml`
- 只改了 SDK → 只更新 `sdk/src/package.json`
- 两者都改 → 分别更新两个文件

### Q4: GitHub Release 的 Tag 用哪个版本号？

**A**: 使用引擎版本号（`pubspec.yaml`），原因：
- Release 主要针对 Flutter 开发者
- 引擎是主产品
- 在 Release Notes 中明确标注 SDK 版本

### Q5: 发版前检查失败怎么办？

**A**: 按以下步骤处理：
1. 仔细阅读错误信息
2. 修复所有错误
3. 重新运行 `pre_release_check.bat`
4. 确保所有检查通过后再继续

### Q6: Web SDK 包的版本号为什么不同？

**A**: 这是正常的！
- Web SDK 有独立的版本号管理
- 只有在 SDK 功能变更时才更新 SDK 版本
- 引擎和 SDK 可以独立演进

### Q7: 如何验证发布包的完整性？

**A**: 使用验证脚本：
```bash
.\scripts\verify_precompiled.bat {引擎版本号} {SDK版本号}
```
脚本会自动检查所有必需文件是否存在。

---

## 脚本参考

### 核心脚本

| 脚本 | 用途 | 参数 |
|------|------|------|
| `pre_release_check.bat` | 发版前全面检查（13项） | 无 |
| `release.bat` | 构建发布包（3个 ZIP） | 无 |
| `release_git.bat` | 自动 Git 提交和推送 | `{版本号}` |
| `verify_precompiled.bat` | 验证发布包完整性 | `{引擎版本号} [SDK版本号]` |

### 辅助脚本

| 脚本 | 用途 | 参数 |
|------|------|------|
| `check_version_consistency.ps1` | 版本一致性检查 | `-Version {版本号}` |
| `check_docs_consistency.ps1` | 文档一致性检查 | `-EngineVersion -SdkVersion` |
| `generate_release_notes.ps1` | 生成 Release Notes | `-Version {版本号}` |
| `generate_commit_template.ps1` | 生成提交模板 | `-Version {版本号}` |

### 快速命令参考

```bash
# 开发测试
.\scripts\build.bat       # 构建并运行
.\scripts\debug.bat       # 调试模式
.\scripts\test_full.bat   # 全面测试

# SDK 构建
.\scripts\build_sdk.bat   # 构建 Web SDK

# 发版流程
.\scripts\pre_release_check.bat          # 发版前检查
.\scripts\release.bat                    # 构建发布包
.\scripts\verify_precompiled.bat 2.4.1 2.4.0   # 验证发布包（第二个参数为 SDK 版本，可省略自动读取）
.\scripts\release_git.bat 2.4.1          # Git 提交推送
```

---

## 附录：发版检查清单

使用此清单确保发版流程完整：

### 版本号更新
- [ ] 确定更新类型（引擎/SDK/两者）
- [ ] 更新相应的版本号文件
- [ ] 更新 CHANGELOG_CN.md
- [ ] 更新 .cursorrules 版本号（引擎更新时）

### 文档更新
- [ ] 更新 Flutter 开发者文档（引擎更新时）
- [ ] 更新 Web 开发者文档（SDK 更新时）
- [ ] 更新 README.md
- [ ] 检查代码注释与文档一致性

### 构建测试
- [ ] 运行 `pre_release_check.bat`（13项检查全部通过）
- [ ] Debug 构建成功
- [ ] Release 构建成功
- [ ] 功能测试通过
- [ ] 单元测试通过

### 发布包
- [ ] 运行 `release.bat` 构建成功
- [ ] 生成 3 个 ZIP 文件
- [ ] 运行 `verify_precompiled.bat` 验证通过
- [ ] Release Notes 自动生成

### Git 发布
- [ ] 提交代码到 Git
- [ ] 创建并推送 Tag
- [ ] 创建 GitHub Release
- [ ] 上传 3 个 ZIP 文件
- [ ] Release Notes 包含双版本号

---

## 更新历史

- **v2.4.1** (2025-11-20): 引入双版本号独立管理，添加文档一致性检查
- **v2.3.0** (2024-XX-XX): 版本号统一管理，CMake 自动生成
- **v2.2.0** (2024-XX-XX): 引入自动化发版脚本

---

**文档维护**: 本文档应与 `.cursorrules` 保持同步  
**反馈渠道**: GitHub Issues  
**最后审核**: 2025-11-20
