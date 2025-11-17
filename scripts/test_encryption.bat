@echo off
REM ========================================
REM AnyWP Engine - 加密解密功能测试脚本
REM ========================================

echo.
echo ========================================
echo AnyWP Engine - 加密解密功能测试
echo ========================================
echo.

REM 设置日志文件
set LOG_FILE=test_logs\test_encryption_%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log
set LOG_FILE=%LOG_FILE: =0%

echo [1/4] 清理旧的构建... > %LOG_FILE%
echo [1/4] 清理旧的构建...
cd example
call flutter clean >> ..\%LOG_FILE% 2>&1
if errorlevel 1 (
    echo ❌ 清理失败
    exit /b 1
)
echo ✅ 清理完成
echo.

echo [2/4] 重新构建 Release 版本... >> ..\%LOG_FILE%
echo [2/4] 重新构建 Release 版本...
echo    这将确保最新的 C++ 代码被编译
call flutter build windows --release >> ..\%LOG_FILE% 2>&1
if errorlevel 1 (
    echo ❌ 构建失败
    echo    查看日志: %LOG_FILE%
    exit /b 1
)
echo ✅ 构建成功
echo.

echo [3/4] 检查编译产物... >> ..\%LOG_FILE%
echo [3/4] 检查编译产物...
if not exist "build\windows\x64\runner\Release\anywp_engine_plugin.dll" (
    echo ❌ DLL 文件未生成
    exit /b 1
)
echo ✅ DLL 文件已生成: build\windows\x64\runner\Release\anywp_engine_plugin.dll
echo.

echo [4/4] 运行应用程序测试... >> ..\%LOG_FILE%
echo [4/4] 运行应用程序测试...
echo.
echo ========================================
echo 🚀 应用程序已启动
echo ========================================
echo.
echo 请在应用程序中：
echo 1. 切换到 "Encryption" 标签页
echo 2. 点击 "开始测试" 按钮
echo 3. 观察测试结果
echo.
echo 如果看到 "🎉 测试完全成功！" 表示功能正常
echo.
echo ========================================
echo.

REM 启动应用程序
start "" "build\windows\x64\runner\Release\anywallpaper_engine_example.exe"

cd ..
echo 日志文件: %LOG_FILE%
echo.

