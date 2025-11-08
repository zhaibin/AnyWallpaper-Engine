# 项目结构

AnyWP Engine 的完整目录结构和文件说明。

## 📁 目录结构

```
AnyWallpaper Engine/
├── 📄 README.md                     # 项目主文档（英文）
├── 📄 pubspec.yaml                  # Flutter 插件配置
├── 📄 pubspec.lock                  # 依赖锁定文件
│
├── 📂 lib/                          # Dart 公共 API
│   └── anywp_engine.dart            # Flutter 插件接口
│
├── 📂 windows/                      # Windows 平台实现
│   ├── anywp_engine_plugin.cpp           # 核心 C++ 实现（重构后更精简）
│   ├── anywp_engine_plugin.h             # 插件头文件
│   ├── anywp_sdk.js                      # JavaScript SDK
│   ├── CMakeLists.txt                    # CMake 构建配置
│   ├── packages.config                   # NuGet 包配置
│   ├── 📂 modules/                       # 功能模块 (v1.3.2+)
│   │   ├── iframe_detector.cpp/h         # iframe 检测与坐标映射
│   │   ├── sdk_bridge.cpp/h              # JavaScript SDK 注入与消息桥接
│   │   ├── mouse_hook_manager.cpp/h      # 鼠标钩子管理（框架）
│   │   ├── monitor_manager.cpp/h         # 多显示器管理（框架）
│   │   └── power_manager.cpp/h           # 省电与性能优化（框架）
│   ├── 📂 utils/                         # 工具类 (v1.3.2+)
│   │   ├── state_persistence.cpp/h       # 应用级状态持久化
│   │   ├── url_validator.cpp/h           # URL 白名单/黑名单验证
│   │   └── logger.cpp/h                  # 日志工具
│   ├── 📂 sdk/                           # JavaScript SDK 源码 (v4.2.0+)
│   │   ├── core/                         # 核心模块
│   │   ├── modules/                      # 功能模块
│   │   ├── utils/                        # 工具模块
│   │   └── rollup.config.js              # 构建配置
│   ├── 📂 include/                       # 公共头文件
│   │   └── anywp_engine/
│   │       ├── anywp_engine_plugin.h                 # 主头文件
│   │       ├── anywp_engine_plugin_c_api.h           # C API 头文件
│   │       ├── any_w_p_engine_plugin.h               # Flutter 兼容副本
│   │       ├── any_w_p_engine_plugin_c_api.h         # Flutter 兼容副本
│   │       └── README.md                              # 头文件说明
│   └── 📂 packages/                      # NuGet 包（WebView2）
│       └── Microsoft.Web.WebView2.*/
│
├── 📂 example/                      # 示例应用
│   ├── 📂 lib/
│   │   └── main.dart                # 示例应用主程序
│   ├── 📂 windows/                  # Windows 平台配置
│   ├── 📂 build/                    # 构建输出（生成）
│   ├── pubspec.yaml                 # 示例应用配置
│   └── README.md                    # 示例说明
│
├── 📂 docs/                         # 文档目录
│   ├── README.md                    # 文档索引
│   ├── README_CN.md                 # 中文文档
│   ├── QUICK_START.md               # 快速开始
│   ├── TESTING_GUIDE.md             # 测试指南
│   ├── USAGE_EXAMPLES.md            # 使用示例
│   ├── TROUBLESHOOTING.md           # 故障排除
│   ├── API_BRIDGE.md                # API 桥接文档
│   ├── TECHNICAL_NOTES.md           # 技术说明
│   ├── BUILD_INFO.md                # 构建信息
│   ├── OPTIMIZATION_GUIDE.md        # 优化指南
│   ├── OPTIMIZATION_COMPLETED.md    # 优化报告
│   ├── GITHUB_SETUP.md              # GitHub 设置
│   ├── FINAL_SUMMARY.md             # 项目总结
│   └── PROJECT_STRUCTURE.md         # 本文档
│
├── 📂 scripts/                      # 脚本目录
│   ├── README.md                    # 脚本说明
│   ├── setup_webview2.bat           # WebView2 SDK 安装
│   ├── build_and_run.bat            # 构建并运行
│   ├── run.bat                      # 灵活运行工具
│   ├── test.bat                     # 自动测试
│   └── PUSH_TO_GITHUB.bat           # Git 推送辅助
│
└── 📂 examples/                     # 测试文件
    ├── test_simple.html             # 简单测试页面
    ├── test_api.html                # API 测试页面
    └── test_iframe_ads.html         # iframe 测试页面
```

## 📝 核心文件说明

### 根目录

| 文件 | 说明 |
|------|------|
| `README.md` | 项目主文档，包含功能介绍、快速开始、技术架构等 |
| `pubspec.yaml` | Flutter 插件配置文件 |

### lib/ - Dart API

| 文件 | 说明 |
|------|------|
| `anywp_engine.dart` | Flutter 插件的 Dart 接口，提供 3 个主要方法：<br>- `initializeWallpaper()` - 初始化壁纸<br>- `stopWallpaper()` - 停止壁纸<br>- `navigateToUrl()` - 导航到新 URL |

### windows/ - Windows 实现

