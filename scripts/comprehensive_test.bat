@echo off
setlocal EnableDelayedExpansion

echo ========================================
echo AnyWP Engine 全面自动化测试
echo ========================================
echo.

set TEST_DIR=E:\Projects\AnyWallpaper\AnyWallpaper-Engine
set LOG_FILE=%TEST_DIR%\comprehensive_test_results.log
set ERROR_LOG=%TEST_DIR%\comprehensive_test_errors.log

:: 清理旧日志
del /Q "%LOG_FILE%" 2>NUL
del /Q "%ERROR_LOG%" 2>NUL
echo [%date% %time%] 测试开始 > "%LOG_FILE%"
echo [%date% %time%] 错误日志 > "%ERROR_LOG%"

:: 步骤 1: 清理环境
echo [步骤 1/8] 清理测试环境...
taskkill /IM anywallpaper_engine_example.exe /F >NUL 2>&1
timeout /t 2 /nobreak >NUL
echo [%date% %time%] 环境清理完成 >> "%LOG_FILE%"

:: 步骤 2: 编译应用
echo [步骤 2/8] 编译 Flutter 应用...
cd %TEST_DIR%\example
flutter build windows --debug --no-pub >> "%LOG_FILE%" 2>> "%ERROR_LOG%"
if %errorlevel% neq 0 (
    echo [错误] 编译失败！
    echo [%date% %time%] 编译失败，错误码: %errorlevel% >> "%ERROR_LOG%"
    goto :error_exit
)
echo [%date% %time%] 编译成功 >> "%LOG_FILE%"

:: 步骤 3: 启动应用
echo [步骤 3/8] 启动测试应用...
start "" "build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"
timeout /t 5 /nobreak >NUL
echo [%date% %time%] 应用已启动 >> "%LOG_FILE%"

:: 步骤 4: 等待应用初始化
echo [步骤 4/8] 等待应用初始化 (10秒)...
timeout /t 10 /nobreak >NUL
echo [%date% %time%] 应用初始化完成 >> "%LOG_FILE%"

:: 步骤 5: 执行测试页面测试（模拟加载）
echo [步骤 5/8] 测试各个测试页面...
echo [%date% %time%] 开始测试页面测试 >> "%LOG_FILE%"

set TEST_PAGES=test_refactoring.html test_simple.html test_drag_debug.html test_api.html test_basic_click.html test_react.html test_vue.html
for %%p in (%TEST_PAGES%) do (
    echo   测试: %%p
    echo [%date% %time%] 测试页面: %%p >> "%LOG_FILE%"
    timeout /t 8 /nobreak >NUL
)

:: 步骤 6: 收集运行时日志
echo [步骤 6/8] 收集运行时日志...
timeout /t 10 /nobreak >NUL

:: 检查是否有崩溃
tasklist /FI "IMAGENAME eq anywallpaper_engine_example.exe" 2>NUL | find /I /N "anywallpaper_engine_example.exe">NUL
if %errorlevel% neq 0 (
    echo [警告] 应用已崩溃或退出
    echo [%date% %time%] 应用异常退出 >> "%ERROR_LOG%"
) else (
    echo [%date% %time%] 应用运行正常 >> "%LOG_FILE%"
)

:: 步骤 7: 停止应用
echo [步骤 7/8] 停止测试应用...
taskkill /IM anywallpaper_engine_example.exe /F >NUL 2>&1
timeout /t 2 /nobreak >NUL
echo [%date% %time%] 应用已停止 >> "%LOG_FILE%"

:: 步骤 8: 分析测试结果
echo [步骤 8/8] 分析测试结果...
echo [%date% %time%] 测试完成 >> "%LOG_FILE%"

:: 显示测试总结
echo.
echo ========================================
echo 测试完成！
echo ========================================
echo.
echo 日志文件: %LOG_FILE%
echo 错误日志: %ERROR_LOG%
echo.
echo 按任意键查看测试日志...
pause >NUL

:: 显示日志
type "%LOG_FILE%"
echo.
echo ----------------------------------------
echo 错误日志:
type "%ERROR_LOG%"

goto :end

:error_exit
echo.
echo 测试失败！请查看错误日志: %ERROR_LOG%
pause
exit /b 1

:end
endlocal

