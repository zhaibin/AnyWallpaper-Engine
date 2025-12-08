#!/bin/bash
# Copy examples to app bundle Resources
# This script runs during the Flutter build process

set -e

echo "========================================="
echo " Copying examples to App Bundle"
echo "========================================="

# Get the build directory from Flutter
BUILD_DIR="${BUILT_PRODUCTS_DIR}/${PRODUCT_NAME}.app/Contents/Resources"

# Source examples directory (relative to project root)
PROJECT_ROOT="${SRCROOT}/../.."
EXAMPLES_SRC="${PROJECT_ROOT}/examples"

echo "Source: $EXAMPLES_SRC"
echo "Target: $BUILD_DIR/examples"

# Create Resources directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Copy examples directory
if [ -d "$EXAMPLES_SRC" ]; then
    echo "Copying examples..."
    cp -R "$EXAMPLES_SRC" "$BUILD_DIR/"
    echo "✅ Examples copied successfully"
    
    # List copied files
    echo ""
    echo "Copied files:"
    ls -lh "$BUILD_DIR/examples" | grep ".html" | awk '{print "  - " $9 " (" $5 ")"}'
else
    echo "❌ ERROR: Examples directory not found at $EXAMPLES_SRC"
    exit 1
fi

echo ""
echo "========================================="
echo " Copy Complete"
echo "========================================="

