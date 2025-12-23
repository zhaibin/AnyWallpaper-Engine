# Common Runtime Issues and Solutions

## Issue: Application Crashes on Startup (MSVCP140.dll)

### Symptoms
- Program crashes immediately when launched (no window appears)
- Windows Event Viewer shows:
  - **Faulting module**: `MSVCP140.dll`
  - **Exception code**: `0xc0000005` (Access Violation)
  - **Error offset**: Various offsets (e.g., `0x0000000000013080`)

### Root Cause
This is typically caused by **corrupted or incompatible Visual C++ Runtime** on the user's system.

### Solution

**Step 1: Reinstall Visual C++ Redistributable**

1. Download the latest VC++ Redistributable (x64):
   - https://aka.ms/vs/17/release/vc_redist.x64.exe

2. Run the installer:
   - If already installed, click **Repair**
   - If not installed, click **Install**

3. Restart your computer

4. Try launching the application again

**Step 2: If Issue Persists**

1. Uninstall ALL Visual C++ Redistributable versions:
   - Open "Settings" → "Apps" → "Installed apps"
   - Search for "Microsoft Visual C++ 2015-2022 Redistributable"
   - Uninstall all x64 versions

2. Restart your computer

3. Reinstall the latest version (Step 1)

**Step 3: Verify Installation**

Check if the required DLLs exist in `C:\Windows\System32\`:
- `msvcp140.dll`
- `vcruntime140.dll`
- `vcruntime140_1.dll`

### Why This Happens

- **DLL Corruption**: Windows Update or failed installations can corrupt runtime DLLs
- **Version Conflicts**: Multiple versions from different software installations
- **Incomplete Uninstallation**: Previous software left broken runtime files

### Prevention

- Keep Windows and Visual C++ Redistributable up to date
- Use official installers (avoid "lite" or modified Windows versions)

---

## Issue: WebView2 Runtime Not Found

### Symptoms
- Application shows error: "WebView2 Runtime is required"

### Solution

Download and install WebView2 Runtime:
- https://go.microsoft.com/fwlink/p/?LinkId=2124703

---

## Issue: Program Works on Some PCs but Not Others

### Common Reasons

1. **Different VC++ Runtime versions** → Reinstall runtime
2. **Missing WebView2 Runtime** → Install WebView2
3. **Windows 10 LTSC/Server editions** → Install both VC++ and WebView2
4. **Antivirus interference** → Add application to whitelist
5. **Modified/Lite Windows** → Use official Windows installation

---

**Updated**: 2025-12-23  
**Applies to**: AnyWP Engine v2.6.0+

