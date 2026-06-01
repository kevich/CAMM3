# CAMM3

Cross-platform (macOS / Linux / Windows) Qt 6 / C++ rewrite of the legacy 2012
VB.NET CAMM2 application that drives a Roland CAMM-2 vinyl cutter.

It opens HPGL `.plt` / `.hgl` files, previews them with scale / rotate / zoom,
and streams `PU`/`PD` cut commands to the cutter over a USB-serial port, a
Windows LPT port, or to a file.

> DXF import from the original is intentionally dropped — it was never wired up
> in the legacy app.

## Project layout

```
CAMM3/
├── core/    # camm3_core static lib — Qt Core only, no Widgets, fully unit-testable
│            #   Geometry, PlotJob, HpglParser, HpglEmitter, Settings, transport/
├── ui/      # camm3_ui static lib — Widgets: MainWindow, PlotView, SettingsDialog
├── app/     # CAMM3 executable — thin entry point
├── tests/   # Qt Test unit tests (ctest)
└── i18n/    # CAMM3_ru.ts / CAMM3_en.ts translations
```

## Prerequisites

- **Qt 6.5+** with the `Widgets`, `LinguistTools`, and `Test` modules.
  `SerialPort` is optional — without it the project still builds, but serial
  output is disabled (install it via the Qt Maintenance Tool to enable).
- **CMake 3.21+** and a C++17 compiler (Ninja recommended).

## Build

The repository assumes a Qt install such as `~/Qt/6.x.y/<platform>`. Point
CMake at it via `CMAKE_PREFIX_PATH`.

### macOS / Linux

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH="$HOME/Qt/6.11.1/macos"   # adjust version/platform
cmake --build build
ctest --test-dir build --output-on-failure
./build/app/CAMM3.app/Contents/MacOS/CAMM3      # macOS
# ./build/app/CAMM3                              # Linux
```

### Windows

```pwsh
cmake -S . -B build -G Ninja `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/msvc2022_64"
cmake --build build
ctest --test-dir build --output-on-failure
.\build\app\CAMM3.exe
```

## Status

Bootstrapped (Chunk 0). Domain logic, preview, transports, settings dialog,
i18n, and packaging are tracked as subsequent chunks in the implementation plan.
