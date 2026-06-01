# camm3_ui: Widgets layer (MainWindow, PlotView, SettingsDialog), linking camm3_core.
#
# Globbed with CONFIGURE_DEPENDS so UI chunks only DROP .h/.cpp/.ui files here —
# no edits to this file (keeps parallel subagents from colliding). AUTOUIC picks
# up the globbed .ui files. CMake re-globs on rebuild.
file(GLOB UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ui"
)

# Optional Qt resource bundle (icons). Added by Chunk 10; glob is empty until then.
file(GLOB UI_RESOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.qrc")

add_library(camm3_ui STATIC ${UI_SOURCES} ${UI_RESOURCES})

target_link_libraries(camm3_ui PUBLIC
    camm3_core
    Qt6::Widgets
)

# Expose ui_*.h (AUTOUIC, generated into the binary dir) and this dir to dependents.
target_include_directories(camm3_ui PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
