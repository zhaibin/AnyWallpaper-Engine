@echo off
REM 快速启动全面自动化测试
REM 
REM 使用方法:
REM   run_comprehensive_test.bat           # 标准测试 (无锁屏)
REM   run_comprehensive_test.bat --lock    # 包含锁屏测试

cd /d "%~dp0"

if "%1"=="--lock" (
    call comprehensive_auto_test.bat --with-lock-screen
) else (
    call comprehensive_auto_test.bat
)

REM 测试完成后自动分析结果
echo.
echo ========================================
echo 正在分析测试结果...
echo ========================================
powershell -ExecutionPolicy Bypass -File "analyze_test_results.ps1"

echo.
echo 按任意键生成 HTML 报告...
pause >nul
powershell -ExecutionPolicy Bypass -File "analyze_test_results.ps1" -GenerateHTML

exit /b 0

