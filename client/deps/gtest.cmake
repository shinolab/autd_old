
cmake_minimum_required(VERSION 2.8)

# Google Test settings
include(ExternalProject)

ExternalProject_Add(
    GoogleTest
    URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
    CMAKE_ARGS -Dgtest_force_shared_crt=ON
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )

ExternalProject_Get_Property(GoogleTest source_dir)
set(GTEST_INCLUDE_DIR ${source_dir}/include)

# Add compiler flag for MSVC 2012
if(MSVC_VERSION EQUAL 1700)
  add_definitions(-D_VARIADIC_MAX=10)
endif()

if(MSVC)
  set(Suffix ".lib")
else()
  set(Suffix ".a")
endif()

ExternalProject_Get_Property(GoogleTest binary_dir)

if(CMAKE_GENERATOR MATCHES "Makefiles")
    set(INSTALL_DIR ${binary_dir})
else()
    set(INSTALL_DIR ${binary_dir}/Debug)
endif()


add_library(gtest STATIC IMPORTED)
set_property(
    TARGET gtest
    PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${Suffix}
    )
add_library(gtest_main STATIC IMPORTED)
set_property(
    TARGET gtest_main
    PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${Suffix}
    )
