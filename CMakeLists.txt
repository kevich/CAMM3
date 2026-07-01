cmake_minimum_required(VERSION 3.21)

project(CAMM3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 5 or Qt 6. Every modern platform builds against Qt 6; the Windows 7
# artifact builds against Qt 5.15 (the last Qt line that supports Win7 — Qt 6
# requires Windows 10 and crashes with 0xC0000005 on launch under Win7).
# Discover whichever major version is installed, preferring Qt 6, then pin a
# sensible minimum for that line. Core/Widgets/LinguistTools/Test are required;
# SerialPort is optional so the project still configures where it is absent. The
# SerialTransport (Chunk 5) is guarded on Qt${QT_VERSION_MAJOR}SerialPort_FOUND.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools Test)
if(QT_VERSION_MAJOR EQUAL 6)
    set(CAMM3_QT_MIN 6.5)
else()
    set(CAMM3_QT_MIN 5.15)
endif()
find_package(Qt${QT_VERSION_MAJOR} ${CAMM3_QT_MIN} REQUIRED COMPONENTS Core Widgets LinguistTools Test)
find_package(Qt${QT_VERSION_MAJOR} ${CAMM3_QT_MIN} QUIET OPTIONAL_COMPONENTS SerialPort)

if(Qt${QT_VERSION_MAJOR}SerialPort_FOUND)
    message(STATUS "Qt${QT_VERSION_MAJOR} SerialPort found: SerialTransport will be built.")
else()
    message(WARNING "Qt${QT_VERSION_MAJOR} SerialPort NOT found: SerialTransport is disabled. "
                    "Install it via the Qt Maintenance Tool to enable serial output.")
endif()

# Qt 5 lacks the qt_add_executable / qt_finalize_executable helpers Qt 6 uses for
# automatic finalization (plugin import, deployment wiring). Provide thin shims so
# the per-target CMakeLists below stay version-agnostic. Under Qt 6 the real
# helpers are used as-is.
if(QT_VERSION_MAJOR LESS 6)
    macro(qt_add_executable _target)
        set(_camm3_exe_args "${ARGN}")
        list(REMOVE_ITEM _camm3_exe_args MANUAL_FINALIZATION)
        add_executable(${_target} ${_camm3_exe_args})
    endmacro()
    macro(qt_finalize_executable)
    endmacro()
endif()

enable_testing()

add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(app)
add_subdirectory(tests)

# Translations (Chunk 11 fills the .ts content). The lupdate/lrelease wiring
# lives in app/CMakeLists.txt, alongside the CAMM3 target it attaches to — the
# Qt 5 path generates resource sources that must resolve in the target's own
# directory scope.
