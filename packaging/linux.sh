#!/usr/bin/env bash
# Package the Linux ELF into a portable AppImage using linuxdeploy + its Qt
# plugin (the actively-maintained toolchain; probonopd/linuxdeployqt is fragile
# on modern distros and aborts silently).
#
# Usage: packaging/linux.sh [QT_BIN_DIR]
#   QT_BIN_DIR  optional path to the Qt bin dir (containing qmake). Prepended to
#               PATH so the Qt plugin locates the matching Qt. If omitted, qmake
#               must already be on PATH.
#
# These tools are NOT part of Qt. Download them from
#   https://github.com/linuxdeploy/linuxdeploy/releases
#   https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases
# and either put both on PATH (linuxdeploy auto-discovers a plugin named
# `linuxdeploy-plugin-qt` on PATH) or set LINUXDEPLOY to linuxdeploy's path.
set -euo pipefail

QT_BIN_DIR="${1:-}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$REPO_ROOT/build/app/CAMM3"

if [ -n "$QT_BIN_DIR" ]; then
    export PATH="$QT_BIN_DIR:$PATH"   # so the Qt plugin finds the matching qmake
fi

LINUXDEPLOY="${LINUXDEPLOY:-$(command -v linuxdeploy || true)}"
if [ -z "$LINUXDEPLOY" ] || [ ! -x "$LINUXDEPLOY" ]; then
    echo "error: linuxdeploy not found. Install it (see packaging/README.md) and" >&2
    echo "       put it on PATH or set LINUXDEPLOY=/path/to/linuxdeploy." >&2
    exit 1
fi

if ! command -v qmake >/dev/null 2>&1; then
    echo "error: qmake not on PATH. Pass the Qt bin dir as arg 1 so the Qt plugin" >&2
    echo "       can locate the matching Qt." >&2
    exit 1
fi
# The Qt plugin reads $QMAKE to find Qt; default it to the qmake we just located.
export QMAKE="${QMAKE:-$(command -v qmake)}"

if [ ! -f "$BIN" ]; then
    echo "error: $BIN not found. Build the project first (see packaging/README.md)." >&2
    exit 1
fi

# Staging desktop file + icon. linuxdeploy installs them into the AppDir's
# standard locations itself (via -d / -i), so we don't pre-build the FHS tree.
APPDIR="$REPO_ROOT/build/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR"

DESKTOP="$REPO_ROOT/build/CAMM3.desktop"
cat > "$DESKTOP" <<'EOF'
[Desktop Entry]
Type=Application
Name=CAMM3
Exec=CAMM3
Icon=CAMM3
Categories=Utility;
Terminal=false
EOF

# linuxdeploy requires an icon at a standard resolution (it rejects a 1x1
# placeholder), so embed a 256x256 solid-colour PNG and write it if none exists.
ICON="$REPO_ROOT/build/CAMM3.png"
if [ ! -f "$ICON" ]; then
    base64 --decode > "$ICON" <<'EOF'
iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAACYUlEQVR42u3UMQEAAAjDMJRgcP5vMIADcsRAj1YnA/xUIoABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAGAAIoABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAGAAIoABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAGAAQoABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAgAEABgAYAGAAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYACAAQAGABgAYABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAIABAAYAGABgAMBlAXoiHT6ayFZgAAAAAElFTkSuQmCC
EOF
fi

cd "$REPO_ROOT/build"
echo "Building AppImage with $LINUXDEPLOY (QMAKE=$QMAKE)"
"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$BIN" \
    --desktop-file "$DESKTOP" \
    --icon-file "$ICON" \
    --plugin qt \
    --output appimage

echo "Done: AppImage written to $REPO_ROOT/build (CAMM3-*.AppImage)"
