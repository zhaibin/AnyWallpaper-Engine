# AnyWP Engine 自动化测试脚本使用指南

## 📋 脚本概述

本项目包含三个核心测试脚本，用于全面自动化测试 AnyWP Engine：

### 1. `comprehensive_auto_test.bat` - 主测试脚本
**功能**：
- ✅ 自动编译项目 (Debug 模式)
- ✅ 复制 SDK 文件到正确位置
- ✅ 实时监控内存占用 (每秒采样，CSV 格式)
- ✅ 实时监控 CPU 使用率 (每秒采样，CSV 格式)
- ✅ 运行所有测试用例（8 个，约 95 秒）
- ✅ 可选锁屏性能测试
- ✅ 自动收集和分析日志
- ✅ 生成详细测试报告

**使用方法**：
```bash
# 标准测试 (无锁屏)
.\scripts\comprehensive_auto_test.bat

# 包含锁屏测试
.\scripts\comprehensive_auto_test.bat --with-lock-screen
```

**测试时长**：约 95 秒 (8 个测试用例)

**输出文件**：
- `test_logs\comprehensive_test_YYYYMMDD_HHMMSS.log` - 主测试报告
- `test_logs\memory_YYYYMMDD_HHMMSS.csv` - 内存监控数据
- `test_logs\cpu_YYYYMMDD_HHMMSS.csv` - CPU 监控数据
- `test_logs\build_YYYYMMDD_HHMMSS.log` - 编译日志
- `test_logs\app_output_YYYYMMDD_HHMMSS.log` - 应用输出日志

### 2. `analyze_test_results.ps1` - 结果分析脚本
**功能**：
- ✅ 解析内存和 CPU 性能数据
- ✅ 计算性能指标（最大值、平均值、增长率）
- ✅ 检测内存泄漏趋势
- ✅ 性能评分系统 (0-100 分)
- ✅ 日志完整性验证
- ✅ 生成 HTML 可视化报告

**使用方法**：
```powershell
# 命令行分析
.\scripts\analyze_test_results.ps1

# 生成 HTML 报告
.\scripts\analyze_test_results.ps1 -GenerateHTML
```

**性能评分标准**：
| 指标 | 优秀 | 良好 | 需改进 |
|------|------|------|--------|
| 最大内存 | < 250 MB | < 300 MB | > 300 MB |
| 内存增长率 | < 5% | < 10% | > 10% |
| 平均 CPU | < 10% | < 15% | > 15% |
| 测试时长 | > 90s | > 50s | < 50s |

**评分规则**：
- 100 分：所有指标优秀
- 80-99 分：良好，有小问题
- 60-79 分：需改进
- < 60 分：存在严重问题

### 3. `run_comprehensive_test.bat` - 一键启动脚本
**功能**：
- ✅ 自动运行主测试
- ✅ 测试完成后自动分析结果
- ✅ 提示生成 HTML 报告

**使用方法**：
```bash
# 标准测试
.\scripts\run_comprehensive_test.bat

# 包含锁屏测试
.\scripts\run_comprehensive_test.bat --lock
```

---

## 🧪 测试用例列表

| 序号 | 测试页面 | 描述 | 时长 | 测试重点 |
|------|---------|------|------|---------|
| 1 | test_refactoring.html | 🔧 重构测试 | 20s | 模块化架构、SDKBridge、消息通信 |
| 2 | test_simple.html | 🎨 基础壁纸 | 10s | 渐变、时钟、SDK 版本 |
| 3 | test_api.html | ⚙️ API 测试 | 15s | 完整 API 覆盖 |
| 4 | test_basic_click.html | 👆 点击检测 | 10s | 鼠标事件、透明穿透 |
| 5 | test_visibility.html | 👁️ 可见性 | 10s | 省电模式、暂停/恢复 |
| 6 | test_react.html | ⚛️ React SPA | 10s | SPA 路由、状态管理 |
| 7 | test_vue.html | 💚 Vue SPA | 10s | Vue 集成 |
| 8 | test_iframe_ads.html | 📺 iframe 映射 | 10s | 坐标转换、广告检测 |

**总计**：8 个测试用例，约 95 秒

**已排除**：
- ~~test_drag_debug.html~~ - 拖拽调试（需要手动交互，不适合自动化）

---

## 📊 监控指标

### 1. 内存监控
**采样频率**：1 秒/次  
**监控指标**：
- **WorkingSet (MB)** - 工作集（物理内存）
- **PrivateBytes (MB)** - 私有字节
- **VirtualMemory (MB)** - 虚拟内存

**警告阈值**：
- ⚠️ 工作集 > 250 MB
- ❌ 工作集 > 300 MB
- ❌ 内存增长率 > 10%

### 2. CPU 监控
**采样频率**：1 秒/次  
**监控指标**：
- **CPU (%)** - CPU 使用率
- **Threads** - 线程数
- **Handles** - 句柄数

**警告阈值**：
- ⚠️ 平均 CPU > 10%
- ❌ 平均 CPU > 15%

### 3. 日志监控
**检查项目**：
- ✅ SDK 加载日志
- ✅ SDK 注入成功日志
- ✅ 测试开始/完成日志
- ❌ 错误日志统计

---

## 🔍 日志完整性检查

### 必须包含的日志：
```
[AnyWP] [API] SDK loaded from: windows\anywp_sdk.js (size: XXXXX bytes)
[AnyWP] [API] SDK executed successfully on current page
[开始自动测试] 或 [测试.*开始]
[测试完成] 或 [所有测试完成]
```

### 不应出现的日志：
```
ERROR
Failed
失败
错误
```

---

## 🔐 锁屏测试流程

如果启用锁屏测试 (`--with-lock-screen`)：

