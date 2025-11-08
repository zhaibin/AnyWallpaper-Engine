# C++ 模块化重构 - 文档维护清单

> 本次重构：将 `anywp_engine_plugin.cpp` (4000+ 行) 拆分为多个模块
> 生成时间：2025-11-07

---

## ✅ 已完成更新的文档

### 1. `.cursorrules` ✅
**更新内容**:
- ✅ 新增"文件组织"中的 `modules/` 和 `utils/` 目录结构
- ✅ 新增"编码规范 - C++"中的模块化设计原则
- ✅ 新增"代码架构"章节，说明重构目标和模块分类
- ✅ 更新版本为 v1.3.2-dev

### 2. `docs/PROJECT_STRUCTURE.md` ✅
**更新内容**:
- ✅ 更新 windows/ 目录结构图
- ✅ 详细列出所有新模块（3 个工具类 + 5 个功能模块）
- ✅ 新增"C++ 模块化架构"章节
- ✅ 说明设计原则和重构目标

### 3. `docs/TECHNICAL_NOTES.md` ✅
**更新内容**:
- ✅ 更新"Plugin Structure"为模块化层次图
- ✅ 新增"Modular Architecture Benefits"对比
- ✅ 新增"Modular Components Details"章节（含所有模块详细说明）
- ✅ 新增"Build System"配置说明
- ✅ 更新"Future Enhancements"计划

### 4. `docs/FOR_FLUTTER_DEVELOPERS.md` ✅
**更新内容**:
- ✅ 新增 v1.3.2 重要更新章节
- ✅ 说明重构对 Flutter 开发者完全透明
- ✅ 列出所有新增模块
- ✅ 强调向后兼容性

---

## 🔴 需要更新的文档（高优先级）

### 1. `docs/INTEGRATION_ARCHITECTURE.md` 🔴
**当前状态**: 包含 C++ 实现的架构图，但未反映模块化设计

**需要更新**:
- [ ] 更新"C++ Native Implementation"部分的架构图
- [ ] 添加模块化层次（utils/ 和 modules/）
- [ ] 说明模块之间的交互关系
- [ ] 更新集成工作流程（如涉及编译）

**影响范围**: 中等（架构理解相关）

**建议内容**:
```
C++ Native Implementation (v1.3.2+ Modular)
├─ Core Plugin (anywp_engine_plugin.cpp)
│  └─ 负责 WorkerW 挂载、WebView2 初始化、方法桥接
├─ Utility Classes (utils/)
│  ├─ StatePersistence - 状态持久化
│  ├─ URLValidator - URL 验证
│  └─ Logger - 日志记录
└─ Functional Modules (modules/)
   ├─ IframeDetector - iframe 检测
   ├─ SDKBridge - SDK 桥接
   ├─ MouseHookManager - 鼠标钩子
   ├─ MonitorManager - 多显示器管理
   └─ PowerManager - 省电管理
```

---

### 2. `docs/API_BRIDGE.md` 🔴
**当前状态**: 说明 C++ 和 JavaScript 的桥接实现，但可能引用旧的实现方式

**需要更新**:
- [ ] 检查是否有直接引用 `anywp_engine_plugin.cpp` 的实现细节
- [ ] 如果提到 SDK 注入和消息处理，更新为 `SDKBridge` 模块
- [ ] 如果提到 iframe 检测，更新为 `IframeDetector` 模块
- [ ] 添加模块化架构的说明（可选）

**影响范围**: 中等（技术实现理解相关）

**建议内容**:
```
### SDK Injection (v1.3.2+)

现在由专门的 `SDKBridge` 模块处理：
- 负责 SDK 文件注入
- 消息监听器设置
- 消息分发和处理

参考: windows/modules/sdk_bridge.cpp
```

---

### 3. `docs/README.md` 🟡
**当前状态**: 文档索引，可能需要提及重构

**需要更新**:
- [ ] 在"Technical & Architecture"部分添加重构说明
- [ ] 可以添加"v1.3.2 更新"标记

**影响范围**: 低（导航文档）

**建议内容**:
```markdown
### Technical & Architecture 🔧

| 文档 | 用途 |
|-----|------|
| [INTEGRATION_ARCHITECTURE.md](INTEGRATION_ARCHITECTURE.md) | 系统架构 ⭐ 已更新 v1.3.2 模块化 |
| [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) | 技术细节 ⭐ 已更新 v1.3.2 模块化 |
| [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) | 项目结构 ⭐ 已更新 v1.3.2 模块化 |
```

---

## 🟢 可选更新的文档（低优先级）

### 1. `docs/AUTO_RELEASE_GUIDE.md` 🟢
**当前状态**: 自动发布流程指南

**需要检查**:
- [ ] 是否提到编译步骤（如提到，需说明新模块已自动包含在 CMakeLists.txt）
- [ ] 是否需要在发布说明中提及模块化重构

**影响范围**: 低（发布流程，构建系统已自动处理）

---

