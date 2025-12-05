#!/bin/bash
# ==========================================
# AnyWP Engine - Verify macOS Precompiled Package
# ==========================================

set -e

VERSION="$1"
if [ -z "$VERSION" ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 2.2.0"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
RELEASE_DIR="$PROJECT_ROOT/release"

PRECOMPILED_ZIP="$RELEASE_DIR/anywp_engine_macos_v${VERSION}_precompiled.zip"
SOURCE_ZIP="$RELEASE_DIR/anywp_engine_macos_v${VERSION}_source.zip"

echo "========================================"
echo " Verifying macOS Packages v${VERSION}"
echo "========================================"
echo ""

ERRORS=0

# Check if ZIP files exist
echo "[1/5] Checking ZIP files..."
if [ ! -f "$PRECOMPILED_ZIP" ]; then
    echo "  [ERROR] Precompiled ZIP not found: $PRECOMPILED_ZIP"
    ERRORS=$((ERRORS + 1))
else
    echo "  [OK] Precompiled ZIP found ($(du -h "$PRECOMPILED_ZIP" | cut -f1))"
fi

if [ ! -f "$SOURCE_ZIP" ]; then
    echo "  [ERROR] Source ZIP not found: $SOURCE_ZIP"
    ERRORS=$((ERRORS + 1))
else
    echo "  [OK] Source ZIP found ($(du -h "$SOURCE_ZIP" | cut -f1))"
fi

# Extract precompiled package to temp directory
echo ""
echo "[2/5] Extracting precompiled package..."
TEMP_DIR=$(mktemp -d)
unzip -q "$PRECOMPILED_ZIP" -d "$TEMP_DIR"
PRECOMPILED_DIR="$TEMP_DIR/anywp_engine_macos_v${VERSION}_precompiled"

# Verify precompiled package structure
echo ""
echo "[3/5] Verifying precompiled package structure..."

check_file() {
    if [ -e "$1" ]; then
        echo "  [OK] $2"
    else
        echo "  [ERROR] Missing: $2"
        ERRORS=$((ERRORS + 1))
    fi
}

check_file "$PRECOMPILED_DIR/lib/anywp_engine.dart" "Dart API (lib/)"
check_file "$PRECOMPILED_DIR/lib/dart/anywp_engine.dart" "Dart API (lib/dart/)"
check_file "$PRECOMPILED_DIR/include/anywp_engine/anywp_engine_plugin.h" "C API Header"
check_file "$PRECOMPILED_DIR/macos/anywp_engine.podspec" "Podspec"
check_file "$PRECOMPILED_DIR/macos/CMakeLists.txt" "CMakeLists.txt"
check_file "$PRECOMPILED_DIR/README.md" "README.md"
check_file "$PRECOMPILED_DIR/CHANGELOG_CN.md" "CHANGELOG_CN.md"
check_file "$PRECOMPILED_DIR/LICENSE" "LICENSE"
check_file "$PRECOMPILED_DIR/pubspec.yaml" "pubspec.yaml"
check_file "$PRECOMPILED_DIR/INTEGRATION_GUIDE_MACOS.md" "Integration Guide"

# Check SDK files
echo ""
echo "[4/5] Verifying SDK files..."
SDK_FOUND=0
if [ -f "$PRECOMPILED_DIR/sdk/anywp_sdk.min.js" ]; then
    echo "  [OK] Minified SDK found"
    SDK_FOUND=1
fi
if [ -f "$PRECOMPILED_DIR/sdk/anywp_sdk.js" ]; then
    echo "  [OK] Unminified SDK found"
    SDK_FOUND=1
fi
if [ $SDK_FOUND -eq 0 ]; then
    echo "  [ERROR] No SDK files found"
    ERRORS=$((ERRORS + 1))
fi

# Check examples
echo ""
echo "[5/5] Verifying examples..."
EXAMPLE_COUNT=$(find "$PRECOMPILED_DIR/examples" -name "*.html" 2>/dev/null | wc -l)
if [ "$EXAMPLE_COUNT" -gt 0 ]; then
    echo "  [OK] Found $EXAMPLE_COUNT example files"
else
    echo "  [WARNING] No example HTML files found"
fi

# Verify framework (optional, may not be present in all builds)
if [ -d "$PRECOMPILED_DIR/Frameworks" ]; then
    echo "  [OK] Frameworks directory exists"
else
    echo "  [NOTE] Frameworks directory not present (OK for CocoaPods-based integration)"
fi

# Cleanup
rm -rf "$TEMP_DIR"

# Summary
echo ""
echo "========================================"
if [ $ERRORS -eq 0 ]; then
    echo " ✅ Verification PASSED"
    echo "========================================"
    echo ""
    echo "Precompiled package is ready for distribution!"
    exit 0
else
    echo " ❌ Verification FAILED"
    echo "========================================"
    echo ""
    echo "Found $ERRORS error(s). Please fix them before distributing."
    exit 1
fi

