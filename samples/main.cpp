///	@file
///	@brief		Main entry of sample.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2023-, Mura. All rights reserved.

#include <xxx/config.hxx>
#include <xxx/exceptions.hxx>
#include <xxx/logger.hxx>
#include <xxx/xxx.hxx>

#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <regex>
#include <sstream>
#include <tuple>
#include <vector>

namespace {

/// @brief	Gets the usage of this program.
/// @param[in]	result		Result of this program. Zero means success. Negative value means an error.
/// @return		String of the usage.
std::string
get_usage(int result = 0) {
	// TODO: This is just a sample.
	if (result == 0) {
		return "xxx sample\n (c) 2023-, Mura.\n This is just a sample.";
	} else if (0 < result) {
		return "xxx sample\n (c) 2023-, Mura.\n This is just a sample."
			   "\n[Usage] $ ./sample  {options} {arguments}"
			   "\n[Options]"
			   "\n  -h,--help   : Shows this usage."
			   "\n  -c:path     : Uses the path as configuration file. (default: /etc/xxx.conf)";
	} else {
		return "xxx sample\n (c) 2023-, Mura.\n This is just a sample."
			   "\n\nAn error occurred.";
	}
}

static char const Unexpected[]{"Unexpected exception occurred"};
}	 // namespace

int main(int ac, char** av) {
	try {
		xxx::initialize_cpp();

		auto& logger = xxx::log::logger("");
		logger.set_console(true);
		logger.set_logger("");
		logger.set_path("");
		logger.set_level(xxx::log::level_t::All);

		auto const [result, options] = xxx::config::get_options(ac, av);
		std::clog << get_usage(result) << std::endl;
		if (result != 0) {
			return result;
		}

		std::filesystem::path const path{options.contains("c") ? options.at("c").at(0) : "xxx.conf"};
		auto const					configurations = std::filesystem::exists(path)
													   ? xxx::config::configurations_t{path, options}
													   : xxx::config::configurations_t{options};

		xxx::log::tracer_t tracer{logger};

		// TODO:

		logger.info(xxx::log::cat("path:", path.string(), "(exists:", std::filesystem::exists(path) ? "true" : "false", ")"));
		logger.info(xxx::log::cat("xxx v", xxx::get_major_version(xxx::get_version()), ".", xxx::get_minor_version(xxx::get_version()), ".", xxx::get_revision(xxx::get_version()), ".", xxx::get_extra_version(xxx::get_version())));

		return 0;
	} catch (std::exception const& e) {
		xxx::suppress_exceptions([&e]() {
			auto& logger = xxx::log::logger("");
			logger.err(xxx::log::enclose(Unexpected, e.what()));
		});
	} catch (...) {
		xxx::suppress_exceptions([]() {
			auto& logger = xxx::log::logger("");
			logger.err(Unexpected);
		});
	}
	return -1;
}
