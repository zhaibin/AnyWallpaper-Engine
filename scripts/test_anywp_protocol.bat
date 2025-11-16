@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo  测试 anywp:// 协议 (v2.1.10)
echo ========================================
echo.

set "PROJECT_ROOT=%~dp0.."
cd /d "%PROJECT_ROOT%"

echo [1/5] 准备测试环境...
echo.

REM 创建测试目录
set "TEST_DIR=%USERPROFILE%\Desktop\AnyWP_Test"
if not exist "%TEST_DIR%" mkdir "%TEST_DIR%"
echo ✓ 测试目录: %TEST_DIR%

REM 创建缓存目录
set "CACHE_DIR=%TEST_DIR%\cache"
if not exist "%CACHE_DIR%" mkdir "%CACHE_DIR%"
echo ✓ 缓存目录: %CACHE_DIR%

echo.
echo [2/5] 生成测试图片...
echo.

REM 使用 PowerShell 生成测试 JPEG 图片
powershell -NoProfile -Command ^
"Add-Type -AssemblyName System.Drawing; ^
$bitmap = New-Object System.Drawing.Bitmap(800, 600); ^
$graphics = [System.Drawing.Graphics]::FromImage($bitmap); ^
$graphics.Clear([System.Drawing.Color]::FromArgb(103, 126, 234)); ^
$font = New-Object System.Drawing.Font('Arial', 48, [System.Drawing.FontStyle]::Bold); ^
$brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White); ^
$graphics.DrawString('AnyWP Test', $font, $brush, 150, 250); ^
$font2 = New-Object System.Drawing.Font('Arial', 24); ^
$graphics.DrawString('anywp://file Protocol', $font2, $brush, 220, 320); ^
$bitmap.Save('%TEST_DIR%\test_image_001.jpg', [System.Drawing.Imaging.ImageFormat]::Jpeg); ^
$graphics.Dispose(); ^
$bitmap.Dispose();"

if not exist "%TEST_DIR%\test_image_001.jpg" (
    echo ❌ 生成测试图片失败
    pause
    exit /b 1
)

echo ✓ 测试图片已生成: %TEST_DIR%\test_image_001.jpg

REM 复制图片创建更多测试文件
copy "%TEST_DIR%\test_image_001.jpg" "%TEST_DIR%\test_image_002.jpg" >nul
copy "%TEST_DIR%\test_image_001.jpg" "%TEST_DIR%\test_image_003.jpg" >nul
echo ✓ 生成了 3 个测试图片

echo.
echo [3/5] 编译 Flutter 项目...
echo.

cd example
call flutter clean >nul 2>&1
call flutter pub get >nul 2>&1
call flutter build windows --debug 2>&1 | findstr /C:"Building" /C:"Built" /C:"error" /C:"Error"

if !ERRORLEVEL! neq 0 (
    echo ❌ 编译失败
    cd ..
    pause
    exit /b 1
)

cd ..
echo ✓ 编译成功

echo.
echo [4/5] 生成测试 HTML 页面...
echo.

