#!/bin/bash
# Automated build script for macOS with examples bundling
# Usage: ./build_macos.sh [debug|release]

set -e

MODE="${1:-release}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================="
echo " Building macOS App with Examples"
echo " Mode: $MODE"
echo "========================================="
echo ""

# Step 1: Clean and prepare
echo "[1/4] Cleaning previous build..."
flutter clean
flutter pub get

# Step 2: Build the app
echo ""
echo "[2/4] Building macOS application ($MODE)..."
if [ "$MODE" = "debug" ]; then
    flutter build macos --debug
    BUILD_DIR="build/macos/Build/Products/Debug"
else
    flutter build macos --release
    BUILD_DIR="build/macos/Build/Products/Release"
fi

# Step 3: Copy examples to app bundle
echo ""
echo "[3/4] Copying examples to app bundle..."
RESOURCES_DIR="$BUILD_DIR/Runner.app/Contents/Resources"
mkdir -p "$RESOURCES_DIR"
cp -R ../examples "$RESOURCES_DIR/"

EXAMPLE_COUNT=$(ls -1 "$RESOURCES_DIR/examples"/*.html 2>/dev/null | wc -l | tr -d ' ')
echo "  ✅ Copied $EXAMPLE_COUNT example files"

# Step 4: Verification
echo ""
echo "[4/4] Verifying bundle..."
if [ -d "$RESOURCES_DIR/examples" ]; then
    echo "  ✅ Examples directory exists in bundle"
    echo "  ✅ App is ready to run"
else
    echo "  ❌ Examples directory NOT found in bundle"
    exit 1
fi

echo ""
echo "========================================="
echo " Build Complete!"
echo "========================================="
echo ""
echo "App location: $BUILD_DIR/Runner.app"
echo ""
echo "To run:"
echo "  open $BUILD_DIR/Runner.app"
echo ""
echo "Or:"
echo "  flutter run -d macos --release"
echo ""

