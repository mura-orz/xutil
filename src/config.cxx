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
#include <algorithm>
#include <numeric>
#include <stdexcept>

#if __has_include(<ranges>)
#include <ranges>
#endif

namespace xxx::config {

inline bool
is_invalid_key(std::string_view const key) {
	std::regex const	key_re{ R"(^[_.A-Za-z][-_.A-Za-z0-9]*$)" };
	return ! std::regex_match(key.cbegin(), key.cend(), key_re);
}

void
configurations_t::validate_keys(std::unordered_map<std::string, std::string> const& options) {
#if __has_include(<ranges>)
	auto const	keys	= options | std::views::keys;
	if (auto const itr = std::ranges::find_if(keys, is_invalid_key); itr != std::ranges::cend(keys)) {
		throw std::invalid_argument(__func__);
	}
#else
	using std::cbegin, std::cend;
	if (auto const itr = std::find_if(cbegin(options), cend(options), [](auto const& kv) { return is_invalid_key(kv.first); }); itr != cend(options)) {
		throw std::invalid_argument(__func__);
	}
#endif
}

configurations_t::configurations_t(options_t const& options) : config_{options} {
	validate_keys(config_);
}

configurations_t::configurations_t(options_t&& options) : config_{std::move(options)} {
	validate_keys(config_);
}

configurations_t::configurations_t(std::filesystem::path const& path) : config_{} {
	load_file(config_, path);
}

configurations_t::configurations_t(std::filesystem::path const& path, options_t const& options) : config_{options} {
	validate_keys(config_);
	decltype(config_)	config;
	load_file(config, path);
	config_.merge(config);
}

configurations_t::configurations_t(std::filesystem::path const& path, options_t&& options) : config_{std::move(options)} {
	validate_keys(config_);
	decltype(config_)	config;
	load_file(config, path);
	config_.merge(config);
}

void
configurations_t::load_file(options_t& config, std::filesystem::path const& path) {
	// Opens the file
	std::ifstream	ifs;
	ifs.exceptions(std::ios::badbit|std::ios::failbit);
	ifs.open(path);		// would throw an exception if failed.
	ifs.exceptions(std::ios::badbit);

	// Reads lines.
	std::regex const	re{ R"(^[ \t\v]*([_.A-Za-z][-_.A-Za-z0-9]*)[ \t\v]*=[ \t\v]*(.*)$)" };
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
		config[key]	= value;	// It overrides previous value if the key already has existed.
	}
}

bool
configurations_t::contains(std::string const& key) const noexcept {
	return config_.contains(key);
}

std::unordered_set<std::string>
configurations_t::keys() const {
#if __has_include(<ranges>)
	auto const	keys	= config_ | std::views::keys | std::views::common;
	return std::unordered_set(std::ranges::cbegin(keys), std::ranges::cend(keys));
#else
	std::unordered_set<std::string>		keys;
	std::transform(config_.cbegin(), config_.cend(), std::inserter(keys, keys.end()),
		[](auto const& a){ return a.first; });
	return keys;
#endif
}

std::tuple<int, std::unordered_multimap<std::string, std::string>>
get_options(int ac, char* av[]) {
	std::vector<std::string_view> const		args(av+1, av+ac);
	if (args.empty())	return { 0, {} };
	try {
#if __has_include(<ranges>)
		auto	as	= args | std::views::filter([](auto const& a) { return a == "-" || !a.starts_with("-"); }) | std::views::transform([](auto const& a) {
					return std::make_pair(std::string{}, std::string{a});
				}) | std::views::common;
		auto	os	= args | std::views::filter([](auto const& a){ return a != "-" && a.starts_with("-"); }) | std::views::transform([](auto const& a) {
					std::regex const	option_re{ R"(^--?([a-zA-Z0-9]+)(?:[=:](.+))?$)" };
					if (std::cmatch result; std::regex_match(a.data(), result, option_re)) {
						return std::make_pair(std::string{result.str(1)}, std::string{result.size() < 2 ? "" : result.str(2)});
					} else throw std::invalid_argument(std::string{a});
				}) | std::views::common;
		auto		ms		= std::vector{ std::vector<std::pair<std::string, std::string>>(as.begin(), as.end()), std::vector<std::pair<std::string, std::string>>(os.begin(), os.end()) } | std::views::join;
		auto const	options	= std::accumulate(ms.begin(), ms.end(), std::move(std::unordered_multimap<std::string, std::string>{}), [](auto&& res, auto const& a) { res.insert(a); return std::move(res);  });
#else
		std::regex const									option_re{ R"(^--?([a-zA-Z0-9]+)(?:[=:](.+))?$)" };
		std::unordered_multimap<std::string, std::string>	options;
		for (auto const& a : args) {
			if (a != "-" && a.starts_with('-')) {
				if (std::cmatch result; std::regex_match(a.data(), result, option_re)) {
					options.insert(std::make_pair(result.str(1), result.size() < 2 ? "" : result.str(2)));
				} else throw std::invalid_argument(std::string{a});
			} else {
				options.insert(std::make_pair("", a));
			}
		}
#endif
		auto const		usage	= options.contains("h") || options.contains("help");
		return { usage ? 1 : 0, options };
	} catch (std::invalid_argument const&) {
		return { -1, {} };
	}
}

}	// namespace xxx::config
