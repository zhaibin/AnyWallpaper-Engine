# AnyWP Engine 编码规范

Last updated: 2026-05-28

本文档定义 AnyWP Engine 当前活跃源码的编码规范。规范适用于 `lib/`、`windows/`、`macos/`、`sdk/src/`、`scripts/`、`examples/`、`templates/` 和 `docs/`；`release/` 下的历史快照、`build/`、`.dart_tool/`、`sdk/src/dist/` 等生成产物不作为日常开发修改对象。

## 基本原则

1. 保持跨平台行为一致：Dart API、Windows C++、macOS Objective-C 和 Web SDK 的同名能力必须有一致的语义、错误返回和文档描述。
2. 优先复用现有模块边界：Windows 功能放入 `windows/modules/` 或 `windows/utils/`，macOS 功能放入 `macos/Classes/Modules/` 或 `macos/Classes/Utils/`，Web SDK 功能放入 `sdk/src/modules/` 或 `sdk/src/utils/`。
3. 只修改当前源码：不要手动同步 `release/` 历史包；发布流程需要生成快照时再处理。
4. 代码必须可诊断：新增平台能力要有清晰日志、失败路径和可复现测试或验证步骤。
5. 公共 API 必须向后兼容：破坏性变更需要版本说明、迁移说明和对应文档更新。
6. 小步提交、低耦合修改：一次变更只解决一个主题，避免把格式化、重构、功能和发布文件混在同一个改动里。

## 文件和目录

- 新增 Dart 公共接口放在 `lib/anywp_engine.dart`，保持 Flutter 插件入口集中。
- 新增 Windows 运行时功能优先放在 `windows/modules/`；通用能力、校验、日志、资源管理放在 `windows/utils/`。
- 新增 macOS 运行时功能优先放在 `macos/Classes/Modules/`；本地文件、日志、状态等工具放在 `macos/Classes/Utils/`。
- TypeScript SDK 源码只改 `sdk/src/` 下的 `.ts` 文件；`sdk/src/dist/` 由构建生成，不手工编辑。
- 根目录 `sdk/anywp_sdk.js`、`sdk/dist/`、`windows/anywp_sdk.js`、`macos/Resources/anywp_sdk.js` 属于分发产物，修改 SDK 源码后通过构建脚本同步。
- 文档新增到 `docs/`，并同步更新 `docs/README.md` 与 `docs/DOCUMENTATION_INDEX.md`。
- 示例页面放在 `examples/` 或对应平台 `Resources/`，需要说明其验证目标；不要把临时调试页面混入正式示例。

## 变更范围

- 业务功能、重构、格式化、测试修复、文档更新分别提交，除非它们是同一个可验证变更的必要组成。
- 修改公共 API 时，同一变更必须包含调用层、平台实现、类型声明、示例或文档更新。
- 修改构建脚本时，要说明影响的平台、入口命令和失败时的退出行为。
- 修改生成文件前先确认它是否由脚本产出；能由脚本生成的文件不手工改。

## 格式化和静态检查

### Dart

- 遵循 `analysis_options.yaml` 和 `flutter_lints`。
- 使用单引号、`const` 构造、`final` 局部变量，避免 `print`，使用 `debugPrint` 或平台日志。
- MethodChannel 参数解析必须做类型防御，避免把平台侧异常暴露成不可读错误。
- 回调注册和轮询计时器必须有取消入口，避免应用关闭后仍触发异步回调。
- 提交前运行：

```bash
flutter analyze
```

### TypeScript SDK

- 遵循 `sdk/src/tsconfig.json`，保持 `strict`、`noUnusedLocals`、`noUnusedParameters`、`noImplicitReturns` 通过。
- 模块使用 ES Module 语法；公共类型放在 `sdk/src/types.ts` 或 `sdk/src/types/`。
- 源码换行使用 LF；不要提交由本地编辑器产生的大范围格式化噪音。
- 与原生桥通信的消息必须使用结构化对象，至少包含稳定的 `type` 字段。
- 对浏览器 API、WebView2 bridge 和 `window.AnyWP` 的访问必须先判断存在性。
- 提交前在 `sdk/src/` 运行：

```bash
npm run typecheck
npm test
```

### C++ Windows

- 使用 C++17 可用特性时先确认 `windows/CMakeLists.txt` 支持。
- 命名空间使用 `anywp_engine`，头文件包含 include guard。
- COM/WebView2 对象优先使用 `Microsoft::WRL::ComPtr` 管理生命周期。
- 平台资源必须有明确释放路径；窗口、钩子、WebView、计时器和回调不能依赖进程退出清理。
- Win32 API 调用失败时记录 `HRESULT`、`GetLastError()` 或可读错误上下文。
- 头文件只暴露必要接口；实现细节、辅助函数和常量优先放在 `.cpp` 的匿名命名空间或私有成员中。

### Objective-C macOS

