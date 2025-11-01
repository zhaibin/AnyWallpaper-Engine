# AnyWP Engine - Naming Convention

## 统一命名规范

为简化项目，所有代码和文件统一使用 **`anywp`** 命名（AnyWallpaper 的缩写）。

### ✅ 正确命名

| 类别 | 格式 | 示例 |
|------|------|------|
| **包名** | `anywp_engine` | `package:anywp_engine` |
| **文件名** | `anywp_*` | `anywp_engine_plugin.cpp`, `anywp_sdk.js` |
| **目录名** | `anywp_*` | `anywp_engine/` |
| **类名** | `AnyWP*` | `class AnyWPEngine`, `AnyWPEnginePlugin` |
| **命名空间** | `anywp_*` | `namespace anywp_engine` |
| **日志前缀** | `[AnyWP]` | `[AnyWP] Initializing...` |
| **项目显示名** | `AnyWP Engine` | 用于UI和文档标题 |

### ❌ 避免使用

| 错误格式 | 原因 |
|----------|------|
| `any_wallpaper` | 不要在 any 和 wallpaper 之间加下划线 |
| `AnyWallpaper` | 太长，使用 `AnyWP` 代替 |
| `anywallpaper` | 虽然可以，但推荐使用更短的 `anywp` |

## 项目文件夹命名

### 当前状态

项目文件夹可能是以下名称之一：
- `AnyWallpaper-Engine` (带连接符，旧命名)
- `AnyWallpaper Engine` (带空格)
- `AnyWP_Engine` (推荐)

### 推荐命名

**推荐使用**: `AnyWP_Engine`

原因：
- ✅ 与代码命名一致
- ✅ 无空格，命令行友好
- ✅ 简短清晰
- ✅ 跨平台兼容

### 如何重命名项目文件夹

⚠️ **注意**: 重命名项目文件夹后，需要更新所有绝对路径引用。

#### Windows 步骤

```powershell
# 1. 关闭 VS Code 和所有相关窗口

# 2. 重命名文件夹
cd E:\Projects\AnyWallpaper
Rename-Item "AnyWallpaper-Engine" "AnyWP_Engine"

# 3. 更新 example/lib/main.dart 中的绝对路径
# 将所有 E:\Projects\AnyWallpaper\AnyWallpaper-Engine
# 改为 E:\Projects\AnyWallpaper\AnyWP_Engine

# 4. 重新打开项目
code E:\Projects\AnyWallpaper\AnyWP_Engine
```

#### 需要更新的文件

重命名后，检查以下文件中的绝对路径：
- `example/lib/main.dart` - 测试 HTML 文件路径
- 任何使用 `file:///` 协议的路径引用

## 代码示例

### Dart 代码

```dart
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  await AnyWPEngine.initializeWallpaper(
    url: 'https://example.com',
    enableMouseTransparent: true,
  );
}
```

### C++ 代码

```cpp
#include "anywp_engine_plugin.h"

namespace anywp_engine {
  class AnyWPEnginePlugin {
    // ...
  };
}
```

### JavaScript 代码

```javascript
// 使用 AnyWP SDK
if (window.AnyWP) {
  AnyWP.ready('MyWallpaper');
}
```

## 品牌显示

在用户界面和文档中：
- **完整名称**: AnyWP Engine
- **简称**: AnyWP
- **标语**: Windows Desktop Wallpaper Engine

## 版本历史

- **v1.0.0**: 统一采用 `anywp` 命名规范
- **之前**: 混用 `anywallpaper`、`any_wallpaper` 等命名

---

**最后更新**: 2025-11-02  
**版本**: 1.0.0


