# 脚本说明

AnyWP Engine 项目的构建和运行脚本。

## 📜 脚本列表

### build_release.bat ⭐ 新增
构建并打包 Release 版本（用于 GitHub Releases）。

**功能**：
- 自动编译 Release 版本
- 收集 DLL、头文件、SDK、文档
- 生成完整的发布包
- 创建 ZIP 压缩包

```bash
.\scripts\build_release.bat
```

**输出**：`release/anywp_engine_v1.1.0.zip`

**详细文档**：[RELEASE_GUIDE.md](../docs/RELEASE_GUIDE.md)

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