| 文件/目录 | 说明 |
|------|------|
| `anywp_engine_plugin.cpp` | 核心 C++ 实现（重构后更精简），包含：<br>- WorkerW 查找和挂载<br>- WebView2 初始化<br>- Flutter 方法桥接<br>- 委托功能到专门模块 |
| `anywp_engine_plugin.h` | 插件类定义和主要接口 |
| `anywp_sdk.js` | JavaScript SDK（构建后的单文件） |
| `CMakeLists.txt` | CMake 构建配置，链接所有模块 |
| **modules/** | **功能模块目录 (v1.3.2+)** |
| `modules/iframe_detector.*` | iframe 边界检测与坐标映射 |
| `modules/sdk_bridge.*` | JavaScript SDK 注入与消息桥接 |
| `modules/mouse_hook_manager.*` | 鼠标钩子管理（框架） |
| `modules/monitor_manager.*` | 多显示器枚举与变更监听（框架） |
| `modules/power_manager.*` | 省电优化与性能管理（框架） |
| **utils/** | **工具类目录 (v1.3.2+)** |
| `utils/state_persistence.*` | 应用级键值对状态持久化 |
| `utils/url_validator.*` | URL 白名单/黑名单验证 |
| `utils/logger.*` | 统一日志记录接口 |
| **sdk/** | **JavaScript SDK 源码 (v4.2.0+)** |
| `sdk/core/` | 核心模块（AnyWP 对象、初始化） |
| `sdk/modules/` | 功能模块（事件、拖拽、存储等） |
| `sdk/utils/` | 工具模块（坐标转换、调试等） |

### docs/ - 文档

| 类型 | 文档 | 用途 |
|------|------|------|
| **用户文档** | README_CN.md | 完整的中文使用指南 |
| | QUICK_START.md | 快速开始（3 步上手） |
| | TESTING_GUIDE.md | 详细测试步骤和检查点 |
| | USAGE_EXAMPLES.md | 代码示例和使用场景 |
| | TROUBLESHOOTING.md | 常见问题和解决方案 |
| **技术文档** | TECHNICAL_NOTES.md | 深度技术实现细节 |
| | API_BRIDGE.md | JavaScript Bridge 完整文档 |
| | BUILD_INFO.md | 编译和分发说明 |
| **优化文档** | OPTIMIZATION_GUIDE.md | 优化策略和方案 |
| | OPTIMIZATION_COMPLETED.md | P0/P1 优化实现报告 |
| **开发文档** | GITHUB_SETUP.md | GitHub 推送指南 |
| | FINAL_SUMMARY.md | 项目完成总结 |

### scripts/ - 脚本

| 脚本 | 用途 |
|------|------|
| `setup_webview2.bat` | 安装 WebView2 SDK（首次必须运行） |
| `build_and_run.bat` | 一键构建和运行（推荐） |
| `run.bat` | 灵活的运行工具，支持多种模式 |
| `test.bat` | 自动测试，带详细说明 |

### examples/ - 测试文件

| 文件 | 说明 |
|------|------|
| `test_simple.html` | 简单测试页面（推荐首次测试）<br>显示渐变背景和时钟 |
| `test_api.html` | API 功能测试页面<br>测试 onClick、openURL 等功能 |
| `test_iframe_ads.html` | iframe 广告检测测试页面<br>测试跨域 iframe 点击 |

## 🔧 构建产物

构建后会生成以下目录（已在 `.gitignore` 中）：

```
example/build/windows/x64/runner/
├── Debug/
│   └── anywallpaper_engine_example.exe    # Debug 版本
└── Release/
    └── anywallpaper_engine_example.exe    # Release 版本
```

## 📦 依赖包

### NuGet 包（自动下载）

```
windows/packages/Microsoft.Web.WebView2.1.0.2592.51/
├── build/native/include/               # WebView2 头文件
├── build/native/x64/                   # x64 库文件
└── build/native/x86/                   # x86 库文件
```

## 🎯 关键路径

### 开发时常用路径

```bash
# 构建
.\scripts\build_and_run.bat

# 运行
.\scripts\run.bat

# 测试
.\scripts\test.bat

# 示例 HTML
examples/test_simple.html
```

### 源代码路径

```bash
# Dart API
lib/anywp_engine.dart

# C++ 实现
windows/anywp_engine_plugin.cpp
windows/anywp_engine_plugin.h

# JavaScript SDK
windows/anywp_sdk.js
```

## 📖 文档导航

- 📘 [快速开始](QUICK_START.md) - 3 步上手
- 🧪 [测试指南](TESTING_GUIDE.md) - 如何测试
- 📚 [使用示例](USAGE_EXAMPLES.md) - 代码示例
- 🔧 [技术说明](TECHNICAL_NOTES.md) - 深度技术
- 🐛 [故障排除](TROUBLESHOOTING.md) - 问题解决

## 🚀 快速开始

```bash
# 1. 安装依赖
.\scripts\setup_webview2.bat

# 2. 构建并运行
.\scripts\build_and_run.bat

# 3. 测试
.\scripts\test.bat
```

---

## 🏗️ C++ 模块化架构 (v1.3.2+)

### 重构目标

将原本 4000+ 行的单文件 `anywp_engine_plugin.cpp` 重构为多个职责明确的模块，提高代码的可维护性和可测试性。

### 模块分类

#### 工具类 (`windows/utils/`)
- **StatePersistence**: 应用级状态持久化（键值对存储）
- **URLValidator**: URL 白名单/黑名单验证
- **Logger**: 统一日志记录接口

#### 功能模块 (`windows/modules/`)
- **IframeDetector**: iframe 边界检测与坐标映射（✅ 已完成）
- **SDKBridge**: JavaScript SDK 注入与消息桥接（✅ 已完成）
- **MouseHookManager**: 鼠标钩子管理（框架）
- **MonitorManager**: 多显示器枚举与变更监听（框架）
- **PowerManager**: 省电优化与性能管理（框架）

### 设计原则

1. **单一职责**: 每个模块只负责一个核心功能
2. **低耦合**: 模块之间通过接口交互，减少依赖
3. **易测试**: 独立模块便于单元测试和集成测试
4. **向后兼容**: 重构不改变外部 API 行为

---

**版本**: 1.3.2-dev  
**最后更新**: 2025-11-07  
**重大更新**: C++ 代码模块化重构完成

