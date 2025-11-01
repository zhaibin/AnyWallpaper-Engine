# 项目文件夹命名说明

## 📁 关于项目文件夹名称

### 当前状况

项目文件夹可能使用以下名称：
- `AnyWallpaper-Engine` (带连接符)
- `AnyWallpaper Engine` (带空格)  
- `AnyWP_Engine` (推荐 ✅)

### 为什么有不同的名称？

1. **历史原因**: 项目最初可能使用了 `AnyWallpaper-Engine` 
2. **跨平台考虑**: Windows 文件系统中连接符和空格都合法
3. **统一命名**: 代码内部已统一使用 `anywp` 作为缩写

### 推荐做法

#### ✅ 推荐使用

```
AnyWP_Engine/
```

**原因:**
- 与代码命名一致 (`anywp_engine`)
- 无连接符，符合项目规范
- 使用下划线，跨平台兼容性好
- 避免空格带来的命令行问题

#### ❌ 不推荐

```
AnyWallpaper-Engine/    # 带连接符
AnyWallpaper Engine/    # 带空格
any-wallpaper-engine/   # 不符合命名规范
```

## 🔄 如何重命名项目文件夹

### 方法 1: 简单重命名（推荐）

```powershell
# 1. 关闭所有编辑器和终端
# 2. 重命名文件夹
cd E:\Projects\AnyWallpaper
Rename-Item "AnyWallpaper-Engine" -NewName "AnyWP_Engine"

# 3. 进入新文件夹
cd AnyWP_Engine

# 4. 清理并重新构建
flutter clean
flutter pub get
cd example
flutter build windows --debug
```

### 方法 2: 克隆到新位置

```powershell
# 如果已经使用 Git
cd E:\Projects\AnyWallpaper
git clone AnyWallpaper-Engine AnyWP_Engine
cd AnyWP_Engine

# 清理并构建
flutter clean
flutter pub get
```

## ⚠️ 重命名注意事项

### 需要更新的地方

1. **IDE 项目配置**
   - VS Code: 关闭并重新打开文件夹
   - Android Studio: Invalidate Caches / Restart

2. **路径引用**
   - 检查 `example/lib/main.dart` 中的测试文件路径
   - 更新任何硬编码的绝对路径

3. **构建缓存**
   ```bash
   flutter clean
   cd example
   flutter clean
   ```

4. **Git 配置** (如果使用)
   - 检查 `.git/config` 中的路径
   - 更新远程仓库 URL（如果包含路径）

### 不需要更新的地方

✅ 以下内容会自动适配：
- Flutter 包名 (`anywp_engine`)
- 所有源代码文件
- CMake 构建配置
- pubspec.yaml 配置

## 🎯 最佳实践

### 1. 新项目

如果重新开始，使用：
```bash
flutter create anywp_engine_example
```

### 2. 现有项目

保持当前文件夹名也可以正常工作，因为：
- 文件夹名不影响包名
- 代码中使用的是包名 (`anywp_engine`)
- 只是为了统一和美观

### 3. 团队协作

如果团队中其他人使用不同的文件夹名：
- 在文档中说明推荐名称
- 使用相对路径而非绝对路径
- 避免在代码中硬编码文件夹名

## 📝 总结

| 方面 | 推荐 | 说明 |
|------|------|------|
| **文件夹名** | `AnyWP_Engine` | 与代码命名一致 |
| **包名** | `anywp_engine` | 已统一 ✅ |
| **类名** | `AnyWPEngine` | 已统一 ✅ |
| **命名空间** | `anywp_engine` | 已统一 ✅ |

**结论**: 虽然文件夹名称灵活，但为了保持一致性，建议使用 `AnyWP_Engine`。如果当前使用其他名称，也不影响项目运行，可以选择是否重命名。

---

**最后更新**: 2025-11-02  
**版本**: 1.0.0

