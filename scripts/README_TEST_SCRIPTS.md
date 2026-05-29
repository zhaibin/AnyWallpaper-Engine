# AnyWP Engine 自动化测试脚本

Windows 自动化测试必须在可用的 **Windows 环境** 中运行。本地开发默认使用 Parallels Desktop 中的 Windows 虚拟机；其他开发者可以使用真实 Windows 机器、Windows 虚拟机或 CI Windows runner。macOS 主机侧只能做脚本文本检查和通用 SDK/文档验证。

目前仅保留两条核心流程：

1. **`scripts\test_full.bat`**  
   - 构建 Debug 版本并依次打开 4 个演示页面
   - 记录控制台输出、内存/CPU 采样等日志  
   - 日志输出位于 `test_logs\`

2. **`scripts\analyze.ps1`**  
   - 解析 `test_full` 生成的 CSV/LOG  
   - 计算最大值、平均值、增长率等指标  
   - 支持 `-GenerateHtml` 生成性能报告

```bash
# 在 Windows 环境中运行完整测试
scripts\test_full.bat

# 在 Windows 环境中解析结果（建议使用 pwsh 7.5.4+）
pwsh ./scripts/analyze.ps1 -GenerateHtml
```

> ⚠️ 所有 PowerShell 脚本默认使用 `pwsh`，若命令提示未找到，请先执行 `scripts\setup.bat` 安装依赖。

## 默认测试页面（约 50 秒）

| 顺序 | 页面 | 验证重点 |
| ---- | ---- | -------- |
| 1 | `test_visibility.html` | 省电模式、暂停 / 恢复 |
| 2 | `test_api.html` | Dart ↔ JS API 覆盖 |
| 3 | `test_basic_click.html` | 鼠标事件与透明穿透 |
| 4 | `test_carousel_control.html` | 轮播控制与 Flutter 消息同步 |

## 日志与报告

- `test_logs\test_full_*.log` — 主流程输出  
- `test_logs\memory_*.csv` / `cpu_*.csv` — 1 秒采样数据  
- `test_logs\app_output_*.log` — 运行时控制台日志  
- `test_logs\performance_report_*.html` — 可选性能报告（执行 `analyze.ps1 -GenerateHtml` 后生成）

> 建议在发版前运行一次完整测试，并使用 `analyze.ps1` 检查性能回归。
