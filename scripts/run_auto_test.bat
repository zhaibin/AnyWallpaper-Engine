@echo off
REM ==========================================
REM AnyWP Engine - 自动化测试脚本
REM ==========================================

echo.
echo ========================================
echo AnyWP Engine - 自动化测试
echo ========================================
echo.

cd /d "%~dp0..\example"

echo 清理旧的日志文件...
del /f /q auto_test_results.log 2>nul
del /f /q test_output.log 2>nul
del /f /q debug_run.log 2>nul

echo.
echo 编译自动化测试应用（Debug 模式）...
flutter build windows --debug --target=lib/auto_test.dart

if errorlevel 1 (
    echo.
    echo [错误] 编译失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo 启动自动化测试...
echo ========================================
echo.
echo 测试将自动进行，请勿操作桌面
echo 预计耗时: 约 2 分钟
echo.

REM 启动测试应用并捕获输出
build\windows\x64\runner\Debug\anywallpaper_engine_example.exe > auto_test_output.log 2>&1

echo.
echo ========================================
echo 测试完成
echo ========================================
echo.

REM 显示测试结果
if exist auto_test_results.log (
    echo 测试日志:
    echo.
    type auto_test_results.log
    echo.
    echo 完整日志已保存到: auto_test_results.log
) else (
    echo [警告] 未找到测试结果文件
)

echo.
echo 应用输出已保存到: auto_test_output.log
echo.

pause

