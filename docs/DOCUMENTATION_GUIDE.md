# AnyWP Engine - Documentation Guide

**文档导航指南** - 为两类开发者提供清晰的文档路径

---

## 👥 开发者类型识别

### 类型 1️⃣ : Flutter 应用开发者

**你的目标：** 将 AnyWP Engine 集成到你的 Flutter Windows 应用中

**你需要：**
- ✅ 如何在 pubspec.yaml 中添加依赖
- ✅ 如何调用 Dart API 控制壁纸
- ✅ 如何处理多显示器
- ✅ 如何配置省电和优化
- ✅ 如何处理错误和回调

**你的文档路径** 👇

---

### 类型 2️⃣ : Web 开发者

**你的目标：** 开发可作为壁纸的网页内容（HTML/React/Vue等）

**你需要：**
- ✅ 如何使用 JavaScript SDK
- ✅ 如何注册点击区域
- ✅ 如何在 React/Vue 中集成
- ✅ 如何处理 SPA 路由
- ✅ 如何优化性能和省电

**你的文档路径** 👇

---

## 📖 Flutter 应用开发者 - 文档路径

### 🚀 第一步：快速开始（5分钟）

**[QUICK_START.md](QUICK_START.md)**
- 安装依赖
- 添加到 pubspec.yaml
- 第一个壁纸应用
- 基础 API 调用

### 📦 第二步：集成到项目

**[PACKAGE_USAGE_GUIDE_CN.md](PACKAGE_USAGE_GUIDE_CN.md)**
- 三种集成方式（本地路径/Git/pub.dev）
- 详细配置说明
- 依赖管理
- 发布准备

### 📚 第三步：API 完整参考

**[DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md)** ⭐ 核心文档
- 所有 Dart API 完整说明
- 参数和返回值详解
- 多显示器支持
- 省电和优化 API
- 配置选项
- 回调机制

**内容索引：**
```dart
// 基础控制
initializeWallpaper()
stopWallpaper()
navigateToUrl()

// 多显示器
getMonitors()
initializeWallpaperOnMonitor()
stopWallpaperOnMonitor()

// 省电优化
pauseWallpaper()
resumeWallpaper()
setAutoPowerSaving()
getPowerState()
getMemoryUsage()
optimizeMemory()

// 配置
setIdleTimeout()
setMemoryThreshold()
setCleanupInterval()
getConfiguration()

// 回调
setOnMonitorChangeCallback()
setOnPowerStateChangeCallback()
```

### 💡 第四步：实用示例

**[API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md)**
- 7个完整示例（可直接使用）
  1. 简单壁纸应用
  2. 多显示器不同内容
  3. 镜像模式
  4. 电池感知应用
  5. 手动电源控制
  6. 用户偏好设置
  7. 完整壁纸管理器

### ⭐ 第五步：最佳实践

**[BEST_PRACTICES.md](BEST_PRACTICES.md)**
- 性能优化技巧
- 内存管理策略
- 省电效率指南
- 多显示器支持建议
- 错误处理模式
- 用户体验优化
- 安全性建议
- 完整检查清单

### 🔧 进阶：深入理解

**可选阅读（深入技术）：**
- [INTEGRATION_ARCHITECTURE.md](INTEGRATION_ARCHITECTURE.md) - 系统架构
- [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) - 技术实现细节
- [API_BRIDGE.md](API_BRIDGE.md) - C++/JS 桥接机制

### 🐛 遇到问题？

**[TROUBLESHOOTING.md](TROUBLESHOOTING.md)**
- 常见问题解决
- 错误诊断
- 调试技巧

---

## 🌐 Web 开发者 - 文档路径

### 🚀 第一步：快速开始（5分钟）

**[WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md) - 第1节** ⭐
- SDK 自动注入说明
- 第一个互动壁纸
- 基础 API 调用

```javascript
// 5行代码创建互动壁纸
if (window.AnyWP) {
  AnyWP.ready('我的壁纸');
  AnyWP.onClick('#button', () => {
    AnyWP.openURL('https://example.com');
  });
}
```

### 📚 第二步：完整 SDK 指南

**[WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md)** ⭐ 核心文档（688行）

