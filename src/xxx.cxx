///	@file
///	@brief		xxx common library.
///	@details	Utilities.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#include <xxx/xxx.hxx>

//	@name	xxx
namespace xxx {

std::uint32_t
get_version() noexcept
{
	return xxx_version;
}

}	// namespace xxx

// ---------------------------------------------------------------------------
// Macro definitions for document

///	@name	Platform-dependent implementation.
///		Only single definition is allowed to define.
///		Several features are required one of the following:
///	@{

#ifndef xxx_win32

///	@brief	Win32 based implementation (defined in CMake)
#define xxx_win32
#endif
#ifndef xxx_posix

///	@brief	POSIX based implementation (defined in CMake)
#define xxx_posix
#endif

///	@}

/// @name	Configurations
///	@{

///	@brief	Whether to use out of standard depent on each platform or not.
///	@details
///		If this macro is defined before compiling logger.cxx,
///		this logger uses standard C++ features only;
///		otherrise, the logger uses platform-depended extensions.
#define	xxx_standard_cpp_only

///	@brief	Whether to log or not
///	@details
///		If this macro is defined before including logger.hxx, this logger is ignored;
///		otherwise, the logger is available.
#define	xxx_no_logging

///	@brief	Whether to use ANSI escape sequence or not
///	@details
///		If this macro is defined before compiling logger.cxx,
///		this logger does not use ANSI escape sequences to color characters on console;
///		otherwise, the logger uses colored output on console.
#define	xxx_no_ansi_escape_sequence

///	@brief	Whether multibyte srtring is UTF-8 or not
///	@details
///		If this macro is defined before including uc.hxx,
///		this utility does not assume multibyte string is UTF-8;
///		otherwise, the utility assumes mullibyte string is already UTF-8.
#define xxx_non_utf8_mbs

///	@}
