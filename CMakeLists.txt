cmake_minimum_required(VERSION 3.21)

project(CAMM3 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6 only. Core/Widgets/LinguistTools/Test are required; SerialPort is optional
# so the project still configures on machines where the SerialPort module is not
# installed. The SerialTransport (Chunk 5) is guarded on Qt6SerialPort_FOUND.
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Test)
find_package(Qt6 6.5 QUIET OPTIONAL_COMPONENTS SerialPort)

if(Qt6SerialPort_FOUND)
    message(STATUS "Qt6 SerialPort found: SerialTransport will be built.")
else()
    message(WARNING "Qt6 SerialPort NOT found: SerialTransport is disabled. "
                    "Install it via the Qt Maintenance Tool to enable serial output.")
endif()

enable_testing()

add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(app)
add_subdirectory(tests)

# Translations (Chunk 11 fills the .ts content; lupdate/lrelease wired here).
set(TS_FILES
    ${CMAKE_SOURCE_DIR}/i18n/CAMM3_ru.ts
    ${CMAKE_SOURCE_DIR}/i18n/CAMM3_en.ts
)
qt_add_translations(CAMM3 TS_FILES ${TS_FILES})
