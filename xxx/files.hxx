///	@file
///	@brief		xxx common library.
///	@details	File utilities.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_FILES_HXX_
#define xxx_FILES_HXX_

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace xxx::file {

/// @brief	Reads the whole contents of a file.
///	tparam	T	Type of character.
/// @param[in]	path	Path of the file.
/// @param[in]	binary	Whether the file is read as binary or text mode.
/// @return		The whole contents of the file.
///	@throw		If the file does not exist, it throws an invalid argument exception.
///	@throw		If the file was failed to read, it would throws an I/O failure exception.
template<typename T = char>
inline std::basic_string<T>
read_whole(std::filesystem::path const& path, bool binary = true) {
	if (! std::filesystem::exists(path)) throw std::invalid_argument(path.string());

	auto const size = std::filesystem::file_size(path);
	if (size == 0u) return {};

	std::basic_ifstream<T> ifs;
	ifs.exceptions(std::ios::badbit | std::ios::failbit);
	ifs.open(path, binary ? std::ios::in | std::ios::binary : std::ios::in);
	ifs.exceptions(std::ios::badbit);

	std::vector<T> buffer;
	buffer.reserve(size + 1);
	std::copy(std::istreambuf_iterator<T>(ifs), std::istreambuf_iterator<T>(), std::back_insert_iterator(buffer));
	buffer.push_back(T{});
	return &buffer[0];
}

/// @brief	Read all the lines of a file.
/// @tparam T	Type of character
/// @param[in]	path	Path of the file.
/// @return 	All the lines of the file.
///	@throw		If the file does not exist, it throws an invalid argument exception.
///	@throw		If the file was failed to read, it would throws an I/O failure exception.
template<typename T>
inline std::vector<std::basic_string<T>>
read_lines(std::filesystem::path const& path) {
	auto const file = read_whole<T>(path, false);
	if (file.empty()) return {};

	std::vector<std::basic_string<T>> lines;
	std::basic_istringstream<T>		  iss{file};
	for (std::basic_string<T> line; std::getline(iss, line); /**/) {
		lines.push_back(line);
	}
	return lines;
}

}	 // namespace xxx::file

#endif	  // xxx_FILES_HXX_
