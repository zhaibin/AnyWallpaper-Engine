@echo off
chcp 65001 >nul 2>&1

echo ==============================================
echo  AnyWP Engine v__VERSION__ pubspec.yaml 片段
echo ==============================================
echo.
echo 将以下内容添加到你的 pubspec.yaml:
echo ----------------------------------------
echo dependencies:
echo   anywp_engine:
echo     path: ./packages/anywp_engine_v__VERSION__
echo ----------------------------------------
echo.
echo 或使用命令：
echo flutter pub add anywp_engine --path=./packages/anywp_engine_v__VERSION__
echo.
pause

