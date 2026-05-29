# AnyWP Engine 发布规范

Last updated: 2026-05-28

本文档定义 AnyWP Engine 的发布准入、版本、产物、验证、提交和回滚规范。具体命令流程参考 [RELEASE_GUIDE.md](RELEASE_GUIDE.md)，版本细节参考 [VERSION_MANAGEMENT.md](VERSION_MANAGEMENT.md)。

## 发布目标

每次发布必须满足：

- 版本来源清晰。
- 产物可复现。
- 文档与代码一致。
- 测试或验证证据可追溯。
- Git 提交、tag 和 release notes 能对应到同一批产物。

## 发布类型

| 类型 | 触发条件 | 必需版本更新 |
|------|----------|--------------|
| Engine 发布 | Dart API、Windows C++、macOS Objective-C、Flutter 插件能力变化 | `pubspec.yaml` |
| Web SDK 发布 | `sdk/src/` API、类型、运行时行为或分发 JS 变化 | `sdk/src/package.json` |
| 联合发布 | Engine 和 Web SDK 同时变化，或需要同时打包分发 | `pubspec.yaml` 与 `sdk/src/package.json` |
| 文档发布 | 仅修正文档、示例说明、排障说明 | 通常不更新版本，除非文档修正发布包内容 |

## 版本规范

- 引擎版本以 `pubspec.yaml` 为源。
- macOS podspec 版本通过 `scripts/sync_version.sh` 从 `pubspec.yaml` 同步。
- Web SDK 版本以 `sdk/src/package.json` 为源。
- `windows/version.h`、分发包目录、release notes 和 commit 模板由构建或发布脚本生成，不手工维护。
- 引擎和 SDK 版本可以不同；发布说明必须同时列出本次涉及的 Engine 版本和 SDK 版本。
- 版本号遵循 `MAJOR.MINOR.PATCH`：
  - `MAJOR`：破坏兼容或迁移成本高。
  - `MINOR`：向后兼容的新能力。
  - `PATCH`：修复、稳定性、文档或构建修正。

## 发布准入

发布前必须完成：

- 工作区只包含本次发布相关改动；无关本地改动不得混入发布提交。
- `CHANGELOG_CN.md` 已记录用户可见变更。
- API、示例和平台差异文档已按 [DOCUMENTATION_STANDARDS.md](DOCUMENTATION_STANDARDS.md) 检查。
- 编码和测试要求已按 [CODING_STANDARDS.md](CODING_STANDARDS.md) 执行。
- 发布脚本和构建脚本没有本地绝对路径、临时调试开关或机器相关假设。

## 必跑验证

根据发布范围执行：

### 环境边界

- Windows 发布验证必须在可用的 **Windows 环境** 中执行。本地开发默认使用 Parallels Desktop Windows VM；其他开发者可以使用真实 Windows 机器、Windows VM 或 CI Windows runner。
- macOS 主机侧不能替代 Windows `.bat`、WebView2、DLL/LIB、WorkerW、Explorer 恢复或 Windows Flutter 构建验证。
- 如果当前只在 macOS shell 中完成了静态检查，发布记录必须标注 Windows 验证为“待 Windows 环境执行”。

### Engine 发布

```bash
flutter analyze
```

Windows 发布还需要：

```bat
REM Run inside a Windows environment from the repository root.
.\scripts\pre_release_check.bat
cd example
flutter build windows --debug
flutter build windows --release
cd ..
.\scripts\test_full.bat
```

macOS 发布还需要：

```bash
bash scripts/sync_version.sh
bash scripts/release_macos.sh
```

### Web SDK 发布

从 `sdk/src/` 运行：

```bash
npm run typecheck
npm test
npm run build
```

### 文档发布

```bash
find docs -maxdepth 1 -name '*.md' | wc -l
```

并人工检查 `docs/README.md`、`docs/DOCUMENTATION_INDEX.md`、新增或变更链接。

## 发布产物

发布包必须能从脚本重新生成。常见产物包括：

- Windows 预编译包：`release/anywp_engine_v{engine_version}_precompiled.zip`。
- Windows 源码包：`release/anywp_engine_v{engine_version}_source.zip`。
- Web SDK 包：`release/anywp_web_sdk_v{sdk_version}.zip`。
- GitHub release notes：`release/GITHUB_RELEASE_NOTES_v{engine_version}.md`。
- 提交模板：`release/commit_msg_v{engine_version}.txt`。

预编译包必须至少包含：

- `bin/anywp_engine_plugin.dll`。
- `bin/WebView2Loader.dll`。
- `lib/anywp_engine_plugin.lib`。
- `lib/dart/anywp_engine.dart`。
- `include/anywp_engine/anywp_engine_plugin_c_api.h`。
- `windows/CMakeLists.txt`。

发布包生成后运行：

```bat
REM Run inside a Windows environment.
.\scripts\release.bat
.\scripts\verify_precompiled.bat {engine_version} {sdk_version}
```

## Git 和 Tag 规范

- 发布提交使用生成的 `release/commit_msg_v{engine_version}.txt`，避免手写遗漏。
- 发布 tag 使用 `v{engine_version}`。当仅发布 SDK 且引擎版本不变时，tag 和 release notes 必须在说明中明确 SDK 版本，避免误导。
- 推送前确认本地分支与远端主分支关系清楚，不把未验证的本地修改混入。
- 不在发布提交里夹带无关重构、实验代码、临时日志或本地配置。

## Release Notes 规范

Release notes 必须包含：

- Engine 版本和 SDK 版本。
- 支持平台。
- 新增、修复、变更、文档、已知问题。
- 破坏性变更和迁移步骤。
- 产物列表和校验方式。

对用户影响较大的改动必须写清：

- 谁会受影响。
- 升级前需要做什么。
- 升级后如何验证。
- 出错时如何回退。

## 回滚规范

发布后发现问题时：

- 先判断是否需要撤回 release artifact、撤回 tag、发布修复版本或仅更新文档。
- 已推送 tag 不直接删除，除非发布尚未公开且团队确认。
- 修复版本优先使用新的 patch 版本，不覆盖已公开 zip。
- 回滚说明必须记录在 `CHANGELOG_CN.md` 或 release notes 中。

## 发布检查清单

发布前逐项确认：

- 版本号来源正确，Engine 和 SDK 版本关系已说明。
- 构建、测试、文档检查命令已执行并记录结果。
- Windows 检查已在 Windows 环境中执行，或明确标记为阻塞项。
- 发布包可解压，关键文件齐全。
- 文档、示例和 changelog 与本次发布一致。
- Git 提交和 tag 使用正确版本。
- 回滚路径明确。
