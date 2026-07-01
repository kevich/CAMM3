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

# Translations: compile the .ts to .qm and embed them under :/i18n/ (the prefix
# main.cpp loads from). Wired here, in the CAMM3 target's own directory scope, so
# the Qt 5 path's generated resource source resolves against the target (a
# top-level scope leaves AUTOGEN unable to find the rule that produces it).
# NOTE: if a .ts file is added/renamed, re-run `cmake -S . -B build` — the Qt 5
# .qrc is generated at configure time, so `cmake --build` alone won't pick it up.
set(TS_FILES
    ${CMAKE_SOURCE_DIR}/i18n/CAMM3_ru.ts
    ${CMAKE_SOURCE_DIR}/i18n/CAMM3_en.ts
)
if(QT_VERSION_MAJOR EQUAL 6)
    # Qt 6 compiles the .ts to .qm and embeds them under :/i18n/ automatically.
    qt_add_translations(CAMM3 TS_FILES ${TS_FILES})
else()
    # Qt 5 has no qt_add_translations. Compile the .qm, generate a .qrc mapping
    # them under /i18n, and let rcc embed it. add_dependencies() forces the .qm
    # to be built before CAMM3 (hence before rcc runs on the generated .qrc).
    qt5_add_translation(CAMM3_QM_FILES ${TS_FILES})
    set(_camm3_ts_qrc "${CMAKE_CURRENT_BINARY_DIR}/camm3_translations.qrc")
    set(_camm3_qrc_body "<RCC>\n  <qresource prefix=\"/i18n\">\n")
    foreach(_camm3_qm ${CAMM3_QM_FILES})
        get_filename_component(_camm3_qm_name ${_camm3_qm} NAME)
        string(APPEND _camm3_qrc_body
            "    <file alias=\"${_camm3_qm_name}\">${_camm3_qm}</file>\n")
    endforeach()
    string(APPEND _camm3_qrc_body "  </qresource>\n</RCC>\n")
    file(WRITE ${_camm3_ts_qrc} "${_camm3_qrc_body}")
    set_source_files_properties(${CAMM3_QM_FILES} PROPERTIES GENERATED TRUE)
    qt5_add_resources(CAMM3_QM_RESOURCES ${_camm3_ts_qrc})
    set_source_files_properties(${CAMM3_QM_RESOURCES} PROPERTIES SKIP_AUTOGEN ON)
    add_custom_target(camm3_qm DEPENDS ${CAMM3_QM_FILES})
    add_dependencies(CAMM3 camm3_qm)
    target_sources(CAMM3 PRIVATE ${CAMM3_QM_RESOURCES})
endif()

qt_finalize_executable(CAMM3)
