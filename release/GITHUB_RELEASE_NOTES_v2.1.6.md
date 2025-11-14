# AnyWP Engine v2.1.6 - Release Notes

**发布日期**: 2025-11-14
**版本**: 2.1.6

---

## 🐛 修复 Windows 桌面架构兼容性


## 🐛 核心修复

### Windows 10 FCU+ 桌面架构兼容性
- **问题描述**: 在 Windows 10 Fall Creators Update (1709+) 的部分系统上，`SHELLDLL_DefView` 直接位于 `Progman` 窗口内部而不是在 `WorkerW` 中，导致壁纸无法显示
- **修复内容**:
  - `desktop_wallpaper_helper.h/cpp`: 添加 `FindChildWindowByClass` 递归查找方法，支持在任意层级查找 SHELLDLL_DefView
  - `desktop_wallpaper_helper.cpp`: 当 SHELLDLL 在 Progman 中时直接使用 Progman 作为父窗口
  - `desktop_wallpaper_helper.cpp`: 增强 WorkerW 创建逻辑，使用 `SendMessageTimeoutW` 和 150ms 延迟确保创建完成
  - `window_manager.cpp`: 增强 Z-order 设置的 fallback 逻辑，支持在 Progman 和 WorkerW 中递归查找 SHELLDLL_DefView
  - `window_manager.cpp`: 添加 `HWND_BOTTOM` 作为最终 fallback，确保在极端情况下也能设置基本的 Z-order
- **兼容性**: 
  - ✅ 支持传统架构（SHELLDLL 在 WorkerW 中）
  - ✅ 支持新架构（SHELLDLL 在 Progman 中）
  - ✅ 向后兼容所有已测试的 Windows 10/11 版本

## 🔧 开发环境改进

- **CMakeLists.txt**: 添加 POST_BUILD 命令自动复制 `anywp_sdk.js` 到开发目录，修复开发环境下 SDK 路径问题

## 🧹 代码清理

- **脚本目录清理**: 删除过时的测试脚本（`auto_test_sdk_injection.bat`、`verify*.bat` 等）
- **文档更新**: 更新 `scripts/README.md` 和 `SCRIPTS_REFERENCE.md`

## 📦 依赖更新

- `window_manager`: 0.3.7 → 0.5.1
- `flutter_lints`: 3.0.0 → 6.0.0
- `@types/node`: 20.19.25 → 22.10.1
- `typescript`: 5.7.3 → 5.9.3 (注: SDK 降级以保持稳定性)

## 🔒 .gitignore 更新

- 添加临时开发文件忽略规则（`.claude/`, `commit_*.bat`, `windows/nul` 等）

---

