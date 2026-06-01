# Packaging CAMM3

These helpers turn a built tree into a distributable artifact per OS using the Qt
deploy tools (`macdeployqt`, `windeployqt`, `linuxdeployqt`). They run **locally**
or in the CI release job — they are not part of the normal `build` workflow,
which only builds, tests, and uploads the raw binary (that raw binary does **not**
run on a machine without Qt installed — Windows reports a missing `Qt6Gui.dll`,
macOS is missing its frameworks).

## CI: the `release` workflow

[`.github/workflows/release.yml`](../.github/workflows/release.yml) runs these
scripts on every platform and uploads the self-contained artifacts (`.dmg`,
`.zip`, `.AppImage`). It triggers on:

- **a tag push** matching `v*` — it also creates a GitHub Release and attaches
  the three artifacts. Cut a release with:

  ```sh
  git tag v0.1.0
  git push origin v0.1.0
  ```

- **a manual run** (Actions → release → "Run workflow") — builds and packages;
  the artifacts are downloadable from the run summary, no Release is created.

## Prerequisites

1. A completed Release build, i.e. you have already run:

   ```sh
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

   The executable lands at:
   - macOS:   `build/app/CAMM3.app`
   - Windows: `build/app/CAMM3.exe`
   - Linux:   `build/app/CAMM3`

2. The **Qt `bin` directory on your `PATH`** (or pass it explicitly to the
   scripts). This is the directory that contains `macdeployqt` / `windeployqt` /
   `qmake`, e.g. `~/Qt/6.8.2/macos/bin`, `C:\Qt\6.8.2\msvc2022_64\bin`, or
   `~/Qt/6.8.2/gcc_64/bin`. The deploy tools must come from the **same** Qt that
   built the app.

The scripts are parameterized by the Qt bin dir: pass it as the first argument,
otherwise they fall back to whatever is on `PATH`.

## macOS

```sh
packaging/macos.sh [QT_BIN_DIR]
```

Runs `macdeployqt` to bundle the Qt frameworks, **ad-hoc code-signs** the bundle
(mandatory on Apple Silicon — an unsigned binary is killed on launch), then wraps
it in `build/app/CAMM3.dmg`.

> **Gatekeeper:** the ad-hoc signature is not a Developer-ID + notarized one, so a
> `.dmg` downloaded from the internet is quarantined. The user must right-click →
> Open once, or run `xattr -dr com.apple.quarantine /Applications/CAMM3.app`.
> Proper distribution needs an Apple Developer ID and notarization.

Manual equivalent:

```sh
~/Qt/6.8.2/macos/bin/macdeployqt build/app/CAMM3.app
codesign --force --deep --sign - build/app/CAMM3.app
hdiutil create -volname CAMM3 -srcfolder build/app/CAMM3.app -ov -format UDZO build/app/CAMM3.dmg
```

## Windows

```powershell
packaging\windows.ps1 [-QtBinDir <path>]
```

Runs `windeployqt` against `build\app\CAMM3.exe` to copy the required Qt DLLs and
plugins next to the .exe, then zips `build\app` into `build\CAMM3-windows.zip`.

Manual equivalent:

```powershell
C:\Qt\6.8.2\msvc2022_64\bin\windeployqt.exe --release --compiler-runtime build\app\CAMM3.exe
Compress-Archive -Path build\app\* -DestinationPath build\CAMM3-windows.zip
```

> **NSIS installer (optional):** for a real installer instead of a zip, point an
> [NSIS](https://nsis.sourceforge.io/) `.nsi` script at the `windeployqt`-populated
> `build\app` folder. Not required for distribution — the zip is sufficient.

## Linux (AppImage)

```sh
packaging/linux.sh [QT_BIN_DIR]
```

Uses [`linuxdeployqt`](https://github.com/probonopd/linuxdeployqt) to bundle Qt and
produce a portable `CAMM3-x86_64.AppImage`. `linuxdeployqt` is not shipped with Qt;
download the AppImage release and either put it on your `PATH` or set
`LINUXDEPLOYQT` to its path before running the script.

`linuxdeployqt` needs `qmake` discoverable so it can locate the matching Qt — this
is why the Qt bin dir must be on `PATH` (the script prepends `QT_BIN_DIR` for you).

Manual equivalent:

```sh
export PATH="$HOME/Qt/6.8.2/gcc_64/bin:$PATH"   # so linuxdeployqt finds qmake
cmake --install build --prefix AppDir/usr        # or copy build/app/CAMM3 manually
linuxdeployqt AppDir/usr/share/applications/CAMM3.desktop -appimage
```

> A `.desktop` file and icon are required by `linuxdeployqt` to build the AppImage.
> The script generates a minimal one if none exists. AppImages built this way are
> portable across most modern x86_64 Linux distributions.
