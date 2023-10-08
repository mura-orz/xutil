///	@file
///	@brief		xxx common library.
///	@details	Utilities.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_HXX_
#define xxx_HXX_

#include <cstdint>

///	@brief	version of xxx.
///		e.g., 0x01020304 means version 1.2.3.4.
#define xxx_version (static_cast<std::uint32_t>(0x00010100))

#if defined(WIN32) || defined(_WIN32)
#if ! defined(xxx_win32)
#define xxx_win32
#endif
#else
#if ! defined(xxx_posix)
#define xxx_posix
#endif
#endif
#if ! (defined(xxx_win32) || defined(xxx_posix))
#error "No platform is specified (xxx_win32/xxx_posix)"
#elif defined(xxx_win32) && defined(xxx_posix)
#error "Multiple platforms are specified (xxx_win32/xxx_posix)"
#endif

///	@name	xxx
///	@brief	root of xxx common library.
namespace xxx {

///	@brief	Gets built version of xxx.
///	@return		Built version of xxx.
std::uint32_t get_version() noexcept;

///	@brief	Gets major version of the @p version.
///		If backward compatibility is broken, this version is imcremented.
///	@param[in]	version		The library versions, xxx_version macro or result of xxx_version()
///	@return		Major version of the @p version.
inline std::uint32_t get_major_version(std::uint32_t version) noexcept { return static_cast<std::uint32_t>((version >> 24) & 0x000000FF); }
///	@brief	Gets minor version of the @p version.
///		If new feature with backward compatibility is added, this version is imcremented.
///		If the major version increases, this version is reset to zero.
///	@param[in]	version		The library versions, xxx_version macro or result of xxx_version()
///	@return		Minor version of the @p version.
inline std::uint32_t get_minor_version(std::uint32_t version) noexcept { return static_cast<std::uint32_t>((version >> 16) & 0x000000FF); }
///	@brief	Gets revision of the @p version.
///		If bug-fixes or trival improvements are added, this version is imcremented.
///		If the minor version increases or resets, this version is reset to zero, too.
///	@param[in]	version		The library versions, xxx_version macro or result of xxx_version()
///	@return		Revision of the @p version.
inline std::uint32_t get_revision(std::uint32_t version) noexcept { return static_cast<std::uint32_t>((version >> 8) & 0x000000FF); }
///	@brief	Gets extra version of the @p version.
///		This version is reserved for especial usage.
///	@param[in]	version		The library versions, xxx_version macro or result of xxx_version()
///	@return		Extra version of the @p version.
inline std::uint32_t get_extra_version(std::uint32_t version) noexcept { return static_cast<std::uint32_t>((version >> 0) & 0x000000FF); }

/// @brief	Initializes several standard C++ libraries.
/// @param[in]	locale		Global locale.
///							If the @p locale is null, this method does not set any locale;
///							otherwise, the method sets the @p locale up.
///							Empty string means native locale typically.
/// @throw	An exception occurred.
void initialize_cpp(char const* locale = "");

}	 // namespace xxx

#endif	  // xxx_HXX_