**内容索引：**
```javascript
// 核心 API
AnyWP.ready(name)
AnyWP.onClick(element, callback, options)
AnyWP.openURL(url)
AnyWP.refreshBounds()
AnyWP.clearHandlers()

// 省电优化（v4.1.0）
AnyWP.onVisibilityChange(callback)  // 新增

// 调试
AnyWP.enableDebug()
AnyWP.setSPAMode(enabled)

// 事件监听
AnyWP.onMouse(callback)
AnyWP.onKeyboard(callback)

// 属性
AnyWP.version
AnyWP.dpiScale
AnyWP.screenWidth
AnyWP.screenHeight
AnyWP.interactionEnabled
```

**包含章节：**
- ✅ 核心 API 详解
- ✅ React 完整集成教程
- ✅ Vue 2/3 完整集成教程
- ✅ SPA 单页应用支持
- ✅ 调试技巧
- ✅ 性能优化
- ✅ 常见问题 FAQ

### 🌍 英文版本

**[WEB_DEVELOPER_GUIDE.md](WEB_DEVELOPER_GUIDE.md)**
- 与中文版内容相同
- 适合国际开发者

### 🔧 第三步：理解底层（可选）

**[API_BRIDGE.md](API_BRIDGE.md)** - 技术实现细节
- C++/JavaScript 桥接架构
- 完整事件传递流程
- 鼠标钩子实现
- 像素坐标系统原理
- 安全机制说明

**适合：** 想深入理解工作原理的开发者

### 📝 第四步：查看示例

**示例 HTML 文件（examples/ 目录）：**
- `test_simple.html` - 基础示例
- `test_react.html` - React 集成
- `test_vue.html` - Vue 集成
- `test_visibility.html` - 可见性 API 演示
- `test_iframe_ads.html` - 复杂场景

---

## 📊 完整文档分类表

### 🔵 Flutter 开发者专用

| 优先级 | 文档 | 行数 | 用途 |
|-------|------|------|------|
| ⭐⭐⭐ | **DEVELOPER_API_REFERENCE.md** | 468 | **必读** - 完整 Dart API |
| ⭐⭐⭐ | **API_USAGE_EXAMPLES.md** | 676 | **推荐** - 7个实用示例 |
| ⭐⭐ | **BEST_PRACTICES.md** | 632 | **重要** - 最佳实践 |
| ⭐⭐ | PACKAGE_USAGE_GUIDE_CN.md | - | 集成方式详解 |
| ⭐ | QUICK_START.md | - | 快速开始 |
| ⭐ | TROUBLESHOOTING.md | - | 故障排查 |
| 📖 | INTEGRATION_ARCHITECTURE.md | - | 系统架构（可选）|
| 📖 | TECHNICAL_NOTES.md | - | 技术细节（可选）|

**推荐阅读顺序：**
1. QUICK_START.md（5分钟）
2. DEVELOPER_API_REFERENCE.md（核心）
3. API_USAGE_EXAMPLES.md（实践）
4. BEST_PRACTICES.md（优化）

---

### 🟢 Web 开发者专用

