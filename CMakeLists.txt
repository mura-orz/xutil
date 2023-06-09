# xxx
# (C) 2018-, Mura, All rights reserved.

cmake_minimum_required (VERSION 3.13)
enable_language(CXX)
set(CMAKE_CXX_STANDARD			20)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

find_package(Threads		REQUIRED)	
cmake_policy(SET			CMP0076		NEW)	# converts relative paths to absolute

##############################################################################
# Configurations

#set(POSIX	$<PLATFORM_ID:Linux>)
set(POSIX	$<NOT:$<PLATFORM_ID:Windows>>)
set(WIN32	$<PLATFORM_ID:Windows>)

set(VC		$<CXX_COMPILER_ID:MSVC>)
set(GC		$<CXX_COMPILER_ID:GNU,Clang>)

set(LANG
	$<${VC}:/GR /GL /GS /Zc:__cplusplus>
	$<${GC}:-frtti -fexceptions>
)
set(VALIDATOR
	$<${VC}:/W4>
	$<${GC}:-Wall -Wextra -pedantic>
)
set(OPTIMIZER
	$<$<AND:${VC},$<CONFIG:Release>>:/O2 /MT>
	$<$<AND:${VC},$<CONFIG:Debug>>:/Od /MTd /Zi>
	$<$<AND:${VC},$<CONFIG:MinSizeRel>>:/O1 /MT>
	$<$<AND:${VC},$<CONFIG:RelWithDebgInfo>>:/O2 /MT /Zi>
	$<$<AND:${GC},$<CONFIG:Release>>:-O3>
	$<$<AND:${GC},$<CONFIG:Debug>>:-O0>
	$<$<AND:${GC},$<CONFIG:MinSizeRel>>:-Os -g>
	$<$<AND:${GC},$<CONFIG:RelWithDebgInfo>>:-O3 -g>
)
set(DEBUG_LINK
	$<$<AND:${VC},$<CONFIG:Debug>>:/DEBUG>
)
set(GCLIB
	$<${GC}:stdc++fs Threads::Threads>
)

##############################################################################

project (xxx)

add_library					(xxx)
target_sources				(xxx	PRIVATE
	src/xxx.cxx
	src/logger.cxx
	src/config.cxx
	src/sig.cxx
)
target_sources				(xxx	PUBLIC
	xxx/xxx.hxx
	xxx/exceptions.hxx
	xxx/finally.hxx
	xxx/logger.hxx
	xxx/config.hxx
	xxx/files.hxx
	xxx/redux.hxx
	xxx/queue.hxx
	xxx/sig.hxx
)
target_compile_features		(xxx	PUBLIC		cxx_std_20)
target_compile_options		(xxx	PUBLIC		${VALIDATOR} ${OPTIMIZER} ${LANG})
target_compile_definitions	(xxx	PUBLIC
	$<$<CONFIG:Debug>:			_DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:	NDEBUG>
	$<${VC}:					_CRT_SECURE_NO_WARNINGS>
	$<${POSIX}:					xxx_posix>
	$<${WIN32}:					xxx_win32>
)
target_include_directories	(xxx	PUBLIC		.)
target_link_libraries		(xxx	INTERFACE	${GCLIB})

add_subdirectory			(test)
add_subdirectory			(samples)