- 类名、方法名遵循现有 Cocoa/Objective-C 风格。
- 对象属性按语义使用 `strong`、`weak`、`copy`、`assign`；字符串属性默认使用 `copy`，除非有明确原因。
- 对 `NSWindow`、`WKWebView`、委托和通知注册保持成对创建/释放。
- 与 WebKit、AppKit 交互必须在主线程执行；后台回调切回主线程后再触碰 UI 对象。
- `NSError` 或异常上下文不能丢失，至少记录 domain、code 和关键路径。

### 脚本

- Windows 脚本使用 `.bat` 或 `.ps1`，macOS/Linux 脚本使用 `.sh`。
- 脚本必须能从仓库根目录运行，或者在文件开头清晰切换到脚本所在目录。
- 失败时返回非零退出码；不要静默吞掉关键命令失败。

## 命名规范

- Dart 类名使用 `PascalCase`，方法、字段、变量使用 `lowerCamelCase`，私有成员以 `_` 开头。
- TypeScript 类型、接口、类使用 `PascalCase`，函数和变量使用 `camelCase`，内部字段可沿用现有 `_internalName` 风格。
- C++ 类名使用 `PascalCase`，函数和方法使用 `PascalCase`，私有成员使用尾随下划线，例如 `shared_environment_`。
- Objective-C 类名使用 `PascalCase`，方法参数名保持可读的 Cocoa 风格。
- 日志组件名使用 `PascalCase`，并遵循 [LOGGING_STANDARDS.md](LOGGING_STANDARDS.md)。

## API 设计

- Dart 公共 API 是跨平台入口；新增能力必须先定义 Dart 层语义，再实现 Windows/macOS 平台分支。
- MethodChannel 方法名使用稳定字符串；新增方法需要在 Dart、Windows、macOS、文档和测试中保持一致。
- 返回值应优先使用明确的 `bool`、结构化 `Map`、列表或异常，不返回含糊的字符串状态。
- Web SDK 对外 API 挂载在 `window.AnyWP`，新增方法需要补充类型声明、测试和 Web 开发文档。
- 新增配置项必须定义默认值、平台差异、非法输入行为和兼容策略。
- API 名称使用动词开头表达动作，例如 `initializeWallpaper`、`stopWallpaper`、`navigateToUrl`。
- 平台不支持的能力必须显式返回可诊断错误或文档化的降级行为，不做静默 no-op。

## 并发和生命周期

- 初始化、恢复、停止、导航等生命周期操作必须考虑重复调用和乱序调用。
- 跨线程回调不能捕获裸指针后长期持有；需要检查对象仍然有效。
- 关闭流程要先停止新任务进入，再释放 WebView、窗口、hook、timer 和持久化状态。
- Explorer 重启、显示器变化、系统锁屏和多显示器切换属于核心稳定性场景，相关修改必须验证。

## 错误处理

- 输入边界在最靠近入口的位置校验：Dart API、MethodChannel handler、SDK public API 都需要防御非法参数。
- 平台调用失败必须记录错误原因；不要只返回 `false` 而没有日志。
- 异步回调要考虑对象已销毁、WebView 已释放、窗口已关闭和 Explorer 重启恢复中的竞态。
- 可恢复错误使用降级或重试；不可恢复错误要向调用层返回明确失败。
- 不吞掉异常。确实需要忽略的错误必须加简短注释说明原因。

## 日志

- Windows C++ 禁止使用 `std::cout` 作为运行时日志，使用统一 `Logger`。
- 日志消息使用英文、无 emoji，组件名使用 `PascalCase`。
- `DEBUG` 用于开发诊断，`INFO` 用于生命周期和状态变化，`WARNING` 用于可恢复异常，`ERROR` 用于操作失败。
- 日志中避免输出访问令牌、用户隐私路径中的敏感片段和大体积内容。

## 测试和验证

- Dart API 或平台桥接变化：运行 `flutter analyze`，并补充 Flutter 或平台验证步骤。
- TypeScript SDK 变化：运行 `npm run typecheck` 和 `npm test`，必要时补充 `sdk/src/__tests__/` 用例。
- Windows C++ 模块变化：优先补充 `windows/test/` 下的单元或综合测试，并运行对应 `.bat` 脚本。
- macOS 平台变化：补充手动验证步骤，至少覆盖初始化、导航、停止和多显示器相关行为。
- 文档或脚本变化：验证命令、路径和版本号是否与当前仓库一致。

## 文档

- 用户可见行为变化必须更新 `README.md`、`docs/` 对应指南或 API 参考。
- 文档写作和索引规则遵循 [DOCUMENTATION_STANDARDS.md](DOCUMENTATION_STANDARDS.md)。
- 发布相关变更遵循 [RELEASE_STANDARDS.md](RELEASE_STANDARDS.md)。
- 示例代码必须能复制运行，不使用省略号替代关键参数。

## 提交流程检查清单

提交前至少确认：

- 只修改当前任务相关文件，没有误改生成产物或历史 release 快照。
- 静态检查、单元测试或手动验证步骤已运行；不能运行时说明原因。
- 新增 API 已覆盖 Dart、Windows、macOS、SDK 类型、测试和文档中的必要部分。
- 日志、错误处理和资源释放路径完整。
- 版本、变更日志或发布说明在需要时已更新。
