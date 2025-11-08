@echo off
REM ========================================
REM AnyWP Engine - 全面自动化测试脚本
REM 版本: 1.0.0
REM 日期: 2025-11-08
REM ========================================
REM 
REM 功能:
REM - 内存占用监控 (每秒采样)
REM - CPU 使用率监控
REM - WebView 事件响应测试
REM - 日志完整性验证
REM - 稳定性测试 (长时间运行)
REM - 锁屏后性能测试 (可选)
REM - 自动生成详细报告
REM
REM 使用方法:
REM   comprehensive_auto_test.bat [--with-lock-screen]
REM
REM ========================================

setlocal enabledelayedexpansion

REM 配置参数
set "PROJECT_ROOT=%~dp0.."
set "EXAMPLE_DIR=%PROJECT_ROOT%\example"
set "BUILD_DIR=%EXAMPLE_DIR%\build\windows\x64\runner\Debug"
set "LOG_DIR=%PROJECT_ROOT%\test_logs"
set "TIMESTAMP=%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
set "TIMESTAMP=%TIMESTAMP: =0%"
set "TEST_REPORT=%LOG_DIR%\comprehensive_test_%TIMESTAMP%.log"
set "MEMORY_LOG=%LOG_DIR%\memory_%TIMESTAMP%.csv"
set "CPU_LOG=%LOG_DIR%\cpu_%TIMESTAMP%.csv"

REM 测试参数
set "TEST_WITH_LOCK_SCREEN=0"
if "%1"=="--with-lock-screen" set "TEST_WITH_LOCK_SCREEN=1"

REM 创建日志目录
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

echo ========================================
echo AnyWP Engine 全面自动化测试
echo ========================================
echo.
echo 测试时间: %date% %time%
echo 项目目录: %PROJECT_ROOT%
echo 日志目录: %LOG_DIR%
echo.
echo 测试配置:
echo - 内存监控: 启用 (1秒采样)
echo - CPU监控: 启用 (1秒采样)
echo - 性能分析: 启用
echo - 日志收集: 启用
echo - 锁屏测试: %TEST_WITH_LOCK_SCREEN% (0=禁用, 1=启用)
echo.
echo ========================================
echo.

REM 初始化报告
echo AnyWP Engine 全面自动化测试报告 > "%TEST_REPORT%"
echo ======================================== >> "%TEST_REPORT%"
echo 测试时间: %date% %time% >> "%TEST_REPORT%"
echo 测试类型: 全面自动化测试 (除拖拽外) >> "%TEST_REPORT%"
echo ======================================== >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

REM 步骤 1: 清理旧进程和日志
echo [步骤 1/10] 清理环境...
echo [步骤 1/10] 清理环境 >> "%TEST_REPORT%"
taskkill /F /IM anywallpaper_engine_example.exe 2>nul
timeout /t 2 /nobreak >nul
echo - 旧进程已清理 >> "%TEST_REPORT%"
echo 完成。
echo.

REM 步骤 2: 编译项目
echo [步骤 2/10] 编译项目 (Debug模式)...
echo [步骤 2/10] 编译项目 >> "%TEST_REPORT%"
cd "%EXAMPLE_DIR%"
call flutter clean >nul 2>&1
call flutter build windows --debug --no-pub > "%LOG_DIR%\build_%TIMESTAMP%.log" 2>&1

if errorlevel 1 (
    echo ❌ 编译失败！查看日志: %LOG_DIR%\build_%TIMESTAMP%.log
    echo ❌ 编译失败 >> "%TEST_REPORT%"
    pause
    exit /b 1
)
echo ✅ 编译成功
echo ✅ 编译成功 >> "%TEST_REPORT%"
echo.

REM 步骤 3: 复制 SDK 文件
echo [步骤 3/10] 复制 SDK 文件...
echo [步骤 3/10] 复制 SDK 文件 >> "%TEST_REPORT%"
if not exist "%BUILD_DIR%\data\flutter_assets\windows\" mkdir "%BUILD_DIR%\data\flutter_assets\windows\"
copy /Y "%PROJECT_ROOT%\windows\anywp_sdk.js" "%BUILD_DIR%\data\flutter_assets\windows\" >nul 2>&1
if errorlevel 1 (
    echo ⚠️ SDK 复制失败，但继续测试
    echo ⚠️ SDK 复制失败 >> "%TEST_REPORT%"
) else (
    echo ✅ SDK 文件已复制
    echo ✅ SDK 文件已复制 >> "%TEST_REPORT%"
)
echo.

