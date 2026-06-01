# Unit tests (Qt Test). One executable per *Test.cpp file, each linking camm3_core.
#
# Globbed with CONFIGURE_DEPENDS so a chunk only needs to DROP a `FooTest.cpp`
# here — no edits to this file (keeps parallel subagents from colliding). The
# test name == file stem, so `ctest -R Foo` works.
#
# Test data (e.g. the sample HPGL file) lives in tests/data and is reachable via
# the CAMM3_TEST_DATA_DIR compile definition.

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*Test.cpp")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    qt_add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE camm3_core Qt6::Test)
    target_compile_definitions(${test_name} PRIVATE
        CAMM3_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
