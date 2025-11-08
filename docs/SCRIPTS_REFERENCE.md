# AnyWP Engine - Scripts Reference

Complete reference for all build, test, and development scripts.

**Last Updated**: 2025-11-08  
**Version**: 1.3.2

---

## 📋 Script Overview

**Total Scripts**: 10 (reduced from 23)  
**All English**: No encoding issues  
**Fully Tested**: Syntax and functionality verified

---

## 🔨 Development Scripts

### `build.bat`
**Purpose**: Build and run application  
**Usage**:
```bash
cd scripts
build.bat
```

**Features**:
- Auto-check WebView2 SDK
- Build Debug version
- Launch application automatically

**When to Use**: Daily development and testing

---

### `run.bat`
**Purpose**: Quick run existing build  
**Usage**:
```bash
cd scripts
run.bat
```

**Features**:
- Detect existing build (prefer Release)
- Run immediately without rebuilding
- Fast iteration

**When to Use**: Test without code changes

---

### `debug.bat`
**Purpose**: Run with logging  
**Usage**:
```bash
cd scripts
debug.bat
```

**Features**:
- Capture output to `debug_run.log`
- Stop existing processes
- Full console output

**When to Use**: Troubleshooting and debugging

---

### `monitor_log.bat`
**Purpose**: Real-time log monitoring  
**Usage**:
```bash
cd scripts
monitor_log.bat
```

**Features**:
- Refresh every 2 seconds
- Show last 50 lines
- Auto-detect log file

**When to Use**: Watch logs in real-time

---

## 📦 Release Scripts

### `release.bat`
**Purpose**: Build release packages  
**Usage**:
```bash
cd scripts
release.bat
```

**Features**:
- Read version from pubspec.yaml
- Build Release version
- Create Flutter Plugin package (~16 MB)
- Create Web SDK package (~56 KB)
- Auto-generate ZIP files

**Output**:
- `release/anywp_engine_v{version}.zip`
- `release/anywp_web_sdk_v{version}.zip`

**When to Use**: Prepare for new release

---

### `build_sdk.bat`
**Purpose**: Build Web SDK  
**Usage**:
```bash
cd scripts
build_sdk.bat
```

**Features**:
- Build TypeScript → JavaScript
- Generate `windows/anywp_sdk.js`
- Run unit tests

**When to Use**: After modifying Web SDK source

---

## ⚙️ Setup Scripts

### `setup.bat`
**Purpose**: Install WebView2 SDK  
**Usage**:
```bash
cd scripts
setup.bat
```

**Features**:
- Download NuGet if needed
- Install Microsoft.Web.WebView2 v1.0.2592.51
- Install to `windows/packages/`

**When to Use**:
- First-time setup
- Missing SDK files
- Reinstall dependencies

---

## 🧪 Testing Scripts

### `test_full.bat`
**Purpose**: Full automated test suite  
**Usage**:
```bash
cd scripts
test_full.bat
```

**Features**:
- Build Debug version
- Run 8 test pages (~95 seconds)
- Monitor memory (CSV format)
- Monitor CPU (CSV format)
- Generate detailed report

**Output**:
- `test_logs/test_full_{timestamp}.log`
- `test_logs/memory_{timestamp}.csv`
- `test_logs/cpu_{timestamp}.csv`
- `test_logs/build_{timestamp}.log`
- `test_logs/app_output_{timestamp}.log`

**When to Use**: Full functionality and performance testing

---

### `analyze.ps1`
**Purpose**: Analyze test results  
**Usage**:
```powershell
cd scripts
./analyze.ps1
./analyze.ps1 -GenerateHTML
```

**Features**:
- Parse memory and CPU data
- Calculate performance metrics
- Performance scoring (0-100)
- Generate HTML report

**Performance Criteria**:
| Metric | Excellent | Good | Needs Improvement |
|--------|-----------|------|-------------------|
| Max Memory | < 250 MB | < 300 MB | > 300 MB |
| Memory Growth | < 5% | < 10% | > 10% |
| Avg CPU | < 10% | < 15% | > 15% |

**When to Use**: After running `test_full.bat`

---

### `verify.bat`
**Purpose**: Static verification of code structure  
**Usage**:
```bash
cd scripts
verify.bat
```

**Features**:
- Check module files exist
- Verify error handling (try-catch)
- Check Logger enhancements
- Verify test framework

**When to Use**: CI/CD checks, code review

---

## 🎯 Common Workflows

### Daily Development
```bash
# 1. First time
scripts\setup.bat

# 2. Build and test
scripts\build.bat

# 3. Quick test (no rebuild)
scripts\run.bat

# 4. Debug issues
scripts\debug.bat
```

