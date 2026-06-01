#!/usr/bin/env bash
# Package the Linux ELF into a portable AppImage using linuxdeployqt.
#
# Usage: packaging/linux.sh [QT_BIN_DIR]
#   QT_BIN_DIR  optional path to the Qt bin dir (containing qmake). Prepended to
#               PATH so linuxdeployqt can locate the matching Qt. If omitted,
#               qmake must already be on PATH.
#
# linuxdeployqt itself is NOT part of Qt. Download it from
#   https://github.com/probonopd/linuxdeployqt/releases
# and either put it on PATH or set LINUXDEPLOYQT to its path.
set -euo pipefail

QT_BIN_DIR="${1:-}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$REPO_ROOT/build/app/CAMM3"

if [ -n "$QT_BIN_DIR" ]; then
    export PATH="$QT_BIN_DIR:$PATH"   # so linuxdeployqt finds the matching qmake
fi

LINUXDEPLOYQT="${LINUXDEPLOYQT:-$(command -v linuxdeployqt || true)}"
if [ -z "$LINUXDEPLOYQT" ] || [ ! -x "$LINUXDEPLOYQT" ]; then
    echo "error: linuxdeployqt not found. Install it (see packaging/README.md) and" >&2
    echo "       put it on PATH or set LINUXDEPLOYQT=/path/to/linuxdeployqt." >&2
    exit 1
fi

if ! command -v qmake >/dev/null 2>&1; then
    echo "error: qmake not on PATH. Pass the Qt bin dir as arg 1 so linuxdeployqt" >&2
    echo "       can locate the matching Qt." >&2
    exit 1
fi

if [ ! -f "$BIN" ]; then
    echo "error: $BIN not found. Build the project first (see packaging/README.md)." >&2
    exit 1
fi

# Build a minimal AppDir layout that linuxdeployqt understands.
APPDIR="$REPO_ROOT/build/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cp "$BIN" "$APPDIR/usr/bin/CAMM3"

DESKTOP="$APPDIR/usr/share/applications/CAMM3.desktop"
cat > "$DESKTOP" <<'EOF'
[Desktop Entry]
Type=Application
Name=CAMM3
Exec=CAMM3
Icon=CAMM3
Categories=Utility;
Terminal=false
EOF

# linuxdeployqt requires an icon; generate a 1x1 placeholder PNG if none exists.
ICON="$APPDIR/usr/share/icons/hicolor/256x256/apps/CAMM3.png"
if [ ! -f "$ICON" ]; then
    printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x06\x00\x00\x00\x1f\x15\xc4\x89\x00\x00\x00\nIDATx\x9cc\x00\x01\x00\x00\x05\x00\x01\r\n-\xb4\x00\x00\x00\x00IEND\xaeB`\x82' > "$ICON"
fi

cd "$REPO_ROOT/build"
echo "Building AppImage with $LINUXDEPLOYQT"
# LINUXDEPLOYQT_EXTRA_ARGS lets CI add robustness flags without changing the
# default local behaviour. In CI we pass -unsupported-allow-new-glibc-versions
# because linuxdeployqt otherwise refuses to run on anything newer than the
# oldest-supported Ubuntu. (The resulting AppImage then needs a glibc at least
# as new as the build host's.)
"$LINUXDEPLOYQT" "$DESKTOP" -appimage ${LINUXDEPLOYQT_EXTRA_ARGS:-}

echo "Done: AppImage written to $REPO_ROOT/build (CAMM3-*.AppImage)"
