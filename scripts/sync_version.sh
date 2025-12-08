#!/bin/bash
# ==========================================
# Sync version from pubspec.yaml to podspec
# ==========================================

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

PUBSPEC="$PROJECT_ROOT/pubspec.yaml"
PODSPEC="$PROJECT_ROOT/macos/anywp_engine.podspec"

if [ ! -f "$PUBSPEC" ]; then
    echo "ERROR: pubspec.yaml not found at $PUBSPEC"
    exit 1
fi

if [ ! -f "$PODSPEC" ]; then
    echo "ERROR: podspec not found at $PODSPEC"
    exit 1
fi

# Extract version from pubspec.yaml
VERSION=$(grep '^version:' "$PUBSPEC" | sed 's/version: *//' | tr -d ' ')

if [ -z "$VERSION" ]; then
    echo "ERROR: Could not extract version from pubspec.yaml"
    exit 1
fi

echo "Syncing version: $VERSION"
echo "  Source: $PUBSPEC"
echo "  Target: $PODSPEC"

# Update podspec version
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS sed
    sed -i '' "s/s.version *= *'[^']*'/s.version          = '$VERSION'/" "$PODSPEC"
else
    # Linux sed
    sed -i "s/s.version *= *'[^']*'/s.version          = '$VERSION'/" "$PODSPEC"
fi

# Verify
NEW_VERSION=$(grep "s.version" "$PODSPEC" | sed "s/.*'\([^']*\)'.*/\1/")

if [ "$NEW_VERSION" = "$VERSION" ]; then
    echo "✅ Version synced successfully: $VERSION"
else
    echo "❌ Version sync failed"
    echo "  Expected: $VERSION"
    echo "  Got: $NEW_VERSION"
    exit 1
fi

