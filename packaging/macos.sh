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

DMG="$REPO_ROOT/build/app/CAMM3.dmg"

# 1) Bundle the Qt frameworks/plugins into the .app and rewrite its rpaths.
#    Done WITHOUT -dmg so we can (ad-hoc) sign the finished bundle before
#    packaging it — see step 2.
echo "Deploying $APP with $MACDEPLOYQT"
"$MACDEPLOYQT" "$APP"

# 2) Ad-hoc code-sign the whole bundle. On Apple Silicon an unsigned binary is
#    killed by the kernel on launch ("code signature invalid"), so this is
#    mandatory — not cosmetic — even without a Developer ID. --deep signs the
#    bundled Qt frameworks too; --force replaces macdeployqt's partial signing.
echo "Ad-hoc code-signing $APP"
codesign --force --deep --sign - "$APP"
codesign --verify --deep --strict "$APP" || {
    echo "error: code-signing verification failed." >&2
    exit 1
}

# 3) Pack the signed bundle into a compressed disk image. A .dmg is a single
#    file, so it survives upload/download without the framework symlink
#    corruption that zipping a raw .app causes.
echo "Building disk image $DMG"
rm -f "$DMG"
hdiutil create -volname CAMM3 -srcfolder "$APP" -ov -format UDZO "$DMG"

echo "Done: $DMG"

# NOTE: this is an ad-hoc signature, not a Developer-ID + notarized one. On a
# machine that downloaded the .dmg from the internet, Gatekeeper quarantines it;
# the user must either right-click -> Open once, or run:
#     xattr -dr com.apple.quarantine /Applications/CAMM3.app
# Proper distribution requires signing with a Developer ID and notarization.
