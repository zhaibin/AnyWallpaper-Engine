# 远程桌面 WebView 消失问题诊断

## 问题
用户报告：远程登录后，桌面的 WebView 消失。

## 诊断版本
已添加详细的窗口验证日志以诊断问题。

## 测试准备
✅ 应用已启动并运行
✅ 详细日志已启用

## 测试步骤

### 1. 确认初始状态
- 确保壁纸已激活并正常显示
- 确认应用正在运行

### 2. 进行远程桌面测试
- 从另一台电脑远程桌面连接，或
- 使用 `mstsc /v:localhost` 连接到本机

### 3. 在远程桌面中
- 工作一段时间
- 可以锁屏/解锁测试

### 4. 回到本地
- 断开远程桌面
- 回到本地物理控制台
- **观察壁纸是否显示**

## 关键日志检查点

恢复时应该看到类似这样的详细日志：

### 单显示器模式
```
[PowerSaving] ========== RESUMING WALLPAPER ==========
[PowerSaving] Verifying wallpaper window state...
[PowerSaving] Single-monitor mode detected
[PowerSaving] WebView window: 0x00140052
[PowerSaving] IsWindow: 1, IsVisible: 1
[PowerSaving] Expected parent (WorkerW): 0x0001010A
[PowerSaving] Actual parent: 0x0001010A
[PowerSaving] WorkerW valid: 1
[PowerSaving] [OK] Window valid, parent relationship OK
```

### 多显示器模式
```
[PowerSaving] ========== RESUMING WALLPAPER ==========
[PowerSaving] Verifying wallpaper window state...
[PowerSaving] Multi-monitor mode detected (1 instances)
[PowerSaving] Checking monitor 0
[PowerSaving] WebView window: 0x00140052
[PowerSaving] IsWindow: 1, IsVisible: 1
[PowerSaving] [OK] Monitor 0 window valid
```

## 可能的问题场景

### 场景 A：窗口完全消失
```
IsWindow: 0
```
**原因**：窗口句柄无效，窗口已被系统销毁
**解决**：需要完全重建窗口

### 场景 B：窗口存在但不可见
```
IsWindow: 1, IsVisible: 0
```
**原因**：窗口被隐藏
**解决**：调用 ShowWindow 显示窗口

### 场景 C：父窗口关系断开
```
IsWindow: 1, IsVisible: 1
Actual parent: 0x00000000 (或其他值)
WorkerW valid: 0
```
**原因**：WorkerW 被系统重建，父子关系断开
**解决**：重新嵌入窗口到新的 WorkerW

### 场景 D：一切正常但看不到
```
IsWindow: 1, IsVisible: 1
Actual parent = Expected parent
WorkerW valid: 1
```
**原因**：可能是 Z-order 问题或其他原因
**解决**：需要进一步诊断

## 已添加的改进

### 1. 单显示器模式详细日志
- 窗口句柄
- IsWindow 状态
- IsVisible 状态
- 期望/实际父窗口
- WorkerW 有效性
- 验证结果

### 2. 多显示器模式详细日志
- 实例数量
- 每个显示器的窗口句柄
- 每个显示器的窗口状态
- 验证结果

### 3. 清晰的状态标记
- [OK] 验证通过
- [X] 验证失败

## 下一步

根据日志结果决定修复方案：
1. 窗口消失 → 自动重建机制已存在
2. 窗口不可见 → 添加 ShowWindow 调用
3. 父窗口断开 → 改进 WorkerW 检测和重新嵌入
4. Z-order 问题 → 添加 Z-order 修复逻辑

---

**请进行远程桌面测试，并提供日志中 "Verifying wallpaper window state" 部分的完整输出！**

