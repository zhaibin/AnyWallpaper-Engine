# 存储隔离与数据清理机制

## 概述

AnyWP Engine 提供了**应用级存储隔离机制**，确保多个应用使用同一引擎时互不干扰，删除应用时不留残留数据。

## 存储路径

### 默认路径
```
%LOCALAPPDATA%\AnyWPEngine\Default\state.json
```

### 自定义应用路径
```
%LOCALAPPDATA%\AnyWPEngine\[YourAppName]\state.json
```

**示例**：
- `C:\Users\YourName\AppData\Local\AnyWPEngine\MyApp\state.json`
- `C:\Users\YourName\AppData\Local\AnyWPEngine\AnotherApp\state.json`

## 配置应用名称

### Dart/Flutter

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // ⚠️ 重要：在初始化壁纸前设置应用名称
  await AnyWPEngine.setApplicationName('MyAwesomeApp');
  
  runApp(MyApp());
}
```

### 查看存储路径

```dart
final storagePath = await AnyWPEngine.getStoragePath();
print('数据存储路径: $storagePath');
```

## 应用隔离优势

###  ✅ 多应用独立存储

不同应用使用不同的存储目录，互不干扰：

```
%LOCALAPPDATA%\AnyWPEngine\
  ├── App1\
  │   └── state.json  (App1 的状态)
  ├── App2\
  │   └── state.json  (App2 的状态)
  └── Default\
      └── state.json  (未配置应用名称的默认存储)
```

### ✅ 无残留卸载

用户卸载应用时，只需删除对应的应用目录：

**Windows 用户**：
```powershell
# 手动清理
Remove-Item -Recurse "$env:LOCALAPPDATA\AnyWPEngine\MyApp"
```

**集成到卸载程序**：
```bat
@echo off
REM uninstall.bat
echo Cleaning up application data...
rmdir /s /q "%LOCALAPPDATA%\AnyWPEngine\MyApp"
echo Done!
```

### ✅ 易于备份和迁移

用户可以轻松备份或迁移配置：

```powershell
# 备份
Copy-Item "$env:LOCALAPPDATA\AnyWPEngine\MyApp" "D:\Backup\MyApp_Config" -Recurse

# 恢复
Copy-Item "D:\Backup\MyApp_Config" "$env:LOCALAPPDATA\AnyWPEngine\MyApp" -Recurse
```

## 数据清理最佳实践

### 1. 为你的应用设置唯一名称

```dart
// ❌ 不推荐：使用通用名称
await AnyWPEngine.setApplicationName('Wallpaper');

// ✅ 推荐：使用应用唯一标识
await AnyWPEngine.setApplicationName('MyCompany_MyApp');
```

### 2. 提供数据清理功能

在应用设置中提供"清理数据"选项：

```dart
ElevatedButton(
  onPressed: () async {
    final confirm = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: Text('清理数据'),
        content: Text('这将删除所有保存的壁纸位置和设置。是否继续？'),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: Text('取消'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, true),
            child: Text('确认'),
          ),
        ],
      ),
    );
    
    if (confirm == true) {
      await AnyWPEngine.clearState();
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('数据已清理')),
      );
    }
  },
  child: Text('清理数据'),
)
```

### 3. 在卸载程序中清理数据

**NSIS 安装程序示例**：

```nsis
Function un.onUninstSuccess
  MessageBox MB_YESNO "是否删除应用数据？" IDYES DeleteData IDNO Done
  DeleteData:
    RMDir /r "$LOCALAPPDATA\AnyWPEngine\MyApp"
  Done:
FunctionEnd
```

**Inno Setup 示例**：

```pascal
[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  DataDir: string;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    if MsgBox('是否删除应用数据？', mbConfirmation, MB_YESNO) = IDYES then
    begin
      DataDir := ExpandConstant('{localappdata}\AnyWPEngine\MyApp');
      DelTree(DataDir, True, True, True);
    end;
  end;
end;
```

## API 参考

### setApplicationName

设置应用名称以隔离存储。

```dart
Future<bool> setApplicationName(String name)
```

**参数**：
- `name`: 应用标识符（字母数字、下划线、连字符，空格会转换为下划线）

**返回**：成功返回 `true`

**注意**：
- 应在任何壁纸初始化前调用
- 切换应用名称会清空内存缓存

### getStoragePath

获取当前应用的存储路径。

```dart
Future<String> getStoragePath()
```

**返回**：完整存储路径

**用途**：
- 文档展示
- 调试验证
- 用户支持

### clearState

清理当前应用的所有状态数据。

```dart
Future<bool> clearState()
```

**返回**：成功返回 `true`

**注意**：此操作不可撤销

## 迁移指南

### 从旧版本（使用注册表）迁移

旧版本使用注册表存储：`HKEY_CURRENT_USER\Software\AnyWPEngine\State`

**迁移步骤**：

1. **删除旧的注册表数据**（可选）：
   ```powershell
   Remove-Item -Path "HKCU:\Software\AnyWPEngine" -Recurse -Force
   ```

2. **更新代码**：
   ```dart
   void main() async {
     WidgetsFlutterBinding.ensureInitialized();
     
     // 新版本：设置应用名称
     await AnyWPEngine.setApplicationName('MyApp');
     
     runApp(MyApp());
   }
   ```

3. **测试验证**：
   ```dart
   final path = await AnyWPEngine.getStoragePath();
   print('新存储路径: $path');
   ```

## 常见问题

### Q: 不设置应用名称会怎样？

A: 默认使用 `"Default"` 作为应用名称，存储在 `%LOCALAPPDATA%\AnyWPEngine\Default\`。

### Q: 应用名称可以包含中文吗？

A: 不建议。应用名称会被清理为仅包含字母数字、下划线和连字符。

### Q: 多个应用可以共享存储吗？

A: 可以，只需设置相同的应用名称。但不推荐，建议每个应用使用独立标识符。

### Q: 如何完全卸载所有数据？

A: 删除整个 `%LOCALAPPDATA%\AnyWPEngine` 目录即可清理所有应用的数据。

```powershell
Remove-Item -Recurse -Force "$env:LOCALAPPDATA\AnyWPEngine"
```

## 总结

✅ **始终为应用设置唯一名称**  
✅ **在卸载程序中提供数据清理选项**  
✅ **向用户明确说明数据存储位置**  
✅ **提供应用内数据清理功能**  

这样可以确保：
- 多应用互不干扰
- 卸载干净无残留
- 用户体验友好

