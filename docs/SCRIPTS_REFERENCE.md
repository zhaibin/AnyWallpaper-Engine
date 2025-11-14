# AnyWP Engine - Scripts Reference

**Last Updated**: 2025-11-15  
**Version**: 2.1.5

---

## Overview

The repository now维持 **16** 个脚本（含 PowerShell 模块），覆盖环境准备、日常开发、自动化测试以及发版流程。全部脚本均为英文，默认使用 `pwsh` ≥ 7.5.4 执行。

| 分类 | 脚本 | 说明 |
| --- | --- | --- |
| Setup & SDK | `setup.bat` | 安装 WebView2 依赖 |
|  | `build_sdk.bat` | 编译 TypeScript SDK，运行单测 |
| Development | `build.bat` | 构建并运行示例（Debug） |
|  | `run.bat` | 快速运行已有构建 |
|  | `debug.bat` | 带日志的调试模式 |
|  | `monitor_log.bat` | Tail `test_logs\debug_run.log` |
| Testing | `test_full.bat` | 8 个演示页面自动化测试 |
|  | `analyze.ps1` | 解析 `test_full` 输出，支持 HTML 报告 |
| Release Automation | `release.bat` | 构建预编译包 / 源码包 / Web SDK 包 |
|  | `check_version_consistency.ps1` | 校验版本号一致性 |
|  | `generate_release_notes.ps1` | 从 CHANGELOG 生成说明 |
|  | `generate_commit_template.ps1` | 生成中文提交模板 |
|  | `release_git.bat` | 自动执行 Git add/commit/tag/push |
|  | `verify_precompiled.bat` | 验证三类发布包内容 |
| Support Module | `release_utils.psm1` | PowerShell 辅助函数库 |

---

## Setup & SDK

### `setup.bat`
- 下载/更新 `nuget.exe`
- 安装 `Microsoft.Web.WebView2.1.0.2592.51`
- 输出：`windows\packages\Microsoft.Web.WebView2.*`

**使用场景**：首次克隆仓库、CI 环境或依赖损坏时执行。

### `build_sdk.bat`
- 安装 `windows\sdk\package.json` 依赖
- 执行 Rollup 打包 `windows\anywp_sdk.js`
- 运行单测并输出覆盖率（`windows\sdk\coverage\`）

**Tip**：发版前需确保最新 `anywp_sdk.js` 已生成。

---

## Development Scripts

### `build.bat`
1. 执行 `flutter clean` / `flutter pub get`
2. `flutter build windows --debug`
3. 启动示例应用

### `run.bat`
- 直接运行 `example\build\windows\x64\runner\Release\...exe`
- 若无 Release 构建，则回退 Debug

### `debug.bat`
- 基于 `build.bat`
- 将控制台输出记录至 `test_logs\debug_run.log`

### `monitor_log.bat`
- 每 2 秒刷新一次 `debug_run.log`，便于排查实时日志

---

## Testing

### `test_full.bat`
- 构建 Debug 版本
- 依次打开 8 个演示页面（约 95 秒）
- 采集以下文件：
  - `test_logs\test_full_{timestamp}.log`
  - `test_logs\memory_{timestamp}.csv`
  - `test_logs\cpu_{timestamp}.csv`
  - `test_logs\app_output_{timestamp}.log`

### `analyze.ps1`
- 解析上述 CSV/LOG
- 指标：最大/平均内存、CPU、增长率
- 选项：`-GenerateHtml` 为 `test_logs\performance_report_{timestamp}.html`

> 建议发版前执行一次 `test_full` + `analyze.ps1`，确认无性能回归。

---

## Release Automation

### `check_version_consistency.ps1`
校验以下位置的版本号：
- `pubspec.yaml`
- `CHANGELOG_CN.md`
- `.cursorrules`
- `docs/PRECOMPILED_DLL_INTEGRATION.md`

在 `release.bat` 中作为第一步执行。

### `generate_release_notes.ps1`
- 读取最新 `CHANGELOG_CN.md` 条目
- 输出 `release/GITHUB_RELEASE_NOTES_v{version}.md`

### `generate_commit_template.ps1`
- 同步解析 changelog
- 生成 `release/commit_msg_v{version}.txt`（中文提交模板）

### `release_utils.psm1`
PowerShell 辅助模块，提供：
- `Get-PubspecVersion`
- `Get-ChangelogSection`
- 复用逻辑被以上 `.ps1` 脚本调用

### `release.bat`
1. 调用 `check_version_consistency.ps1`
2. 构建 Flutter Release（example/windows）
3. 生成：
   - `release/anywp_engine_v{version}_precompiled.zip`
   - `release/anywp_engine_v{version}_source.zip`
   - `release/anywp_web_sdk_v{version}.zip`
4. 自动执行：
   - `generate_release_notes.ps1`
   - `generate_commit_template.ps1`
5. 输出下一步提示（验证、Git、GitHub Release）

### `verify_precompiled.bat`
核对三类发布包的关键文件是否齐全；发包后务必执行：

```bash
scripts\verify_precompiled.bat 2.1.5
```

### `release_git.bat`
```bash
scripts\release_git.bat 2.1.5         # 默认 push
scripts\release_git.bat 2.1.5 --no-push
```
- 自动执行 `git add` / `git commit`（使用生成的模板）
- 创建并推送 tag（可跳过 push）

---

## 快速流程

```bash
# 0. 环境准备（首次执行）
scripts\setup.bat

# 1. SDK 如有改动
scripts\build_sdk.bat

# 2. 自动化测试
scripts\test_full.bat
pwsh ./scripts/analyze.ps1 -GenerateHtml

# 3. 发版构建
scripts\release.bat
scripts\verify_precompiled.bat 2.1.5
scripts\release_git.bat 2.1.5
```

> GitHub Release 页面内容可直接复制 `release/GITHUB_RELEASE_NOTES_v{version}.md`，上传三个 ZIP 包即可。

