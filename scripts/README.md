# 脚本说明

AnyWP Engine 项目的构建和运行脚本。

## 📜 脚本列表

### setup_webview2.bat
安装 WebView2 SDK（首次构建前必须运行）。

```bash
.\scripts\setup_webview2.bat
```

### build_and_run.bat
构建并运行（推荐）。

```bash
.\scripts\build_and_run.bat
```

### run.bat
灵活的运行工具。

```bash
# 运行 Debug 版本（默认）
.\scripts\run.bat

# 运行 Release 版本
.\scripts\run.bat -r

# 使用 Flutter 运行（支持热重载）
.\scripts\run.bat -f

# 帮助
.\scripts\run.bat -h
```

### test.bat
测试脚本（自动选择 Release/Debug 版本）。

```bash
.\scripts\test.bat
```

### PUSH_TO_GITHUB.bat
推送到 GitHub（推送到 main 分支）。

```bash
.\scripts\PUSH_TO_GITHUB.bat
```

---

## 🚀 快速开始

```bash
# 1. 首次使用
.\scripts\setup_webview2.bat
.\scripts\build_and_run.bat

# 2. 日常开发（热重载）
.\scripts\run.bat -f

# 3. 测试
.\scripts\test.bat
```

---

## 相关文档

- [快速开始](../docs/QUICK_START.md)
- [测试指南](../docs/TESTING_GUIDE.md)

