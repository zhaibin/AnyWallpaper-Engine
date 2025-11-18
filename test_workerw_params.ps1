Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;

public class Win32 {
    [DllImport("user32.dll", CharSet = CharSet.Auto)]
    public static extern IntPtr FindWindow(string className, string windowName);
    
    [DllImport("user32.dll", CharSet = CharSet.Auto)]
    public static extern IntPtr FindWindowEx(IntPtr parent, IntPtr child, string className, string windowName);
    
    [DllImport("user32.dll")]
    public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam, uint flags, uint timeout, out IntPtr result);
    
    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);
    
    [DllImport("user32.dll")]
    public static extern int GetClassName(IntPtr hWnd, StringBuilder className, int maxCount);
    
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
}
"@

Write-Host "`n=== Testing WorkerW Creation with Different Parameters ===`n"

$progman = [Win32]::FindWindow("Progman", $null)
Write-Host "Progman: 0x$($progman.ToString('X'))"

# Check current state
$defViewInProgman = [Win32]::FindWindowEx($progman, [IntPtr]::Zero, "SHELLDLL_DefView", $null)
Write-Host "SHELLDLL_DefView in Progman BEFORE: $(if ($defViewInProgman -ne [IntPtr]::Zero) { 'YES (0x' + $defViewInProgman.ToString('X') + ')' } else { 'NO' })"

Write-Host "`n=== Sending 0x052C with wParam=0xD, lParam=1 (alternative variant) ===`n"

$result = [IntPtr]::Zero
$ret = [Win32]::SendMessageTimeout($progman, 0x052C, [IntPtr]::new(0xD), [IntPtr]::new(1), 0, 1000, [ref]$result)
Write-Host "SendMessageTimeout result: ret=$($ret.ToInt64()), result=$($result.ToInt64())"

Start-Sleep -Seconds 1

# Check after
$defViewInProgman = [Win32]::FindWindowEx($progman, [IntPtr]::Zero, "SHELLDLL_DefView", $null)
Write-Host "`nSHELLDLL_DefView in Progman AFTER: $(if ($defViewInProgman -ne [IntPtr]::Zero) { 'YES (still there)' } else { 'NO (moved!)' })"

# Look for WorkerW with SHELLDLL_DefView
$script:foundWorkerW = $false
$callback = {
    param($hWnd, $lParam)
    $className = New-Object System.Text.StringBuilder 256
    [Win32]::GetClassName($hWnd, $className, 256) | Out-Null
    
    if ($className.ToString() -eq "WorkerW") {
        $defView = [Win32]::FindWindowEx($hWnd, [IntPtr]::Zero, "SHELLDLL_DefView", $null)
        if ($defView -ne [IntPtr]::Zero) {
            Write-Host "✅ Found WorkerW with SHELLDLL_DefView: 0x$($hWnd.ToString('X'))"
            $script:foundWorkerW = $true
        }
    }
    return $true
}

[Win32]::EnumWindows($callback, [IntPtr]::Zero) | Out-Null

if (-not $script:foundWorkerW) {
    Write-Host "❌ No WorkerW with SHELLDLL_DefView found"
}

Write-Host "`n=== End of Test ===`n"
