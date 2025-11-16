@echo off
REM 编译自定义协议单元测试

echo ========================================
echo  Building Custom Scheme Unit Tests
echo ========================================

REM 设置编译器（需要安装 Visual Studio）
set CL_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\bin\Hostx64\x64\cl.exe"
set INCLUDE_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared"
set LIB_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64"

REM 检查编译器是否存在
if not exist %CL_PATH% (
  echo Error: Visual Studio C++ compiler not found!
  echo Please adjust CL_PATH in this script to match your VS installation.
  pause
  exit /b 1
)

REM 设置环境变量
set INCLUDE=%INCLUDE_PATH%
set LIB=%LIB_PATH%

REM 编译测试
echo.
echo Compiling test_custom_scheme.cpp...
%CL_PATH% /EHsc /std:c++17 /W3 /nologo ^
  test_custom_scheme.cpp ^
  ../utils/mime_type_detector.cpp ^
  ../utils/logger.cpp ^
  /Fe:test_custom_scheme.exe ^
  /I"../" ^
  /link shell32.lib

if %ERRORLEVEL% neq 0 (
  echo.
  echo Build failed!
  pause
  exit /b 1
)

echo.
echo ========================================
echo  Build Successful!
echo ========================================
echo.
echo Run test_custom_scheme.exe to execute tests
echo.

pause