1. 测试脚本运行到步骤 7 时自动触发锁屏
2. 系统倒计时 5 秒后锁屏
3. **用户手动操作**：等待 10 秒后输入密码解锁
4. 脚本继续运行，内存和 CPU 监控持续记录
5. 用于验证锁屏/解锁对性能的影响

**注意事项**：
- 锁屏期间监控继续运行
- 解锁后立即恢复正常测试
- 报告中会标注锁屏测试完成

---

## 📁 文件结构

```
AnyWallpaper-Engine/
├── scripts/
│   ├── comprehensive_auto_test.bat      # 主测试脚本
│   ├── analyze_test_results.ps1         # 结果分析脚本
│   ├── run_comprehensive_test.bat       # 一键启动脚本
│   └── README_TEST_SCRIPTS.md           # 本文档
├── test_logs/                           # 测试日志目录 (自动创建)
│   ├── comprehensive_test_*.log         # 测试报告
│   ├── memory_*.csv                     # 内存数据
│   ├── cpu_*.csv                        # CPU 数据
│   ├── build_*.log                      # 编译日志
│   ├── app_output_*.log                 # 应用日志
│   ├── monitor_memory.ps1               # 内存监控脚本 (临时)
│   ├── monitor_cpu.ps1                  # CPU 监控脚本 (临时)
│   └── performance_report_*.html        # HTML 报告
└── example/
    ├── lib/
    │   └── auto_test.dart                # 自动化测试 Dart 代码
    └── auto_test_output.log              # 自动化测试日志
```

---

## 🚀 快速开始

### 第一次使用

1. **运行完整测试**：
```bash
cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine
.\scripts\run_comprehensive_test.bat
```

2. **等待测试完成** (约 2 分钟)

3. **查看自动分析结果**（控制台输出）

4. **按任意键生成 HTML 报告**

5. **浏览器自动打开报告**

### 日常使用

**快速测试**：
```bash
.\scripts\run_comprehensive_test.bat
```

**包含锁屏测试**：
```bash
.\scripts\run_comprehensive_test.bat --lock
```

**仅分析上次结果**：
```bash
.\scripts\analyze_test_results.ps1 -GenerateHTML
```

---

## 📈 报告示例

### 控制台输出
```
========================================
AnyWP Engine - 测试结果分析工具
========================================

📂 分析文件:
  - 内存日志: memory_20251108_143022.csv
  - CPU日志: cpu_20251108_143022.csv
  - 测试报告: comprehensive_test_20251108_143022.log

[1/5] 分析内存数据...
✅ 内存分析完成
  - 最大工作集: 234.5 MB
  - 平均工作集: 198.3 MB
  - 内存增长率: 3.2%

[2/5] 分析 CPU 数据...
✅ CPU 分析完成
  - 最大 CPU: 12.5%
  - 平均 CPU: 5.8%
  - 最大线程数: 42

[3/5] 计算性能评分...
✅ 性能评分: 🎉 95/100
  所有指标正常！

[4/5] 检查日志完整性...
✅ 日志完整性检查通过

[5/5] 生成 HTML 报告...
✅ HTML 报告已生成: test_logs\performance_report_20251108_143530.html

========================================
分析完成！
========================================
```

### HTML 报告
- 包含性能评分
- 内存/CPU 统计卡片
- 问题诊断列表
- 日志完整性结果
- 测试文件链接

---

## ⚠️ 注意事项

1. **测试前确保**：
   - 无其他 `anywallpaper_engine_example.exe` 进程运行
   - 有足够磁盘空间存储日志 (约 10 MB)
   - 关闭其他高 CPU 占用程序（避免干扰）

2. **锁屏测试**：
   - 需要用户手动输入密码解锁
   - 锁屏时间约 10 秒
   - 不建议在生产环境使用

3. **日志清理**：
   - 旧日志文件不会自动删除
   - 建议定期清理 `test_logs/` 目录
   - 保留最近 5-10 次测试日志即可

4. **性能基准**：
   - 首次运行可能略慢（WebView2 初始化）
   - 建议多次测试取平均值
   - 不同硬件配置结果会有差异

---

## 🐛 故障排除

### Q: 编译失败
**A**: 检查 `test_logs\build_*.log`，确保 Flutter 环境正常

### Q: 应用启动失败
**A**: 
- 检查是否已安装 WebView2 Runtime
- 运行 `.\scripts\setup_webview2.bat`

### Q: 性能评分低
**A**: 
1. 查看 HTML 报告中的"问题诊断"
2. 检查内存增长曲线 (Excel 打开 CSV)
3. 检查应用日志中的错误

### Q: 监控数据为空
**A**: 
- 确保应用成功启动
- 检查 PowerShell 执行策略
- 手动运行 `monitor_memory.ps1` 测试

---

## 📚 相关文档

- **开发者API参考**: `docs/DEVELOPER_API_REFERENCE.md`
- **测试指南**: `docs/TESTING_GUIDE.md`
- **故障排除**: `docs/TROUBLESHOOTING.md`
- **Cursor规则**: `.cursorrules`

---

## 🔄 更新日志

### v1.3.2 (2025-11-08)
- ✅ 移除过时和重复的测试脚本
- ✅ 优化测试流程和文档
- ✅ 更新测试用例列表（排除拖拽调试）

### v1.0.0 (2025-11-08)
- ✅ 初始版本
- ✅ 内存和 CPU 监控
- ✅ 自动化测试集成
- ✅ HTML 报告生成
- ✅ 锁屏测试支持

---

**作者**: AI Assistant  
**维护**: AnyWP Engine Team  
**版本**: 1.3.2  
**最后更新**: 2025-11-08
