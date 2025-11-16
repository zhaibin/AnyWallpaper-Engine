# Version Management Guide

## 版本号统一管理

### 📍 版本号定义位置

AnyWP Engine 的版本号在以下 **4 个关键位置** 定义，发布前必须保持一致：

#### 1️⃣ **Dart/Flutter 层** (pubspec.yaml)
```yaml
# File: pubspec.yaml
version: 2.1.10
```
- **作用**: Flutter 插件版本
- **影响**: pub.dev 发布、依赖解析
- **必须修改**: ✅ 每次发布

#### 2️⃣ **C++ 引擎层** (anywp_engine_plugin.cpp)
```cpp
// File: windows/anywp_engine_plugin.cpp
namespace {
constexpr char kPluginVersion[] = "2.1.10";
constexpr char kSDKVersion[] = "2.1.10";  // Built-in Web SDK version
}
```
- **作用**: C++ 引擎版本 + 内置 SDK 版本
- **影响**: `AnyWPEngine.getPluginVersion()` / `AnyWPEngine.getSDKVersion()`
- **必须修改**: ✅ 每次发布

#### 3️⃣ **TypeScript SDK 源码** (windows/sdk/core/AnyWP.ts)
```typescript
// File: windows/sdk/core/AnyWP.ts
export const AnyWP: AnyWPSDK = {
  version: '2.1.10',
  // ...
};
```
- **作用**: TypeScript SDK 源码版本
- **影响**: 编译后的 `anywp_sdk.js` 文件
- **必须修改**: ✅ 每次发布（编译前）

#### 4️⃣ **TypeScript 项目配置** (windows/sdk/package.json)
```json
// File: windows/sdk/package.json
{
  "name": "anywp-sdk",
  "version": "2.1.10",
  // ...
}
```
- **作用**: npm 项目版本
- **影响**: SDK 开发、测试、文档
- **必须修改**: ✅ 每次发布

#### 5️⃣ **Cursor Rules** (.cursorrules)
```
**Version**: 2.1.10 | **Updated**: 2025-11-16
```
- **作用**: AI 编码助手识别当前版本
- **影响**: 自动化提交消息、版本检查
- **必须修改**: ✅ 每次发布

---

## 🔧 版本号修改工作流

### **手动修改步骤**

1. **更新 Dart 插件版本**
   ```bash
   # 编辑 pubspec.yaml
   version: 2.1.11
   ```

2. **更新 C++ 引擎版本**
   ```cpp
   // 编辑 windows/anywp_engine_plugin.cpp
   constexpr char kPluginVersion[] = "2.1.11";
   constexpr char kSDKVersion[] = "2.1.11";
   ```

3. **更新 TypeScript SDK 源码**
   ```typescript
   // 编辑 windows/sdk/core/AnyWP.ts
   version: '2.1.11',
   ```

4. **更新 SDK 项目配置**
   ```json
   // 编辑 windows/sdk/package.json
   "version": "2.1.11",
   ```

5. **更新 Cursor Rules**
   ```
   # 编辑 .cursorrules (底部)
   **Version**: 2.1.11 | **Updated**: YYYY-MM-DD
   ```

6. **重新编译 SDK**
   ```bash
   cd windows/sdk
   npm run build
   ```

7. **验证版本一致性**
   ```bash
   .\scripts\check_version_consistency.ps1
   ```

---

## ✅ 版本验证 API

### **Dart API**

```dart
// 获取引擎版本
final engineVersion = await AnyWPEngine.getPluginVersion();
print('Engine Version: $engineVersion');  // Output: 2.1.10

// 获取内置 SDK 版本
final sdkVersion = await AnyWPEngine.getSDKVersion();
print('SDK Version: $sdkVersion');  // Output: 2.1.10

// 验证版本一致性
assert(engineVersion == sdkVersion, 'Version mismatch!');
```

### **JavaScript API**

```javascript
// 在 Web 壁纸中检查 SDK 版本
console.log('SDK Version:', window.AnyWP.version);  // Output: 2.1.10

// 运行时检查版本
if (window.AnyWP.version !== '2.1.10') {
  console.warn('SDK version mismatch!');
}
```

---

## 🚨 常见问题

### **Q1: 为什么有两个版本号 (kPluginVersion 和 kSDKVersion)?**

**A**: 为了支持独立的 SDK 更新：
- **`kPluginVersion`**: C++ 引擎核心版本
- **`kSDKVersion`**: 内置的 JavaScript SDK 版本
- **当前策略**: 两者保持一致（同步更新）
- **未来策略**: 可能独立更新（SDK 版本 ≤ 引擎版本）

### **Q2: 忘记更新某个位置的版本号会怎样?**

**A**: 
- ❌ `pubspec.yaml` 未更新 → pub.dev 发布失败
- ❌ `anywp_engine_plugin.cpp` 未更新 → API 返回错误版本号
- ❌ `AnyWP.ts` 未更新 → 前端显示旧版本
- ❌ `package.json` 未更新 → npm 测试/文档版本错误

### **Q3: 如何自动化版本检查?**

**A**: 使用内置脚本：
```bash
# 检查版本一致性
.\scripts\check_version_consistency.ps1

# 发布流程会自动检查
.\scripts\release.bat  # 内部调用版本检查
```

---

## 📦 发布检查清单

- [ ] `pubspec.yaml` → `version: X.Y.Z`
- [ ] `windows/anywp_engine_plugin.cpp` → `kPluginVersion[]` + `kSDKVersion[]`
- [ ] `windows/sdk/core/AnyWP.ts` → `version: 'X.Y.Z'`
- [ ] `windows/sdk/package.json` → `"version": "X.Y.Z"`
- [ ] `.cursorrules` (底部) → `**Version**: X.Y.Z`
- [ ] 运行 `npm run build` (在 `windows/sdk/`)
- [ ] 运行 `.\scripts\check_version_consistency.ps1`
- [ ] 验证 `AnyWPEngine.getPluginVersion()` 和 `AnyWPEngine.getSDKVersion()` 返回正确版本

---

## 🎯 最佳实践

1. **始终先更新版本号，再开始开发**
2. **使用 `.\scripts\release.bat` 自动化发布** (包含版本检查)
3. **在 PR 中包含版本号变更**
4. **每次提交后运行 `check_version_consistency.ps1`**
5. **在 CHANGELOG_CN.md 中记录版本变更**

---

**维护者**: AnyWP Engine Team  
**最后更新**: 2025-11-16  
**当前版本**: 2.1.10

