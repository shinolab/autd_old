cmake_minimum_required(VERSION 3.0)

include(../deps/gtest.cmake)

include_directories( ${EIGEN3_INCLUDE_DIR} )
include_directories(../include)
include_directories( ${GTEST_INCLUDE_DIR} )
add_executable(geometry-test geometry.cpp)
target_link_libraries(geometry-test
	gtest
    gtest_main
    autd3
    )

add_test(
    NAME geometry
    COMMAND $<TARGET_FILE:geometry-test>
    )

