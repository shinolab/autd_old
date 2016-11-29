cmake_minimum_required(VERSION 3.0)

project(autd3)
set(VERSION_MAJOR   3   CACHE STRING "Project major version number.")
set(VERSION_MINOR   1   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

ADD_CUSTOM_TARGET(cpplint
  python
  ${PROJECT_SOURCE_DIR}/deps/cpplint.py
  --filter=-runtime/rtti,-build/include_order,-build/c++11
  --linelength=150
  ${PROJECT_SOURCE_DIR}/lib/*
  )

# ADS
add_subdirectory(deps/BeckhoffADS)

add_subdirectory(lib)
add_subdirectory(doc)
add_subdirectory(example)
add_subdirectory(cython)

# test
if(WITH_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
