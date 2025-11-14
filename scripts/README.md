# AnyWP Engine - Scripts Directory

All development, testing, and release scripts for AnyWP Engine.

**Quick Links**:
- **Complete Reference**: `../docs/SCRIPTS_REFERENCE.md`
- **Test Scripts Guide**: `README_TEST_SCRIPTS.md`

---

## 📁 Scripts Overview

> 当前目录共 **16** 个脚本（含 PowerShell 模块），按用途划分如下：

### Development
- `build.bat` – 构建并运行示例应用（Debug）
- `run.bat` – 直接运行已有构建（优先 Release）
- `debug.bat` – 附带日志采集的调试模式
- `monitor_log.bat` – 实时 tail `test_logs\debug_run.log`

### Setup & SDK
- `setup.bat` – 初始化/更新 WebView2 依赖
- `build_sdk.bat` – 编译 TypeScript Web SDK 并运行单测

### Testing
- `test_full.bat` – 自动化运行 8 个示例页面并采集日志
- `analyze.ps1` – 解析 `test_full` 生成的性能数据

### Release Automation
- `release.bat` – 一键构建预编译包 / 源码包 / Web SDK 包
- `check_version_consistency.ps1` – 版本一致性校验
- `generate_release_notes.ps1` – 从 `CHANGELOG_CN.md` 生成发布说明
- `generate_commit_template.ps1` – 生成中文提交模板
- `release_git.bat` – 自动执行 `git add` / commit / tag / push
- `verify_precompiled.bat` – 验证三类发布包内容
- `release_utils.psm1` – 供上述 PowerShell 脚本复用的工具模块

> ⚠️ **PowerShell 版本**：建议统一使用 `pwsh` ≥ 7.5，所有 `.ps1` 脚本均默认优先调用该版本。

---

## 🚀 Quick Start

```bash
# First time setup
scripts\setup.bat

# Daily development
scripts\build.bat

# Quick test
scripts\run.bat

# Full test
scripts\test_full.bat

# Release build
scripts\release.bat

# Git automation (可选)
scripts\release_git.bat 2.1.5
```

---

## 📖 Documentation

- **Full Reference**: `../docs/SCRIPTS_REFERENCE.md`
- **Test Guide**: `README_TEST_SCRIPTS.md`

---

**All scripts are in English to avoid encoding issues ✅**
