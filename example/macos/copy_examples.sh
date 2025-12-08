#!/bin/bash
# Pre-build script to copy examples to app bundle
# Place this in: example/macos/

set -e

echo "========================================="
echo " Pre-build: Preparing examples"
echo "========================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
EXAMPLES_SRC="$PROJECT_ROOT/examples"
BUILD_DIR="$SCRIPT_DIR/build/macos/Build/Products"

echo "Project root: $PROJECT_ROOT"
echo "Examples source: $EXAMPLES_SRC"

# Function to copy examples to a specific build configuration
copy_examples() {
    local config=$1
    local target_dir="$BUILD_DIR/$config/Runner.app/Contents/Resources"
    
    if [ -d "$target_dir" ]; then
        echo "Copying to $config..."
        mkdir -p "$target_dir"
        cp -R "$EXAMPLES_SRC" "$target_dir/" 2>/dev/null || true
        echo "  ✅ Done"
    fi
}

# Check if we're being called during Flutter build
if [ -n "$BUILT_PRODUCTS_DIR" ]; then
    # Called from Xcode build phase
    echo "Running from Xcode build phase"
    TARGET_DIR="$BUILT_PRODUCTS_DIR/$PRODUCT_NAME.app/Contents/Resources"
    echo "Target: $TARGET_DIR"
    
    mkdir -p "$TARGET_DIR"
    cp -R "$EXAMPLES_SRC" "$TARGET_DIR/"
    echo "✅ Examples copied to bundle"
else
    # Called manually or from Flutter build
    echo "Running manually"
    echo ""
    echo "To manually copy examples after build:"
    echo "  Release: $BUILD_DIR/Release/Runner.app/Contents/Resources"
    echo "  Debug:   $BUILD_DIR/Debug/Runner.app/Contents/Resources"
    echo ""
    
    # Try to copy to existing builds
    copy_examples "Release"
    copy_examples "Debug"
fi

echo ""
echo "========================================="
echo " Pre-build Complete"
echo "========================================="