REM 步骤 4: 启动性能监控
echo [步骤 4/10] 启动性能监控...
echo [步骤 4/10] 启动性能监控 >> "%TEST_REPORT%"

REM 创建内存监控 PowerShell 脚本
echo $processName = "anywallpaper_engine_example" > "%LOG_DIR%\monitor_memory.ps1"
echo $logFile = "%MEMORY_LOG%" >> "%LOG_DIR%\monitor_memory.ps1"
echo "Timestamp,WorkingSet(MB),PrivateBytes(MB),VirtualMemory(MB)" ^| Out-File $logFile >> "%LOG_DIR%\monitor_memory.ps1"
echo while ($true) { >> "%LOG_DIR%\monitor_memory.ps1"
echo     $proc = Get-Process -Name $processName -ErrorAction SilentlyContinue >> "%LOG_DIR%\monitor_memory.ps1"
echo     if ($proc) { >> "%LOG_DIR%\monitor_memory.ps1"
echo         $ws = [math]::Round($proc.WorkingSet64 / 1MB, 2) >> "%LOG_DIR%\monitor_memory.ps1"
echo         $pb = [math]::Round($proc.PrivateMemorySize64 / 1MB, 2) >> "%LOG_DIR%\monitor_memory.ps1"
echo         $vm = [math]::Round($proc.VirtualMemorySize64 / 1MB, 2) >> "%LOG_DIR%\monitor_memory.ps1"
echo         $ts = Get-Date -Format "HH:mm:ss" >> "%LOG_DIR%\monitor_memory.ps1"
echo         "$ts,$ws,$pb,$vm" ^| Out-File $logFile -Append >> "%LOG_DIR%\monitor_memory.ps1"
echo     } >> "%LOG_DIR%\monitor_memory.ps1"
echo     Start-Sleep -Seconds 1 >> "%LOG_DIR%\monitor_memory.ps1"
echo } >> "%LOG_DIR%\monitor_memory.ps1"

REM 创建 CPU 监控 PowerShell 脚本
echo $processName = "anywallpaper_engine_example" > "%LOG_DIR%\monitor_cpu.ps1"
echo $logFile = "%CPU_LOG%" >> "%LOG_DIR%\monitor_cpu.ps1"
echo "Timestamp,CPU(%%),Threads,Handles" ^| Out-File $logFile >> "%LOG_DIR%\monitor_cpu.ps1"
echo while ($true) { >> "%LOG_DIR%\monitor_cpu.ps1"
echo     $proc = Get-Process -Name $processName -ErrorAction SilentlyContinue >> "%LOG_DIR%\monitor_cpu.ps1"
echo     if ($proc) { >> "%LOG_DIR%\monitor_cpu.ps1"
echo         $cpu = [math]::Round($proc.CPU, 2) >> "%LOG_DIR%\monitor_cpu.ps1"
echo         $threads = $proc.Threads.Count >> "%LOG_DIR%\monitor_cpu.ps1"
echo         $handles = $proc.HandleCount >> "%LOG_DIR%\monitor_cpu.ps1"
echo         $ts = Get-Date -Format "HH:mm:ss" >> "%LOG_DIR%\monitor_cpu.ps1"
echo         "$ts,$cpu,$threads,$handles" ^| Out-File $logFile -Append >> "%LOG_DIR%\monitor_cpu.ps1"
echo     } >> "%LOG_DIR%\monitor_cpu.ps1"
echo     Start-Sleep -Seconds 1 >> "%LOG_DIR%\monitor_cpu.ps1"
echo } >> "%LOG_DIR%\monitor_cpu.ps1"

REM 启动监控进程
start /B powershell -WindowStyle Hidden -File "%LOG_DIR%\monitor_memory.ps1"
start /B powershell -WindowStyle Hidden -File "%LOG_DIR%\monitor_cpu.ps1"

echo ✅ 性能监控已启动 (内存 + CPU)
echo ✅ 性能监控已启动 >> "%TEST_REPORT%"
echo - 内存日志: %MEMORY_LOG% >> "%TEST_REPORT%"
echo - CPU日志: %CPU_LOG% >> "%TEST_REPORT%"
echo.

REM 步骤 5: 启动测试应用
echo [步骤 5/10] 启动自动化测试应用...
echo [步骤 5/10] 启动自动化测试应用 >> "%TEST_REPORT%"
cd "%BUILD_DIR%"
start /B "" "anywallpaper_engine_example.exe" --target="lib/auto_test.dart" > "%LOG_DIR%\app_output_%TIMESTAMP%.log" 2>&1

