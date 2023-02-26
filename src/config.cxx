///	@file
///	@brief		xxx common library.
///	@details	Configurations.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#include <xxx/config.hxx>

#include <fstream>
#include <regex>
#include <string_view>

#if __has_include(<ranges>)
#include <ranges>
#endif

namespace xxx::config {

inline bool
is_invalid_key(std::string_view const key) {
	std::regex const	key_re{ R"(^[-_.A-Za-z0-9]+$)" };
	return ! std::regex_match(key.cbegin(), key.cend(), key_re);
}

configurations_t::configurations_t(std::unordered_map<std::string, std::string> const& options) : config_{options} {
#if __has_include(<ranges>)
	auto const	keys	= config_ | std::views::keys;
	if (auto const itr = std::ranges::find_if(keys, is_invalid_key); itr != std::ranges::cend(keys)) {
		throw std::invalid_argument(__func__);
	}
#else
	using std::cbegin, std::cend;
	if (auto const itr = std::find_if(cbegin(config_), cend(config_), [](auto const& kv) { return is_invalid_key(kv.first); }); itr != cend(config_)) {
		throw std::invalid_argument(__func__);
	}
#endif
 }

configurations_t::configurations_t(std::unordered_map<std::string, std::string>&& options) : config_{std::move(options)} {
#if __has_include(<ranges>)
	auto const	keys	= config_ | std::views::keys;
	if (auto const itr = std::ranges::find_if(keys, is_invalid_key); itr != std::ranges::cend(keys)) {
		throw std::invalid_argument(__func__);
	}
#else
	using std::cbegin, std::cend;
	if (auto const itr = std::find_if(cbegin(config_), cend(config_), [](auto const& kv) { return is_invalid_key(kv.first); }); itr != cend(config_)) {
		throw std::invalid_argument(__func__);
	}
#endif
 }

configurations_t::configurations_t(std::filesystem::path const& path) {
	// Opens the file
	std::ifstream	ifs;
	ifs.exceptions(std::ios::badbit|std::ios::failbit);
	ifs.open(path);		// would throw an exception if failed.
	ifs.exceptions(std::ios::badbit);

	// Reads lines.
	std::regex const	re{ R"(^[ \t\v]*([-_.A-Za-z0-9]+)[ \t\v]*=[ \t\v]*(.*)$)" };
	for (std::string line; std::getline(ifs, line); ) {
		if (line.empty() || line.at(0) == '#' || line.find_first_not_of(" \t\v\r\n") == std::string::npos) {
			continue;	// Skips empty or comment line.
		}

		std::smatch		result;
		if ( ! std::regex_match(line.cbegin(), line.cend(), result, re)) {
			throw std::runtime_error(line);		// Symtax error is invalid.
		}
		auto const	key		= result.str(1);
		auto const	value	= result.str(2);
		config_[key]	= value;	// It overrides previous value if the key already has existed.
	}	
}

bool
configurations_t::contains(std::string const& key) const noexcept {
	return config_.contains(key);
}

}	// namespace xxx::config
