# xxx
# (C) 2018-, Mura, All rights reserved.

cmake_minimum_required (VERSION 3.13)
enable_language(CXX)
set(CMAKE_CXX_STANDARD			20)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

find_package(Threads		REQUIRED)	
cmake_policy(SET			CMP0076		NEW)	# converts relative paths to absolute

add_executable				(sample)
target_sources				(sample	PRIVATE
	main.cpp
)
target_compile_definitions	(sample	PUBLIC
	$<$<CONFIG:Debug>:			_DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:	NDEBUG>
	$<${VC}:					_CRT_SECURE_NO_WARNINGS>
	$<${POSIX}:					xxx_posix>
	$<${WIN32}:					xxx_win32>
)
target_compile_features		(sample	PRIVATE		cxx_std_20)
target_compile_options		(sample	PRIVATE		${VALIDATOR} ${OPTIMIZER} ${LANG})
target_include_directories	(sample	PRIVATE		"..")
target_link_libraries		(sample	PRIVATE		xxx)
