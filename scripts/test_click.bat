@echo off
REM Test Click Events
echo ========================================
echo  点击测试 - 请按以下步骤操作
echo ========================================
echo.
echo 1. 应用将启动
echo 2. 请点击壁纸上的红色边框区域
echo 3. 观察控制台输出
echo.
echo 期望看到的日志：
echo   [AnyWP] AnyWP:click event received
echo   [AnyWP] _handleClick called with
echo   [AnyWP] HIT! Calling callback
echo.
echo 如果没有看到这些日志，说明鼠标钩子未触发
echo ========================================
echo.
pause
echo 启动应用...
cd ..\example
build\windows\x64\runner\Debug\anywallpaper_engine_example.exe

