# AnyWP Engine 文档规范

Last updated: 2026-05-28

本文档定义 AnyWP Engine 文档的组织、写作、同步和审核要求。目标是让用户文档、开发者文档、API 文档和发布文档始终能指导真实操作。

## 适用范围

- 根目录文档：`README.md`、`QUICK_INTEGRATION.md`、`CHANGELOG_CN.md`。
- 当前文档目录：`docs/*.md`。
- SDK 和平台内嵌文档：`sdk/src/README.md`、`windows/include/anywp_engine/README.md`、`windows/test/README.md` 等。
- 不包括 `release/` 下历史快照中的复制文档；历史包只在发版生成或回溯验证时处理。

## 文档分类

| 类别 | 主要文件 | 维护要求 |
|------|----------|----------|
| 入门与集成 | `README.md`, `QUICK_START.md`, `FOR_FLUTTER_DEVELOPERS.md`, `PACKAGE_USAGE_GUIDE_CN.md` | 面向首次使用者，步骤必须可执行 |
| API 与示例 | `DEVELOPER_API_REFERENCE.md`, `API_USAGE_EXAMPLES.md`, `WEB_DEVELOPER_GUIDE*.md` | 与 Dart API、TypeScript 类型、平台实现同步 |
| 架构与协议 | `ARCHITECTURE_DESIGN.md`, `MULTIPLATFORM_ARCHITECTURE.md`, `MESSAGE_PROTOCOL.md`, `API_BRIDGE.md` | 描述稳定结构和跨模块边界 |
| 测试与排障 | `TESTING_GUIDE.md`, `TROUBLESHOOTING.md`, `RUNTIME_ISSUES.md`, `LOGGING_STANDARDS.md` | 包含命令、预期结果和失败处理 |
| 发布与维护 | `RELEASE_GUIDE.md`, `RELEASE_STANDARDS.md`, `VERSION_MANAGEMENT.md`, `SCRIPTS_REFERENCE.md` | 说明版本、产物、验证、提交和 tag |

## 写作规则

- 标题用清晰名词或动词短语；同一文档内层级不要跳跃。
- 命令必须标明运行目录。默认从仓库根目录运行，例外时在命令前写明 `cd`。
- 示例代码必须完整到可复制运行；不要使用 `...` 省略关键参数、错误处理或导入。
- 文档中出现版本号、文件名、方法名、脚本名时使用反引号。
- 用户操作步骤使用编号列表；并列说明使用短项目符号。
- 警告和限制必须说明影响、触发条件和推荐处理方式。
- 中文文档默认使用中文说明；日志消息、代码注释示例和 API 名称保持英文。

## API 文档同步

新增或修改 Dart API 时必须检查：

- `lib/anywp_engine.dart` 的 Dartdoc。
- `docs/DEVELOPER_API_REFERENCE.md`。
- `docs/FOR_FLUTTER_DEVELOPERS.md`。
- `docs/API_USAGE_EXAMPLES.md`。
- `README.md` 中的核心示例。

新增或修改 Web SDK API 时必须检查：

- `sdk/src/types.ts` 或相关类型文件。
- `docs/WEB_DEVELOPER_GUIDE_CN.md`。
- `docs/WEB_DEVELOPER_GUIDE.md`。
- `docs/API_USAGE_EXAMPLES.md`。
- `examples/` 中可运行示例。

新增或修改平台行为时必须检查：

- `docs/PLATFORM_COMPARISON.md`。
- `docs/CROSS_PLATFORM_INTEGRATION.md`。
- 对应平台指南，例如 `MACOS_DEVELOPER_GUIDE.md` 或 Windows 预编译集成文档。

## 文档索引规则

- 新增 `docs/*.md` 后必须更新 `docs/README.md` 和 `docs/DOCUMENTATION_INDEX.md`。
- `docs/DOCUMENTATION_INDEX.md` 的 `Current docs count` 必须与 `find docs -maxdepth 1 -name '*.md' | wc -l` 一致。
- 不把 `release/` 下的历史文档列入当前文档索引。
- 文档移动或删除时，要同步清理所有入口链接和交叉引用。

## 版本和日期

- 规范类、流程类和发布类文档顶部保留 `Last updated: YYYY-MM-DD` 或等价中文日期。
- 版本号只能来自对应源文件：引擎版本来自 `pubspec.yaml`，SDK 版本来自 `sdk/src/package.json`。
- 不在普通示例中写死未来版本号；必须写版本号时同步检查 `VERSION_MANAGEMENT.md` 和 `RELEASE_STANDARDS.md`。

## 命令和平台标注

- Windows 命令使用 PowerShell 或 `.bat` 风格时明确写出，例如 `.\scripts\pre_release_check.bat`。
- Windows 测试、构建和发布验证命令必须标注“在 Windows 环境中执行”；可补充说明本地默认使用 Parallels Desktop Windows VM，其他开发者可使用真实 Windows 机器、Windows VM 或 CI Windows runner。
- macOS/Linux 命令使用 POSIX shell 风格，例如 `bash scripts/release_macos.sh`。
- 平台特定步骤必须标注适用平台，避免 Windows 用户执行 macOS 命令或反之。
- 命令有破坏性影响时必须说明，例如清理构建目录、覆盖发布包、创建 tag。

## 文档验证清单

提交文档前至少检查：

- 链接目标存在，大小写和路径正确。
- 命令能从文档声明的目录运行。
- 示例代码包含必要 import、依赖和参数。
- API 名称、参数名和返回值与源码一致。
- 文档索引和文档数量已更新。
- 发布、版本、变更日志相关文档没有互相矛盾。

## 审核要求

- 用户文档审核重点是可执行性：新用户能否按步骤完成集成。
- API 文档审核重点是一致性：源码、类型、示例和说明是否匹配。
- 发布文档审核重点是完整性：版本、产物、验证、提交、tag 和回滚是否覆盖。
- 架构文档审核重点是边界：模块职责、数据流和平台差异是否清楚。
