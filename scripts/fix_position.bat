@echo off
chcp 65001 >nul
echo ========================================
echo  白色框位置修复工具
echo ========================================
echo.
echo 此脚本将通过 C++ 桥接清除保存的位置数据
echo.
pause

echo.
echo 正在清除 wallpaper_test_box 状态...

REM 创建临时 HTML 来调用 clearState
echo ^<html^>^<script^> > "%TEMP%\clear_state.html"
echo setTimeout(function() { >> "%TEMP%\clear_state.html"
echo   if (window.chrome ^&^& window.chrome.webview) { >> "%TEMP%\clear_state.html"
echo     window.chrome.webview.postMessage({type: 'clearState'}); >> "%TEMP%\clear_state.html"
echo     alert('状态已清除！请关闭应用并重新启动。'); >> "%TEMP%\clear_state.html"
echo   } else { >> "%TEMP%\clear_state.html"
echo     alert('chrome.webview 不可用'); >> "%TEMP%\clear_state.html"
echo   } >> "%TEMP%\clear_state.html"
echo }, 1000); >> "%TEMP%\clear_state.html"
echo ^</script^>^</html^> >> "%TEMP%\clear_state.html"

echo.
echo 临时文件已创建: %TEMP%\clear_state.html
echo.
echo 请在应用中执行以下操作：
echo 1. 在 URL 输入框中粘贴: file:///%TEMP:\=/%/clear_state.html
echo 2. 点击 Start 加载此页面
echo 3. 等待弹窗显示 "状态已清除"
echo 4. 关闭应用，重新启动
echo.
pause


