@echo off
REM ============================================================================
REM Test Engine and SDK Version APIs
REM ============================================================================

setlocal EnableDelayedExpansion

echo.
echo ========================================
echo   测试版本号 API
echo ========================================
echo.

REM Create test Dart script
set TEST_SCRIPT=example\lib\test_versions.dart
echo import 'package:anywp_engine/anywp_engine.dart'; > %TEST_SCRIPT%
echo. >> %TEST_SCRIPT%
echo void main() async { >> %TEST_SCRIPT%
echo   print('========================================'); >> %TEST_SCRIPT%
echo   print('  AnyWP Engine Version Test'); >> %TEST_SCRIPT%
echo   print('========================================\n'); >> %TEST_SCRIPT%
echo. >> %TEST_SCRIPT%
echo   // Get Engine Version >> %TEST_SCRIPT%
echo   final engineVersion = await AnyWPEngine.getPluginVersion(); >> %TEST_SCRIPT%
echo   print('📦 Engine Version: $engineVersion'); >> %TEST_SCRIPT%
echo. >> %TEST_SCRIPT%
echo   // Get SDK Version >> %TEST_SCRIPT%
echo   final sdkVersion = await AnyWPEngine.getSDKVersion(); >> %TEST_SCRIPT%
echo   print('🎨 Web SDK Version: $sdkVersion'); >> %TEST_SCRIPT%
echo. >> %TEST_SCRIPT%
echo   // Verify versions match >> %TEST_SCRIPT%
echo   if (engineVersion == sdkVersion) { >> %TEST_SCRIPT%
echo     print('\n✅ Versions match! (Both: $engineVersion)'); >> %TEST_SCRIPT%
echo   } else { >> %TEST_SCRIPT%
echo     print('\n⚠️ Version mismatch!'); >> %TEST_SCRIPT%
echo     print('   Engine: $engineVersion'); >> %TEST_SCRIPT%
echo     print('   SDK: $sdkVersion'); >> %TEST_SCRIPT%
echo   } >> %TEST_SCRIPT%
echo. >> %TEST_SCRIPT%
echo   print('\n========================================\n'); >> %TEST_SCRIPT%
echo } >> %TEST_SCRIPT%

echo ✅ 测试脚本已创建: %TEST_SCRIPT%
echo.

REM Run test
echo 🚀 运行测试...
echo.

cd example
call flutter run -d windows --dart-define=TEST_VERSIONS=1 lib/test_versions.dart

cd ..

REM Clean up
del %TEST_SCRIPT% >nul 2>&1

echo.
echo ========================================
echo   测试完成
echo ========================================
echo.

pause