### 2. `docs/PRECOMPILED_DLL_INTEGRATION.md` 🟢
**当前状态**: 预编译 DLL 集成指南

**需要检查**:
- [ ] 确认提到的文件列表是否需要更新（modules/ 和 utils/ 的 .cpp/.h 文件）
- [ ] 如果有"What's Inside"章节，可以提及模块化架构

**影响范围**: 低（用户侧，DLL 已包含所有模块）

**建议内容**:
```markdown
## What's Inside the Package?

### DLL 文件
- `bin/anywp_engine_plugin.dll` - 编译后的插件（包含所有模块）

### 源代码（可选自行编译）
- `windows/anywp_engine_plugin.cpp/h` - 核心插件
- `windows/modules/` - 功能模块（v1.3.2+）
- `windows/utils/` - 工具类（v1.3.2+）
```

---

### 3. `docs/TESTING_GUIDE.md` 🟢
**当前状态**: 测试指南

**需要检查**:
- [ ] 测试步骤是否仍然有效（功能未变，应该无需更新）
- [ ] 可以添加"模块化测试"章节（未来）

**影响范围**: 极低（外部行为未改变）

---

### 4. `docs/CHEAT_SHEET_CN.md` 🟢
**当前状态**: 速查表

**需要检查**:
- [ ] API 使用方式未改变，无需更新
- [ ] 可在"版本说明"部分添加 v1.3.2 标记（可选）

**影响范围**: 极低

---

## ❌ 无需更新的文档

以下文档与外部 API 相关，重构未改变其内容：

- ✅ `docs/DEVELOPER_API_REFERENCE.md` - Dart API 参考（外部接口未变）
- ✅ `docs/API_USAGE_EXAMPLES.md` - Dart 示例代码（外部接口未变）
- ✅ `docs/BEST_PRACTICES.md` - 最佳实践（外部接口未变）
- ✅ `docs/QUICK_START.md` - 快速开始（外部接口未变）
- ✅ `docs/PACKAGE_USAGE_GUIDE_CN.md` - 包使用指南（外部接口未变）
- ✅ `docs/WEB_DEVELOPER_GUIDE_CN.md` - Web 开发指南（JS API 未变）
- ✅ `docs/WEB_DEVELOPER_GUIDE.md` - Web 开发指南（JS API 未变）
- ✅ `docs/FOR_WEB_DEVELOPERS.md` - Web 开发者导航（JS API 未变）
- ✅ `docs/TROUBLESHOOTING.md` - 故障排查（问题类型未变）
- ✅ `docs/SDK_CHANGELOG.md` - Web SDK 版本历史（JS SDK 未变）
- ✅ `docs/SDK_*` - Web SDK 相关文档（JS SDK 未变）
- ✅ `docs/RELEASE_GUIDE.md` - 发布指南（流程未变）
- ✅ `docs/RELEASE_TEMPLATE.md` - 发布模板（格式未变）

---

## 📊 优先级总结

### 🔴 高优先级（建议立即更新）
1. `docs/INTEGRATION_ARCHITECTURE.md` - 架构图和实现说明
2. `docs/API_BRIDGE.md` - 桥接实现细节

### 🟡 中优先级（建议更新）
3. `docs/README.md` - 文档索引标记

### 🟢 低优先级（可选更新）
4. `docs/AUTO_RELEASE_GUIDE.md` - 检查构建流程说明
5. `docs/PRECOMPILED_DLL_INTEGRATION.md` - 添加模块化说明
6. `docs/TESTING_GUIDE.md` - 可添加模块测试章节
7. `docs/CHEAT_SHEET_CN.md` - 可添加版本标记

---

## 📝 更新模板

### 标准化更新说明

在每个更新的文档中，建议添加以下标记：

```markdown
---

**版本更新 (v1.3.2)**:
- 本文档已更新以反映 C++ 模块化重构
- C++ 实现已从单文件拆分为多个模块
- 外部 API 行为完全保持不变

---
```

### 模块引用格式

提到新模块时，使用统一格式：

```markdown
**模块名称** (`windows/modules/module_name.*` 或 `windows/utils/utility_name.*`)
- **职责**: 简短说明
- **关键方法**: `Method1()`, `Method2()`
- **使用场景**: 何时被调用
```

---

## ✅ 执行建议

### 第一阶段（立即）
1. 更新 `docs/INTEGRATION_ARCHITECTURE.md`
2. 更新 `docs/API_BRIDGE.md`
3. 更新 `docs/README.md`

### 第二阶段（本周内）
4. 检查 `docs/PRECOMPILED_DLL_INTEGRATION.md`
5. 检查 `docs/AUTO_RELEASE_GUIDE.md`

### 第三阶段（下次发布前）
6. 可选更新其他低优先级文档
7. 全面审查所有文档一致性

---

**生成时间**: 2025-11-07  
**重构版本**: v1.3.2-dev  
**文档状态**: 4 个已更新，2-3 个待更新，其余无需更新

