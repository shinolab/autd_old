set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps)

find_package( Cython )

if (Cython_FOUND)

	include(UseCython)

	file( COPY ./ ${PROJECT_SOURCE_DIR}/example/ DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
	  FILES_MATCHING PATTERN "*.py" )

	# If the pyx file is a C++ file, we should specify that here.
	set_source_files_properties(
	  ${CMAKE_CURRENT_SOURCE_DIR}/autd3.pxd
	  ${CMAKE_CURRENT_SOURCE_DIR}/pyautd3.pyx
	  ${CMAKE_CURRENT_SOURCE_DIR}/shared_ptr.pxd
	  PROPERTIES CYTHON_IS_CXX TRUE )

  set(Matlab_INCLUDE_DIRS "")
	find_package( Eigen3 REQUIRED )
	find_package( Boost REQUIRED )
	find_package( Threads REQUIRED )
	find_package( MATLAB )

	include_directories( ${PROJECT_SOURCE_DIR}/include ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${Ads_INCLUDE_DIR} ${Matlab_INCLUDE_DIRS})

	# Multi-file cython modules do not appear to be working at the moment.
	cython_add_module( pyautd3 pyautd3.pyx autd3.pxd eigen.pxd)
	target_link_libraries(pyautd3 autd3)

endif()
