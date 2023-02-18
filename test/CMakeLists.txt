# xxx
# (C) 2018-, Mura, All rights reserved.

cmake_minimum_required (VERSION 3.13)
enable_language(CXX)
set(CMAKE_CXX_STANDARD			20)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

find_package(Threads		REQUIRED)	
cmake_policy(SET			CMP0076		NEW)	# converts relative paths to absolute

# This unit test implementation requires C++17 feature on regex, which is supported since g++12.
# On Ubuntu 20.04 of GitHub Action, default version of g++ cannot be used here because of version 11.
# Furthermore, prebuilt binaries of the gtest package cannot used here because they are built by g++11
# of the default, too.
# Thus, it uses not the $GTEST_BOTH_LIBRARIES but gtest_all_test.cc directly instead. 
# The default version of gtest on the Ubuntu20.04 cannot be compiled by g++-12.
# So it uses current source set of gtest.
# It does not use submodule here because extra storage is necessary for change logs on gtest.
find_path (GTest_inc		src				HINTS ./googletest-main/googletest)
find_file (GTest_main		gtest_main.cc	HINTS ./googletest-main/googletest/src/)
find_file (GTest_all		gtest-all.cc	HINTS ./googletest-main/googletest/src/)
message("inc:  ${GTest_inc}")
message("main: ${GTest_main}")
message("all:  ${GTest_all}")

enable_testing()

add_executable				(test)
target_sources				(test	PRIVATE
	ut.cxx
	${GTest_all}
	${GTest_main}
)
target_compile_definitions	(test	PUBLIC
	$<$<CONFIG:Debug>:			_DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:	NDEBUG>
	$<${VC}:					_CRT_SECURE_NO_WARNINGS>
	$<${POSIX}:					xxx_posix>
	$<${WIN32}:					xxx_win32>
)
add_test					(unittest		ut.exe)
target_compile_features		(test	PRIVATE		cxx_std_20)
target_compile_options		(xxx	PRIVATE		${VALIDATOR} ${OPTIMIZER} ${LANG})
target_include_directories	(test	PRIVATE		".." ${GTest_inc})
target_link_libraries		(test	PRIVATE		xxx )
