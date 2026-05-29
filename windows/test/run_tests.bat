@echo off
REM AnyWP Engine - Compatibility test runner
REM
REM The historical unit_tests.cpp suite references APIs that no longer exist
REM after the Windows modules were refactored. Keep this entry point stable by
REM delegating to the maintained independent-module comprehensive suite.

echo ========================================
echo AnyWP Engine - Unit Tests
echo ========================================
echo.
echo Delegating to maintained comprehensive tests...
echo.

call "%~dp0run_comprehensive_test.bat"
exit /b %ERRORLEVEL%