### Web SDK Development
```bash
# 1. Edit TypeScript source (windows/sdk/)

# 2. Build SDK
scripts\build_sdk.bat

# 3. Test in Flutter
scripts\run.bat
```

### Testing Workflow
```bash
# 1. Full test suite
scripts\test_full.bat

# 2. Analyze results
scripts\analyze.ps1 -GenerateHTML

# 3. Static verification
scripts\verify.bat
```

### Release Workflow
```bash
# 1. Build release packages
scripts\release.bat

# 2. Verify packages
#    Check release/anywp_engine_v{version}/
#    Check release/anywp_web_sdk_v{version}.zip

# 3. Push to GitHub
git push origin main

# 4. Create tag
git tag -a v{version} -m "Description"
git push origin v{version}

# 5. Create GitHub Release
#    Upload both ZIP files
```

---

## ✅ Script Features

### All English
- No Chinese characters in scripts
- No encoding issues (UTF-8 or ASCII only)
- International compatibility

### Error Handling
- Check file existence
- Validate build results
- Clear error messages
- Non-zero exit codes on failure

### User Friendly
- Clear progress indicators
- Detailed usage instructions
- Helpful error messages
- Pause for review when needed

---

## 🗑️ Removed Scripts (v1.3.2)

**Duplicate Functionality**:
- ❌ `build_and_run.bat` → `build.bat`
- ❌ `test.bat` → `run.bat`
- ❌ `debug_run.bat` → `debug.bat`
- ❌ `setup_webview2.bat` → `setup.bat`
- ❌ `build_release_v2.bat` → `release.bat`
- ❌ `comprehensive_auto_test.bat` → `test_full.bat`
- ❌ `analyze_test_results.ps1` → `analyze.ps1`
- ❌ `verify_optimizations.bat` → `verify.bat`

**Deprecated**:
- ❌ `test_draggable.bat` - Use `run.bat` + manual page switch
- ❌ `build_web_sdk.ps1` - Integrated into `release.bat`
- ❌ `run_comprehensive_test.bat` - Redundant wrapper
- ❌ `test_refactoring.bat` - Refactoring complete
- ❌ `test_optimizations.bat` - Duplicate of `verify.bat`
- ❌ `comprehensive_test.bat` - Simple version, replaced
- ❌ `run_auto_test.bat` - Deprecated wrapper
- ❌ `auto_test.py` - Hardcoded paths
- ❌ `automated_test.ps1` - Incomplete implementation

**Historical**:
- ❌ `build_release.bat` - Old version (v1)
- ❌ `PUSH_TO_GITHUB.bat` - Encoding issues

---

## 📊 Script Statistics

**By Category**:
- Development: 4 scripts
- Release: 2 scripts
- Setup: 1 script
- Testing: 3 scripts

**Total**: 10 scripts (down from 23)

**Reduction**: 57% fewer scripts

---

## 🚫 Script Policy

**NO MORE SCRIPTS**: Script collection is now complete and stable.

**When to Break This Rule**:
- Critical new functionality that doesn't fit existing scripts
- Must be approved and documented
- Should replace an existing script if possible

**Alternative Solutions**:
- Add features to existing scripts
- Use script arguments/parameters
- Create helper functions in existing scripts
- Document manual procedures

---

## 📝 Script Naming Conventions

**Format**: `{action}.{ext}`

**Examples**:
- `build.bat` - Build action
- `run.bat` - Run action
- `test_full.bat` - Full test (qualifier needed)

**Rules**:
- All lowercase
- Underscore for multi-word
- No version numbers in name
- English only

---

## 🔧 Maintenance

### Adding Features
1. Identify target script
2. Add feature with comments
3. Update this documentation
4. Test thoroughly

### Modifying Scripts
1. Test before committing
2. Maintain backward compatibility
3. Update documentation
4. Keep English only

### Removing Scripts
1. Verify no dependencies
2. Update all documentation
3. Check CI/CD pipelines
4. Announce to team

---

## 📖 See Also

- **Developer Guide**: `docs/FOR_FLUTTER_DEVELOPERS.md`
- **Web SDK Guide**: `docs/WEB_DEVELOPER_GUIDE_CN.md`
- **Testing Guide**: `scripts/README_TEST_SCRIPTS.md`
- **Integration**: `docs/PRECOMPILED_DLL_INTEGRATION.md`

---

**All scripts tested and verified ✅**  
**No encoding issues ✅**  
**Fully English ✅**

