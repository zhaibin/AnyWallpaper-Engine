#!/bin/bash
# ==========================================
# AnyWP Engine - Build Web SDK Script
# Build TypeScript SDK to JavaScript
# ==========================================

set -e

MODE="${1:-development}"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SDK_DIR="$PROJECT_ROOT/sdk"

echo "========================================"
echo " Building Web SDK (${MODE} mode)"
echo "========================================"
echo ""

# Check if SDK directory exists
if [ ! -d "$SDK_DIR" ]; then
    echo "ERROR: SDK directory not found: $SDK_DIR"
    exit 1
fi

# Check if package.json exists
if [ ! -f "$SDK_DIR/package.json" ]; then
    echo "ERROR: package.json not found in SDK directory"
    exit 1
fi

# Install dependencies if needed
if [ ! -d "$SDK_DIR/node_modules" ]; then
    echo "[1/3] Installing SDK dependencies..."
    cd "$SDK_DIR"
    npm install
    cd "$PROJECT_ROOT"
else
    echo "[1/3] Dependencies already installed"
fi

# Build SDK
echo ""
echo "[2/3] Building SDK..."
cd "$SDK_DIR"

if [ "$MODE" = "production" ]; then
    npm run build:prod
else
    npm run build
fi

# Verify output
echo ""
echo "[3/3] Verifying build output..."
if [ -f "$SDK_DIR/dist/anywp_sdk.js" ]; then
    SIZE=$(du -h "$SDK_DIR/dist/anywp_sdk.js" | cut -f1)
    echo "  [OK] anywp_sdk.js ($SIZE)"
else
    echo "  [ERROR] anywp_sdk.js not found"
    exit 1
fi

if [ -f "$SDK_DIR/dist/anywp_sdk.min.js" ] && [ "$MODE" = "production" ]; then
    SIZE=$(du -h "$SDK_DIR/dist/anywp_sdk.min.js" | cut -f1)
    echo "  [OK] anywp_sdk.min.js ($SIZE)"
fi

# Copy to platform directories
echo ""
echo "Copying SDK to platform directories..."

# Windows
if [ -d "$PROJECT_ROOT/windows" ]; then
    cp "$SDK_DIR/dist/anywp_sdk.js" "$PROJECT_ROOT/windows/"
    echo "  [OK] Copied to windows/"
    if [ -f "$SDK_DIR/dist/anywp_sdk.min.js" ]; then
        cp "$SDK_DIR/dist/anywp_sdk.min.js" "$PROJECT_ROOT/windows/"
        echo "  [OK] Copied minified to windows/"
    fi
fi

# macOS
if [ -d "$PROJECT_ROOT/macos/Resources" ]; then
    cp "$SDK_DIR/dist/anywp_sdk.js" "$PROJECT_ROOT/macos/Resources/"
    echo "  [OK] Copied to macos/Resources/"
    if [ -f "$SDK_DIR/dist/anywp_sdk.min.js" ]; then
        cp "$SDK_DIR/dist/anywp_sdk.min.js" "$PROJECT_ROOT/macos/Resources/"
        echo "  [OK] Copied minified to macos/Resources/"
    fi
fi

cd "$PROJECT_ROOT"

echo ""
echo "========================================"
echo " SDK Build Complete!"
echo "========================================"
echo ""
echo "Output files:"
echo "  - sdk/dist/anywp_sdk.js"
if [ "$MODE" = "production" ]; then
    echo "  - sdk/dist/anywp_sdk.min.js"
fi
echo ""
echo "Platform copies:"
echo "  - windows/anywp_sdk.js"
if [ -d "$PROJECT_ROOT/macos/Resources" ]; then
    echo "  - macos/Resources/anywp_sdk.js"
fi
echo ""

exit 0

