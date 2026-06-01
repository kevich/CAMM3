#!/usr/bin/env bash
# Package the macOS app bundle into a .dmg using macdeployqt.
#
# Usage: packaging/macos.sh [QT_BIN_DIR]
#   QT_BIN_DIR  optional path to the Qt bin dir (containing macdeployqt).
#               If omitted, macdeployqt is taken from PATH.
set -euo pipefail

QT_BIN_DIR="${1:-}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP="$REPO_ROOT/build/app/CAMM3.app"

if [ -n "$QT_BIN_DIR" ]; then
    MACDEPLOYQT="$QT_BIN_DIR/macdeployqt"
else
    MACDEPLOYQT="$(command -v macdeployqt || true)"
fi

if [ -z "$MACDEPLOYQT" ] || [ ! -x "$MACDEPLOYQT" ]; then
    echo "error: macdeployqt not found. Put the Qt bin dir on PATH or pass it as arg 1." >&2
    exit 1
fi

if [ ! -d "$APP" ]; then
    echo "error: $APP not found. Build the project first (see packaging/README.md)." >&2
    exit 1
fi

echo "Deploying $APP -> .dmg with $MACDEPLOYQT"
"$MACDEPLOYQT" "$APP" -dmg

echo "Done: $REPO_ROOT/build/app/CAMM3.dmg"