| 优先级 | 文档 | 行数 | 用途 |
|-------|------|------|------|
| ⭐⭐⭐ | **WEB_DEVELOPER_GUIDE_CN.md** | 688 | **必读** - 完整 SDK 指南 |
| ⭐⭐ | **WEB_DEVELOPER_GUIDE.md** | 642 | 英文版本 |
| ⭐ | **API_BRIDGE.md** | 721 | 技术原理（可选）|
| 📝 | examples/*.html | - | 实际示例代码 |

**推荐阅读顺序：**
1. WEB_DEVELOPER_GUIDE_CN.md（完整教程）
2. examples/test_*.html（示例代码）
3. API_BRIDGE.md（理解原理，可选）

---

### 🟡 通用文档

| 文档 | 用途 | 受众 |
|-----|------|------|
| **README.md** | 项目首页（英文）| 所有人 |
| **README_CN.md** | 项目首页（中文）| 中文用户 |
| **docs/README.md** | 文档导航中心 | 所有人 |
| CHANGELOG_CN.md | 更新日志 | 所有人 |
| SDK_CHANGELOG.md | SDK 版本历史 | Web开发者 |

---

### 🔴 待处理文档（可能冗余）

| 文档 | 内容 | 与其他文档的关系 |
|-----|------|----------------|
| **USAGE_EXAMPLES.md** | 简单示例 | ⚠️ 与 API_USAGE_EXAMPLES.md 重叠 |
| CHEAT_SHEET_CN.md | 速查表 | ⚠️ 与 DEVELOPER_API_REFERENCE.md 部分重叠 |
| PROJECT_STRUCTURE.md | 项目结构 | ℹ️ 对贡献者有用 |
| TESTING_GUIDE.md | 测试指南 | ℹ️ 对贡献者有用 |

---

## 🎯 优化建议

### 建议 1：合并重复的示例文档

**问题：**
- `USAGE_EXAMPLES.md` (14KB, 简单示例)
- `API_USAGE_EXAMPLES.md` (20KB, 详细示例)

**解决方案：**
- ❌ 删除 `USAGE_EXAMPLES.md`
- ✅ 保留 `API_USAGE_EXAMPLES.md`（更全面）

### 建议 2：CHEAT_SHEET_CN.md 定位

**当前：** 速查表（可能与 API 参考重叠）

**建议：**
- 保留作为快速参考
- 或合并到 DEVELOPER_API_REFERENCE.md 的开头

### 建议 3：贡献者文档分离

**贡献者专用文档（不影响开发者）：**
- PROJECT_STRUCTURE.md
- TESTING_GUIDE.md
- 建议移到 `docs/development/` 子目录

---

## 📋 最终推荐文档结构

### 结构 A：按受众分类（推荐）

```
docs/
├── 📱 flutter-developers/
│   ├── QUICK_START.md                    # 快速开始
│   ├── DEVELOPER_API_REFERENCE.md        # 完整API ⭐
│   ├── API_USAGE_EXAMPLES.md             # 7个示例 ⭐
│   ├── BEST_PRACTICES.md                 # 最佳实践 ⭐
│   ├── PACKAGE_USAGE_GUIDE_CN.md         # 集成指南
│   └── TROUBLESHOOTING.md                # 故障排查
│
├── 🌐 web-developers/
│   ├── WEB_DEVELOPER_GUIDE_CN.md         # 完整指南（中文）⭐
│   ├── WEB_DEVELOPER_GUIDE.md            # 完整指南（英文）⭐
│   └── API_BRIDGE.md                     # 技术原理（可选）
│
├── 📘 general/
│   ├── README.md                         # 文档导航
│   ├── INTEGRATION_ARCHITECTURE.md       # 系统架构
│   ├── TECHNICAL_NOTES.md                # 技术细节
│   ├── CHANGELOG_CN.md                   # 更新日志
│   └── SDK_CHANGELOG.md                  # SDK版本历史
│
└── 🔧 contributors/
    ├── PROJECT_STRUCTURE.md              # 项目结构
    ├── TESTING_GUIDE.md                  # 测试指南
    └── CHEAT_SHEET_CN.md                 # 速查表
```

### 结构 B：当前扁平结构（简化）

```
docs/
├── 📱 FOR_FLUTTER_DEVELOPERS.md         # Flutter开发者索引 🆕
├── 🌐 FOR_WEB_DEVELOPERS.md             # Web开发者索引 🆕
│
├── DEVELOPER_API_REFERENCE.md            # Flutter API ⭐
├── API_USAGE_EXAMPLES.md                 # Flutter 示例 ⭐
├── BEST_PRACTICES.md                     # 最佳实践 ⭐
├── WEB_DEVELOPER_GUIDE_CN.md             # Web SDK 指南 ⭐
├── WEB_DEVELOPER_GUIDE.md                # Web SDK (EN) ⭐
│
├── API_BRIDGE.md                         # 技术实现
├── PACKAGE_USAGE_GUIDE_CN.md             # 集成指南
├── QUICK_START.md                        # 快速开始
├── TROUBLESHOOTING.md                    # 故障排查
├── CHANGELOG_CN.md                       # 更新日志
└── ... (其他文档)
```

---

## 🗑️ 建议删除的冗余文档

### 1. USAGE_EXAMPLES.md ❌
**理由：**
- 内容完全被 API_USAGE_EXAMPLES.md 包含
- 后者更详细（20KB vs 14KB）
- 避免维护两套示例

### 2. （可选）简化其他文档
待进一步分析...

---

## 🎯 实施步骤

### 立即执行：
1. ✅ 删除 SDK_API_REFERENCE.md（已完成）
2. ❓ 删除 USAGE_EXAMPLES.md？
3. ❓ 创建分类索引文档？

### 待讨论：
- 是否采用分类目录结构？
- CHEAT_SHEET_CN.md 如何处理？
- 贡献者文档是否分离？

---

**下一步：** 请确认优化方向，我将立即执行！

