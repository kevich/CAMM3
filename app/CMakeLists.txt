# The CAMM3 application executable. Thin entry point; all logic lives in
# camm3_ui / camm3_core.
# Application icon, wired per platform:
#   - resources.qrc embeds the PNG so main.cpp can set the runtime window/taskbar
#     icon (QApplication::setWindowIcon) on every platform.
#   - macOS: the .icns is copied into the bundle's Resources and named via
#     MACOSX_BUNDLE_ICON_FILE so Finder/Dock show it.
#   - Windows: the .rc compiles the .ico into the .exe so Explorer/taskbar show it.
set(app_sources
    main.cpp
    resources.qrc
)
if(APPLE)
    set(app_icon_macos ${CMAKE_CURRENT_SOURCE_DIR}/icons/CAMM3.icns)
    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND app_sources ${app_icon_macos})
elseif(WIN32)
    list(APPEND app_sources ${CMAKE_CURRENT_SOURCE_DIR}/icons/CAMM3.rc)
endif()

qt_add_executable(CAMM3
    MANUAL_FINALIZATION
    ${app_sources}
)

target_link_libraries(CAMM3 PRIVATE camm3_ui)

set_target_properties(CAMM3 PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE CAMM3.icns
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CAMM3
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(CAMM3)