REM 等待应用启动
timeout /t 5 /nobreak >nul

REM 检查进程是否启动
tasklist /FI "IMAGENAME eq anywallpaper_engine_example.exe" 2>nul | find /I "anywallpaper_engine_example.exe" >nul
if errorlevel 1 (
    echo ❌ 应用启动失败！
    echo ❌ 应用启动失败 >> "%TEST_REPORT%"
    goto cleanup
)

echo ✅ 应用启动成功 (PID: 
for /f "tokens=2" %%i in ('tasklist /FI "IMAGENAME eq anywallpaper_engine_example.exe" /NH 2^>nul') do set "APP_PID=%%i"
echo %APP_PID%)
echo ✅ 应用启动成功 (PID: %APP_PID%) >> "%TEST_REPORT%"
echo.

REM 步骤 6: 测试运行监控 (约 100 秒)
echo [步骤 6/10] 测试运行中 (预计 100 秒)...
echo [步骤 6/10] 测试运行监控 >> "%TEST_REPORT%"
echo.
echo 进度: [          ] 0%%
set /a "TOTAL_SECONDS=100"
set /a "CHECK_INTERVAL=5"
set /a "ELAPSED=0"

:test_loop
timeout /t %CHECK_INTERVAL% /nobreak >nul
set /a "ELAPSED+=CHECK_INTERVAL"
set /a "PERCENT=ELAPSED*100/TOTAL_SECONDS"
set /a "BARS=PERCENT/10"

REM 绘制进度条
set "PROGRESS="
for /L %%i in (1,1,10) do (
    if %%i LEQ !BARS! (
        set "PROGRESS=!PROGRESS!█"
    ) else (
        set "PROGRESS=!PROGRESS! "
    )
)
echo 进度: [!PROGRESS!] !PERCENT!%%

REM 检查进程是否仍在运行
tasklist /FI "IMAGENAME eq anywallpaper_engine_example.exe" 2>nul | find /I "anywallpaper_engine_example.exe" >nul
if errorlevel 1 (
    echo.
    echo ℹ️ 应用已提前退出 (运行时间: !ELAPSED! 秒)
    echo ℹ️ 应用已提前退出 (运行时间: !ELAPSED! 秒) >> "%TEST_REPORT%"
    goto test_complete
)

if !ELAPSED! LSS %TOTAL_SECONDS% goto test_loop

:test_complete
echo.
echo ✅ 测试运行完成
echo ✅ 测试运行完成 >> "%TEST_REPORT%"
echo.

REM 步骤 7: 锁屏测试 (可选)
if "%TEST_WITH_LOCK_SCREEN%"=="1" (
    echo [步骤 7/10] 锁屏性能测试...
    echo [步骤 7/10] 锁屏性能测试 >> "%TEST_REPORT%"
    echo.
    echo ⚠️ 系统将在 5 秒后锁屏...
    echo 请在锁屏后等待 10 秒，然后输入密码解锁
    timeout /t 5 /nobreak
    
    REM 锁屏
    rundll32.exe user32.dll,LockWorkStation
    
    REM 等待解锁
    echo 等待解锁...
    timeout /t 15 /nobreak >nul
    
    echo ✅ 锁屏测试完成
    echo ✅ 锁屏测试完成 >> "%TEST_REPORT%"
    echo.
) else (
    echo [步骤 7/10] 跳过锁屏测试 (未启用)
    echo [步骤 7/10] 跳过锁屏测试 >> "%TEST_REPORT%"
    echo.
)

REM 步骤 8: 停止应用和监控
echo [步骤 8/10] 停止应用和监控...
echo [步骤 8/10] 停止应用和监控 >> "%TEST_REPORT%"
taskkill /F /IM anywallpaper_engine_example.exe 2>nul
taskkill /F /FI "WINDOWTITLE eq monitor_memory.ps1*" 2>nul
taskkill /F /FI "WINDOWTITLE eq monitor_cpu.ps1*" 2>nul
timeout /t 2 /nobreak >nul
echo ✅ 应用和监控已停止
echo ✅ 应用和监控已停止 >> "%TEST_REPORT%"
echo.

REM 步骤 9: 分析性能数据
echo [步骤 9/10] 分析性能数据...
echo [步骤 9/10] 分析性能数据 >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

