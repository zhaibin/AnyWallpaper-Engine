@echo off
REM 诊断并修复 WorkerW 创建问题
setlocal enabledelayedexpansion

echo ========================================
echo WorkerW Creation Diagnostic Tool
echo ========================================
echo.

echo [STEP 1] Restarting Explorer to ensure clean desktop state...
taskkill /f /im explorer.exe > nul 2>&1
timeout /t 2 /nobreak > nul
start explorer.exe
echo Waiting for Explorer to initialize...
timeout /t 5 /nobreak > nul
echo.

echo [STEP 2] Running PowerShell diagnostic...
powershell -ExecutionPolicy Bypass -Command ^
"Add-Type @'^
using System;^
using System.Runtime.InteropServices;^
using System.Text;^
^
public class Win32 {^
    [DllImport(\"user32.dll\", CharSet = CharSet.Auto)]^
    public static extern IntPtr FindWindow(string className, string windowName);^
    ^
    [DllImport(\"user32.dll\", CharSet = CharSet.Auto)]^
    public static extern IntPtr FindWindowEx(IntPtr parent, IntPtr child, string className, string windowName);^
    ^
    [DllImport(\"user32.dll\")]^
    public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam, uint flags, uint timeout, out IntPtr result);^
    ^
    [DllImport(\"user32.dll\")]^
    public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);^
    ^
    [DllImport(\"user32.dll\")]^
    public static extern int GetClassName(IntPtr hWnd, StringBuilder className, int maxCount);^
    ^
    [DllImport(\"user32.dll\")]^
    public static extern bool IsWindowVisible(IntPtr hWnd);^
    ^
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);^
}^
'@;^
^
Write-Host '=== Initial Desktop State ===';^
Write-Host '';^
$progman = [Win32]::FindWindow('Progman', $null);^
if ($progman -eq [IntPtr]::Zero) {^
    Write-Host '❌ CRITICAL: Progman window NOT found!';^
    Write-Host '   This should never happen. Explorer may be corrupted.';^
    exit 1;^
}^
Write-Host \"✅ Progman found: 0x$($progman.ToString('X'))\";^
^
$defViewInProgman = [Win32]::FindWindowEx($progman, [IntPtr]::Zero, 'SHELLDLL_DefView', $null);^
Write-Host \"SHELLDLL_DefView in Progman: $(if ($defViewInProgman -ne [IntPtr]::Zero) { 'YES (0x' + $defViewInProgman.ToString('X') + ')' } else { 'NO' })\";^
^
$workerWCount = 0;^
$callback = {^
    param($hWnd, $lParam)^
    $className = New-Object System.Text.StringBuilder 256;^
    [Win32]::GetClassName($hWnd, $className, 256) ^| Out-Null;^
    if ($className.ToString() -eq 'WorkerW') { $script:workerWCount++ }^
    return $true;^
};^
[Win32]::EnumWindows($callback, [IntPtr]::Zero) ^| Out-Null;^
Write-Host \"Total WorkerW windows: $workerWCount\";^
^
Write-Host '';^
Write-Host '=== Testing 0x052C message (wParam=0, lParam=0) ===';^
Write-Host '';^
^
for ($i = 1; $i -le 3; $i++) {^
    $result = [IntPtr]::Zero;^
    $ret = [Win32]::SendMessageTimeout($progman, 0x052C, [IntPtr]::Zero, [IntPtr]::Zero, 0, 1000, [ref]$result);^
    Write-Host \"  Attempt $i/3: ret=$($ret.ToInt64()), result=$($result.ToInt64())\";^
    Start-Sleep -Milliseconds 150;^
}^
^
Write-Host '';^
Write-Host 'Waiting for desktop to reorganize...';^
Start-Sleep -Seconds 2;^
^
Write-Host '';^
Write-Host '=== Desktop State After Messages ===';^
Write-Host '';^
^
$defViewInProgman = [Win32]::FindWindowEx($progman, [IntPtr]::Zero, 'SHELLDLL_DefView', $null);^
Write-Host \"SHELLDLL_DefView in Progman: $(if ($defViewInProgman -ne [IntPtr]::Zero) { 'YES (❌ Not moved)' } else { 'NO (✅ Moved to WorkerW!)' })\";^
^
$script:foundWorkerWWithDefView = $false;^
$script:workerWWithDefViewHandle = [IntPtr]::Zero;^
$callback2 = {^
    param($hWnd, $lParam)^
    $className = New-Object System.Text.StringBuilder 256;^
    [Win32]::GetClassName($hWnd, $className, 256) ^| Out-Null;^
    if ($className.ToString() -eq 'WorkerW') {^
        $defView = [Win32]::FindWindowEx($hWnd, [IntPtr]::Zero, 'SHELLDLL_DefView', $null);^
        if ($defView -ne [IntPtr]::Zero) {^
            $script:foundWorkerWWithDefView = $true;^
            $script:workerWWithDefViewHandle = $hWnd;^
        }^
    }^
    return $true;^
};^
[Win32]::EnumWindows($callback2, [IntPtr]::Zero) ^| Out-Null;^
^
if ($script:foundWorkerWWithDefView) {^
    Write-Host \"✅ SUCCESS: WorkerW with SHELLDLL_DefView found: 0x$($script:workerWWithDefViewHandle.ToString('X'))\";^
    Write-Host '';^
    Write-Host '   This means the 0x052C message worked correctly!';^
    Write-Host '   Your program should now be able to display wallpaper.';^
    exit 0;^
} else {^
    Write-Host '❌ FAILED: WorkerW with SHELLDLL_DefView NOT found';^
    Write-Host '';^
    Write-Host '   The 0x052C message did NOT create the correct WorkerW structure.';^
    Write-Host '   This may be due to:';^
    Write-Host '   1. Windows 11 version incompatibility';^
    Write-Host '   2. System policies blocking desktop modification';^
    Write-Host '   3. Third-party software interfering';^
    exit 1;^
}"

echo.
echo ========================================
echo.
if %ERRORLEVEL% EQU 0 (
    echo ✅ Diagnostic PASSED - WorkerW structure is correct
    echo.
    echo Now testing your application...
    timeout /t 3 /nobreak > nul
    .\scripts\debug.bat
) else (
    echo ❌ Diagnostic FAILED - Cannot create WorkerW structure
    echo.
    echo This indicates a fundamental compatibility issue with your system.
    echo Possible solutions:
    echo   1. Check Windows Update for latest patches
    echo   2. Check if any security software is blocking desktop modification
    echo   3. Try running as Administrator
)

pause

