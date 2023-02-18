# xxx
# (C) 2018-, Mura, All rights reserved.

cmake_minimum_required (VERSION 3.13)
enable_language(CXX)
set(CMAKE_CXX_STANDARD			20)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

find_package(Threads		REQUIRED)	
cmake_policy(SET			CMP0076		NEW)	# converts relative paths to absolute

find_package (GTest	REQUIRED)
message("inc:  ${GTEST_IINCLUDE_DIRS}")
message("lib:  ${GTEST_BOTH_LIBRARIES}")

enable_testing()

add_executable				(test)
target_sources				(test	PRIVATE
	ut.cxx
)
target_compile_definitions	(test	PUBLIC
	$<$<CONFIG:Debug>:			_DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:	NDEBUG>
	$<${VC}:					_CRT_SECURE_NO_WARNINGS>
	$<${POSIX}:					xxx_posix>
	$<${WIN32}:					xxx_win32>
)
target_compile_features		(test	PRIVATE		cxx_std_20)
target_compile_options		(xxx	PRIVATE		${VALIDATOR} ${OPTIMIZER} ${LANG})
target_include_directories	(test	PRIVATE		".." ${GTEST_INCLUDE_DIRS})
target_link_libraries		(test	PRIVATE		xxx ${GTEST_BOTH_LIBRARIES})

add_test					(unittest	test)