REM 使用 PowerShell 分析内存数据
powershell -Command "$data = Import-Csv '%MEMORY_LOG%'; $maxWS = ($data | Measure-Object -Property 'WorkingSet(MB)' -Maximum).Maximum; $avgWS = [math]::Round(($data | Measure-Object -Property 'WorkingSet(MB)' -Average).Average, 2); $maxPB = ($data | Measure-Object -Property 'PrivateBytes(MB)' -Maximum).Maximum; Write-Output \"内存占用分析:\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"- 最大工作集: $maxWS MB\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"- 平均工作集: $avgWS MB\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"- 最大私有字节: $maxPB MB\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"\" | Out-File '%TEST_REPORT%' -Append"

REM 使用 PowerShell 分析 CPU 数据
powershell -Command "$data = Import-Csv '%CPU_LOG%'; $maxCPU = ($data | Measure-Object -Property 'CPU(%%)' -Maximum).Maximum; $avgCPU = [math]::Round(($data | Measure-Object -Property 'CPU(%%)' -Average).Average, 2); $maxThreads = ($data | Measure-Object -Property 'Threads' -Maximum).Maximum; Write-Output \"CPU 使用率分析:\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"- 最大 CPU: $maxCPU %%\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"- 平均 CPU: $avgCPU %%\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"- 最大线程数: $maxThreads\" | Out-File '%TEST_REPORT%' -Append; Write-Output \"\" | Out-File '%TEST_REPORT%' -Append"

echo ✅ 性能数据分析完成
echo.

REM 步骤 10: 收集和分析日志
echo [步骤 10/10] 收集和分析日志...
echo [步骤 10/10] 收集和分析日志 >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

REM 检查自动化测试日志
if exist "%EXAMPLE_DIR%\auto_test_output.log" (
    echo 自动化测试日志摘要: >> "%TEST_REPORT%"
    findstr /C:"测试完成" /C:"错误" /C:"失败" /C:"成功" "%EXAMPLE_DIR%\auto_test_output.log" >> "%TEST_REPORT%" 2>nul
    echo. >> "%TEST_REPORT%"
)

REM 检查应用输出日志
if exist "%LOG_DIR%\app_output_%TIMESTAMP%.log" (
    echo 应用输出日志 (最后 20 行): >> "%TEST_REPORT%"
    powershell -Command "Get-Content '%LOG_DIR%\app_output_%TIMESTAMP%.log' -Tail 20" >> "%TEST_REPORT%" 2>nul
    echo. >> "%TEST_REPORT%"
)

echo ✅ 日志收集完成
echo.

REM 生成最终报告
echo.
echo ========================================
echo 测试完成！
echo ========================================
echo.
echo 📊 测试报告: %TEST_REPORT%
echo 📈 内存日志: %MEMORY_LOG%
echo 📈 CPU日志: %CPU_LOG%
echo.
echo 查看详细报告请运行:
echo   notepad "%TEST_REPORT%"
echo.

REM 添加测试总结
echo ======================================== >> "%TEST_REPORT%"
echo 测试总结 >> "%TEST_REPORT%"
echo ======================================== >> "%TEST_REPORT%"
echo 测试完成时间: %date% %time% >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

echo 测试文件: >> "%TEST_REPORT%"
echo - 测试报告: %TEST_REPORT% >> "%TEST_REPORT%"
echo - 内存日志: %MEMORY_LOG% >> "%TEST_REPORT%"
echo - CPU日志: %CPU_LOG% >> "%TEST_REPORT%"
echo - 编译日志: %LOG_DIR%\build_%TIMESTAMP%.log >> "%TEST_REPORT%"
echo - 应用日志: %LOG_DIR%\app_output_%TIMESTAMP%.log >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

echo 下一步操作: >> "%TEST_REPORT%"
echo 1. 查看内存曲线: Excel 打开 %MEMORY_LOG% >> "%TEST_REPORT%"
echo 2. 查看 CPU 曲线: Excel 打开 %CPU_LOG% >> "%TEST_REPORT%"
echo 3. 检查应用日志: notepad %LOG_DIR%\app_output_%TIMESTAMP%.log >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

:cleanup
REM 清理监控进程
taskkill /F /FI "WINDOWTITLE eq monitor_memory.ps1*" 2>nul
taskkill /F /FI "WINDOWTITLE eq monitor_cpu.ps1*" 2>nul

echo 按任意键退出...
pause >nul

endlocal
exit /b 0

