# camm3_core: pure Qt Core domain/logic library, no Widgets dependency.
# Unit-testable in isolation.
#
# Sources are globbed with CONFIGURE_DEPENDS so feature chunks only need to DROP
# .h/.cpp files into core/ (or core/transport/) — no edits to this file, which
# keeps parallel subagents from colliding here. CMake re-globs on rebuild.
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(camm3_core STATIC ${CORE_SOURCES})

target_link_libraries(camm3_core PUBLIC Qt6::Core)

# SerialTransport (Chunk 5) compiles only when the SerialPort module is present.
if(Qt6SerialPort_FOUND)
    target_link_libraries(camm3_core PUBLIC Qt6::SerialPort)
    target_compile_definitions(camm3_core PUBLIC CAMM3_HAVE_SERIALPORT=1)
else()
    target_compile_definitions(camm3_core PUBLIC CAMM3_HAVE_SERIALPORT=0)
endif()

# Public headers are included as #include "core/Foo.h" from anywhere in the tree.
target_include_directories(camm3_core PUBLIC ${CMAKE_SOURCE_DIR})