REM 创建测试 HTML 页面（使用开发者自定义路径）
(
echo ^<!DOCTYPE html^>
echo ^<html^>
echo ^<head^>
echo     ^<meta charset="UTF-8"^>
echo     ^<title^>anywp:// Protocol Test^</title^>
echo     ^<style^>
echo         body { 
echo             font-family: Arial; 
echo             background: linear-gradient^(135deg, #667eea 0%%, #764ba2 100%%^); 
echo             color: white; 
echo             padding: 40px; 
echo             margin: 0;
echo         }
echo         .container {
echo             max-width: 1200px;
echo             margin: 0 auto;
echo             background: rgba^(255, 255, 255, 0.1^);
echo             border-radius: 20px;
echo             padding: 40px;
echo         }
echo         h1 { text-align: center; font-size: 2.5em; }
echo         .btn {
echo             background: #667eea;
echo             color: white;
echo             border: none;
echo             padding: 15px 30px;
echo             font-size: 1.1em;
echo             border-radius: 25px;
echo             cursor: pointer;
echo             margin: 10px;
echo         }
echo         .btn:hover { background: #764ba2; }
echo         .result {
echo             background: rgba^(0, 0, 0, 0.3^);
echo             border-radius: 10px;
echo             padding: 20px;
echo             margin-top: 20px;
echo             font-family: monospace;
echo             max-height: 400px;
echo             overflow-y: auto;
echo         }
echo         .success { color: #4ade80; }
echo         .error { color: #f87171; }
echo         .info { color: #60a5fa; }
echo         .test-images {
echo             display: grid;
echo             grid-template-columns: repeat^(auto-fit, minmax^(300px, 1fr^)^);
echo             gap: 20px;
echo             margin-top: 30px;
echo         }
echo         .test-image {
echo             background: rgba^(0, 0, 0, 0.3^);
echo             border-radius: 15px;
echo             padding: 20px;
echo             text-align: center;
echo         }
echo         .test-image img {
echo             width: 100%%;
echo             border-radius: 10px;
echo             margin-bottom: 15px;
echo         }
echo     ^</style^>
echo ^</head^>
echo ^<body^>
echo     ^<div class="container"^>
echo         ^<h1^>🎨 anywp:// Protocol Test^</h1^>
echo         ^<p style="text-align: center; font-size: 1.2em;"^>Developer-Controlled Path Version^</p^>
echo.
echo         ^<div style="text-align: center; margin: 30px 0;"^>
echo             ^<button class="btn" onclick="encryptFiles^(^)"^>Step 1: Encrypt Files^</button^>
echo             ^<button class="btn" onclick="testProtocol^(^)"^>Step 2: Test Protocol^</button^>
echo             ^<button class="btn" onclick="clearResults^(^)"^>Clear^</button^>
echo         ^</div^>
echo.
echo         ^<div class="result" id="result"^>^</div^>
echo         ^<div class="test-images" id="testImages"^>^</div^>
echo     ^</div^>
echo.
echo     ^<script^>
echo         const testDir = '%TEST_DIR%'.replace^(/\\\\/g, '/'^);
echo         const cacheDir = '%CACHE_DIR%'.replace^(/\\\\/g, '/'^);
echo.
echo         function log^(msg, type = 'info'^) {
echo             const result = document.getElementById^('result'^);
echo             const time = new Date^(^).toLocaleTimeString^(^);
echo             const cls = type === 'success' ? 'success' : type === 'error' ? 'error' : 'info';
echo             result.innerHTML += `^<div class="${cls}"^>[${time}] ${msg}^</div^>`;
echo             result.scrollTop = result.scrollHeight;
echo         }
echo.
echo         function clearResults^(^) {
echo             document.getElementById^('result'^).innerHTML = '';
echo             document.getElementById^('testImages'^).innerHTML = '';
echo         }
echo.
echo         async function encryptFiles^(^) {
echo             clearResults^(^);
echo             log^('🔐 开始加密测试文件...', 'info'^);
echo.
echo             // 检查 AnyWP API 是否可用
echo             if ^(!window.AnyWP ^|^| !window.AnyWP.encryptFile^) {
echo                 log^('❌ AnyWP API 不可用', 'error'^);
echo                 return;
echo             }
echo.
echo             const files = [
echo                 { source: `${testDir}/test_image_001.jpg`, dest: `${cacheDir}/img_001.encrypted` },
echo                 { source: `${testDir}/test_image_002.jpg`, dest: `${cacheDir}/img_002.encrypted` },
echo                 { source: `${testDir}/test_image_003.jpg`, dest: `${cacheDir}/img_003.encrypted` }
echo             ];
echo.
echo             for ^(let i = 0; i ^< files.length; i++^) {
echo                 const file = files[i];
echo                 log^(`正在加密 ${i + 1}/3: ${file.source}`, 'info'^);
echo.
echo                 try {
echo                     const result = await window.AnyWP.encryptFile^(file.source, file.dest^);
echo                     if ^(result^) {
echo                         log^(`✅ 加密成功: ${file.dest}`, 'success'^);
echo                     } else {
echo                         log^(`❌ 加密失败: ${file.source}`, 'error'^);
echo                     }
echo                 } catch ^(e^) {
echo                     log^(`❌ 加密异常: ${e.message}`, 'error'^);
echo                 }
echo             }
echo.
echo             log^('\\n✅ 加密完成！现在可以测试协议了。', 'success'^);
echo         }
echo.
echo         async function testProtocol^(^) {
echo             const imagesDiv = document.getElementById^('testImages'^);
echo             imagesDiv.innerHTML = '';
echo.
echo             log^('\\n🚀 开始测试 anywp:// 协议...', 'info'^);
echo.
echo             const testFiles = [
echo                 `${cacheDir}/img_001.encrypted`,
echo                 `${cacheDir}/img_002.encrypted`,
echo                 `${cacheDir}/img_003.encrypted`
echo             ];
echo.
echo             testFiles.forEach^(^(path, index^) =^> {
echo                 const url = `anywp://file?path=${path}`;
echo                 log^(`测试 #${index + 1}: ${url}`, 'info'^);
echo.
echo                 const div = document.createElement^('div'^);
echo                 div.className = 'test-image';
echo.
echo                 const img = document.createElement^('img'^);
echo                 img.src = url;
echo                 img.alt = `Test ${index + 1}`;
echo.
echo                 img.onload = ^(^) =^> {
echo                     log^(`✅ 加载成功 #${index + 1}`, 'success'^);
echo                 };
echo.
echo                 img.onerror = ^(^) =^> {
echo                     log^(`❌ 加载失败 #${index + 1}: ${path}`, 'error'^);
echo                 };
echo.
echo                 const label = document.createElement^('p'^);
echo                 label.textContent = `Test Image ${index + 1}`;
echo                 label.style.fontSize = '1.2em';
echo.
echo                 div.appendChild^(img^);
echo                 div.appendChild^(label^);
echo                 imagesDiv.appendChild^(div^);
echo             }^);
echo.
echo             log^('⏳ 等待图片加载...', 'info'^);
echo         }
echo.
echo         // 页面加载完成
echo         window.addEventListener^('DOMContentLoaded', ^(^) =^> {
echo             log^('✅ 测试页面加载完成', 'success'^);
echo             log^('📂 测试目录: ' + testDir, 'info'^);
echo             log^('📦 缓存目录: ' + cacheDir, 'info'^);
echo             log^('\\n💡 点击 "Step 1" 加密文件，然后点击 "Step 2" 测试协议', 'info'^);
echo         }^);
echo     ^</script^>
echo ^</body^>
echo ^</html^>
) > "%TEST_DIR%\test_protocol.html"

echo ✓ 测试页面已生成: %TEST_DIR%\test_protocol.html

echo.
echo [5/5] 启动应用...
echo.

REM 启动应用
start "" "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"

timeout /t 3 >nul

echo.
echo ========================================
echo  📋 测试说明
echo ========================================
echo.
echo 1. 应用已启动，请在 Wallpaper 标签页中：
echo    - 输入 URL: file:///%TEST_DIR:\=/%/test_protocol.html
echo    - 点击 "Start" 按钮
echo.
echo 2. 在测试页面中：
echo    - 点击 "Step 1: Encrypt Files" - 加密测试图片
echo    - 点击 "Step 2: Test Protocol" - 测试 anywp:// 协议
echo.
echo 3. 验证结果：
echo    ✅ 图片成功显示 = 协议工作正常
echo    ❌ 图片加载失败 = 需要查看错误日志
echo.
echo 📂 测试文件位置:
echo    - 原始图片: %TEST_DIR%
echo    - 加密文件: %CACHE_DIR%
echo    - 测试页面: %TEST_DIR%\test_protocol.html
echo.
echo ========================================
echo.

pause

