# The CAMM3 application executable. Thin entry point; all logic lives in
# camm3_ui / camm3_core.
qt_add_executable(CAMM3
    MANUAL_FINALIZATION
    main.cpp
)

target_link_libraries(CAMM3 PRIVATE camm3_ui)

set_target_properties(CAMM3 PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
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
