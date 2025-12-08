#!/bin/bash
# ==========================================
# AnyWP Engine - Framework Post-Processing Script
# Fix framework structure and sign for distribution
# ==========================================

set -e

FRAMEWORK_PATH="$1"

if [ -z "$FRAMEWORK_PATH" ]; then
    echo "Usage: $0 <path-to-framework>"
    echo "Example: $0 anywp_engine.framework"
    exit 1
fi

if [ ! -d "$FRAMEWORK_PATH" ]; then
    echo "ERROR: Framework not found at $FRAMEWORK_PATH"
    exit 1
fi

echo "========================================="
echo " Framework Post-Processing"
echo "========================================="
echo ""
echo "Framework: $FRAMEWORK_PATH"
echo ""

# Get framework name
FRAMEWORK_NAME=$(basename "$FRAMEWORK_PATH" .framework)
echo "Framework name: $FRAMEWORK_NAME"

# ==========================================
# Step 1: Fix Framework Structure (Symbolic Links)
# ==========================================

echo ""
echo "[1/3] Fixing framework symbolic link structure..."

cd "$FRAMEWORK_PATH"

# Check if Versions directory exists
if [ ! -d "Versions" ]; then
    echo "  Creating Versions/A directory structure..."
    mkdir -p "Versions/A"
    
    # Move binary and resources to Versions/A if they exist at root
    if [ -f "$FRAMEWORK_NAME" ]; then
        mv "$FRAMEWORK_NAME" "Versions/A/"
    fi
    if [ -d "Headers" ]; then
        mv "Headers" "Versions/A/"
    fi
    if [ -d "Resources" ]; then
        mv "Resources" "Versions/A/"
    fi
    if [ -d "Modules" ]; then
        mv "Modules" "Versions/A/"
    fi
fi

# Ensure Current symlink exists
if [ ! -L "Versions/Current" ]; then
    echo "  Creating Versions/Current -> A"
    ln -sf A "Versions/Current"
fi

# Create root-level symlinks
echo "  Creating root-level symlinks..."

if [ ! -L "$FRAMEWORK_NAME" ] && [ -f "Versions/A/$FRAMEWORK_NAME" ]; then
    ln -sf "Versions/Current/$FRAMEWORK_NAME" "$FRAMEWORK_NAME"
    echo "    ✅ $FRAMEWORK_NAME -> Versions/Current/$FRAMEWORK_NAME"
fi

if [ ! -L "Headers" ] && [ -d "Versions/A/Headers" ]; then
    ln -sf "Versions/Current/Headers" "Headers"
    echo "    ✅ Headers -> Versions/Current/Headers"
fi

if [ ! -L "Resources" ] && [ -d "Versions/A/Resources" ]; then
    ln -sf "Versions/Current/Resources" "Resources"
    echo "    ✅ Resources -> Versions/Current/Resources"
fi

if [ ! -L "Modules" ] && [ -d "Versions/A/Modules" ]; then
    ln -sf "Versions/Current/Modules" "Modules"
    echo "    ✅ Modules -> Versions/Current/Modules"
fi

cd - > /dev/null

# ==========================================
# Step 2: Verify Framework Structure
# ==========================================

echo ""
echo "[2/3] Verifying framework structure..."

STRUCTURE_OK=true

if [ ! -L "$FRAMEWORK_PATH/Versions/Current" ]; then
    echo "  ❌ Missing: Versions/Current symlink"
    STRUCTURE_OK=false
fi

if [ ! -L "$FRAMEWORK_PATH/$FRAMEWORK_NAME" ]; then
    echo "  ❌ Missing: $FRAMEWORK_NAME symlink"
    STRUCTURE_OK=false
fi

if [ ! -f "$FRAMEWORK_PATH/Versions/A/$FRAMEWORK_NAME" ]; then
    echo "  ❌ Missing: Versions/A/$FRAMEWORK_NAME binary"
    STRUCTURE_OK=false
fi

if [ "$STRUCTURE_OK" = true ]; then
    echo "  ✅ Framework structure is valid"
else
    echo "  ⚠️  Framework structure has issues"
fi

# ==========================================
# Step 3: Code Sign Framework
# ==========================================

echo ""
echo "[3/3] Signing framework with ad-hoc signature..."

# Remove existing signature
codesign --remove-signature "$FRAMEWORK_PATH" 2>/dev/null || true

# Sign with ad-hoc signature (-)
if codesign --force --deep --sign - "$FRAMEWORK_PATH" 2>/dev/null; then
    echo "  ✅ Framework signed successfully"
    
    # Verify signature
    echo ""
    echo "Signature verification:"
    codesign --verify --verbose "$FRAMEWORK_PATH"
    
    # Display signature info
    echo ""
    echo "Signature info:"
    codesign --display --verbose "$FRAMEWORK_PATH"
else
    echo "  ⚠️  Warning: Framework signing failed"
    echo "  This may cause issues on some systems"
fi

echo ""
echo "========================================="
echo " ✅ Framework Post-Processing Complete"
echo "========================================="
echo ""
echo "Framework is ready for distribution."
echo ""

