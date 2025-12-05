@echo off
REM 可见性修复测试脚本
REM 测试锁屏时可见性回调是否正确触发

echo ============================================
echo AnyWP Engine - 可见性修复测试
echo ============================================
echo.

REM 设置日志文件
set LOG_FILE=test_logs\visibility_fix_test_%DATE:~0,4%%DATE:~5,2%%DATE:~8,2%_%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%.log
set LOG_FILE=%LOG_FILE: =0%

REM 创建日志目录
if not exist test_logs mkdir test_logs

echo [%TIME%] 测试开始 > "%LOG_FILE%"
echo. >> "%LOG_FILE%"

REM 清理旧构建
echo [%TIME%] 清理旧构建... | tee -a "%LOG_FILE%"
cd example
call flutter clean >nul 2>&1
cd ..

REM 重新构建
echo [%TIME%] 构建应用... | tee -a "%LOG_FILE%"
cd example
call flutter build windows --debug >> "%LOG_FILE%" 2>&1
if %ERRORLEVEL% neq 0 (
    echo [错误] 构建失败！ | tee -a "%LOG_FILE%"
    cd ..
    exit /b 1
)
cd ..

echo [%TIME%] 构建成功 | tee -a "%LOG_FILE%"
echo.
echo ============================================
echo 测试说明
echo ============================================
echo.
echo 1. 应用将自动启动
echo 2. 点击 "🎯 Fullscreen Pause" 加载测试页面
echo 3. 按 Win+L 锁屏
echo 4. 解锁返回桌面
echo 5. 观察计数器是否变化：
echo    - 暂停次数应该 +1
echo    - 恢复次数应该 +1
echo.
echo 6. 按任意键关闭应用并查看日志
echo.
echo ============================================
echo.

REM 启动应用
echo [%TIME%] 启动应用... | tee -a "%LOG_FILE%"
start "AnyWP Test" example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe

echo.
echo 应用已启动，请进行测试...
echo.
pause

REM 停止应用
echo [%TIME%] 停止应用... | tee -a "%LOG_FILE%"
taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1

echo.
echo ============================================
echo 测试完成
echo ============================================
echo.
echo 日志文件: %LOG_FILE%
echo.

REM 检查关键日志
echo 检查关键日志... | tee -a "%LOG_FILE%"
findstr /C:"Pausing wallpaper" /C:"Resuming wallpaper" /C:"_notifyVisibilityChange" "%LOG_FILE%" >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [成功] 找到可见性回调相关日志 | tee -a "%LOG_FILE%"
) else (
    echo [警告] 未找到预期的日志输出 | tee -a "%LOG_FILE%"
)

echo.
echo 按任意键退出...
pause >nul

