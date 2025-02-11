///	@file
///	@brief		xxx common library.
///	@details	Utilities.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#include <xxx/exceptions.hxx>
#include <xxx/logger.hxx>
#include <xxx/xxx.hxx>

#include <unordered_map>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <thread>
#include <sstream>

#if defined(xxx_standard_cpp_only)

#elif defined(xxx_win32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define STRICT
#include <Windows.h>
#elif defined(xxx_posix)
#include <syslog.h>
#else
#error "No platform is specified."
#endif

//	@name	xxx
namespace xxx {
namespace log {

#if ! defined(xxx_no_logging)

namespace {
std::regex const function_name_re{R"(^(?:|.* )(?:`?[A-Za-z_{][-A-Za-z_0-9<>'}]*::)*(~?[A-Za-z_][A-Za-z_0-9<>]*) ?\(.*$)"};
}

logger_t::logger_t(level_t level, std::filesystem::path const& path, std::string_view const logger, bool console, bool daily) :
	level_{level}, path_{}, logger_{logger}, console_{console}, daily_{}, ofs_{}, mutex_{}, file_mutex_{}, console_mutex_{} {
	set_path(path, daily);
}

void logger_t::log_(level_t level, std::optional<std::source_location> const& pos, std::string_view const message) {
	using namespace std::string_literals;

	validate_argument(level != level_t::Silent && level != level_t::All);
	if (pos) {
		validate_argument(pos->file_name() != nullptr && pos->function_name() != nullptr);
	}

	if (static_cast<int>(level_) < static_cast<int>(level)) {
		return;
	}

	char const* Lv[]{"[S]", "[F]", "[E]", "[W]", "[N]", "[I]", "[D]", "[T]", "[V]", "[A]"};

	auto const filename{pos ? std::filesystem::path{pos->file_name()}.filename().string() : ""s};

	std::ostringstream oss;

	std::tm lt{};
	{
		using namespace std::chrono_literals;

		auto const now = std::chrono::system_clock::now();
		get_local_now_(now, lt);
		auto const ms = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1s;

		oss << std::put_time(&lt, "%FT%T.")
			<< std::setfill('0') << std::setw(6) << ms.count()
			<< std::put_time(&lt, "%z")
			<< Lv[static_cast<int>(level)];
		oss << std::setfill('_') << std::setw(5) << std::hex << std::uppercase << std::this_thread::get_id() << std::dec;
		if (pos) {
			std::cmatch result;
			oss << "{" << filename << ':' << std::setw(5) << std::setfill('_') << pos->line() << "} ";
			oss << (std::regex_match(pos->function_name(), result, function_name_re) ? result.str(1) : pos->function_name()) << " ";
		}
		oss << message;
	}
	auto const str{oss.str()};

	if (console_) {
		ignore_exceptions([this, &str, level]() {
#if ! defined(xxx_no_ansi_escape_sequence)
			std::string begin;
			switch (level) {
			case level_t::Fatal: begin = "\x1b[37m\x1b[41m"s; break;	// Red (reversed)
			case level_t::Error: begin = "\x1b[31m"s; break;			// Red
			case level_t::Warn: begin = "\x1b[33m"s; break;				// Yellow
			case level_t::Notice: begin = "\x1b[32m"s; break;			// Green
			case level_t::Info: begin = "\x1b[37m"s; break;				// White
			case level_t::Debug: begin = "\x1b[35m"s; break;			// Magenta
			case level_t::Trace: begin = "\x1b[34m"s; break;			// Blue
			case level_t::Verbose: begin = "\x1b[36m"s; break;			// Cyan
			default: begin = "\x1b[0m"s; break;
			}

			auto const end{"\x1b[0m"s};

			std::lock_guard lock{console_mutex_};

			std::clog << begin << str << end << std::endl;
#else
			std::lock_guard lock{console_mutex_};

			std::clog << str << std::endl;
#endif
		});
	}
	if (! path_.empty()) {
		ignore_exceptions([&str, &lt, this]() {
			std::lock_guard lock{file_mutex_};

			if (needs_rotation(lt)) {
				// Rotates previous log file if necessary.
				// Closes current log file if exists.
				if (ofs_.is_open()) {
					ofs_.close();
				}
				ofs_.clear();

				std::filesystem::rename(path_, get_previous_path_());
				open_logfile_(path_, daily_);
			}
			daily_ = lt;

			if (ofs_.is_open()) {
				ofs_ << str << '\n';
				// It does not use std::endl() for performance
				// because the std::endl flushes stream, too.
				// But as the result, all the logs would Not be stored at this program crushed.
			}
		});
	}
	if (! logger_.empty()) {
		ignore_exceptions([&str, level, this]() {
#if defined(xxx_standard_cpp_only)

#elif defined(xxx_win32)
			std::lock_guard lock{mutex_};

			::OutputDebugStringA(("[" + logger_ + "] " + str + "\r\n").c_str());
#elif defined(xxx_posix)
			int lv;
			switch (level) {
			case level_t::Fatal: lv = LOG_CRIT; break;
			case level_t::Error: lv = LOG_ERR; break;
			case level_t::Warn: lv = LOG_WARNING; break;
			case level_t::Notice: lv = LOG_NOTICE; break;
			case level_t::Info: lv = LOG_INFO; break;
			case level_t::Debug: lv = LOG_DEBUG; break;
			case level_t::Trace: lv = LOG_DEBUG; break;
			case level_t::Verbose: lv = LOG_DEBUG; break;
			default: lv = LOG_CRIT; break;
			}

			std::lock_guard lock{mutex_};

			::openlog(logger_.c_str(), LOG_PID, LOG_USER);
			::syslog(lv, "%s", str.c_str());
#else
		// unsupported platform.
#endif
		});
	}
}

void logger_t::get_local_now_(std::chrono::system_clock::time_point const& now, std::tm& tm) const {
	auto const tt{std::chrono::system_clock::to_time_t(now)};
#if defined(xxx_win32)
	::localtime_s(&tm, &tt);
#elif defined(xxx_posix)
	::localtime_r(&tt, &tm);
#else
	{
		std::lock_guard lock{mutex_};
		tm = *std::localtime(&tt);
	}
#endif
}

std::filesystem::path
logger_t::get_previous_path_() const {
	if (! daily_) throw std::logic_error(__func__);

	std::ostringstream oss;
	oss << path_.filename().string() << std::put_time(&*daily_, "%Y%m%d");
	std::string const base = oss.str();

	std::filesystem::path path = path_;
	path.replace_filename(base);

	auto const MAX_RETRY = 10u;	   // It is a magic number.
	for (auto n = 0u; std::filesystem::exists(path); ++n) {
		path.replace_filename(base + "." + std::to_string(n));

		if (MAX_RETRY < n) throw std::runtime_error(__func__);
	}

	return path;
}

void logger_t::open_logfile_(std::filesystem::path const& path, std::optional<std::tm> const& lt) {
	if (ofs_.is_open()) throw std::logic_error(__func__);

	// Updates path.
	path_ = path;
	if (path_.empty()) {
		// Stops log file.
		daily_ = std::nullopt;	  // ignores the lt parameter.
		return;
	}

	// Opens a new file.
	try {
		ofs_.exceptions(std::ios::badbit | std::ios::failbit);
		ofs_.open(path, std::ios::app | std::ios::binary);
		ofs_.exceptions(std::ios::badbit);

		daily_ = lt;	// Stores today or nullopt.
	} catch (...) {
		path_.clear();
		daily_ = std::nullopt;
		throw;	  // Don't take care of file stream here.
	}
}

void logger_t::set_path(std::filesystem::path const& path, bool daily) {
	std::lock_guard l{file_mutex_};

	// Closes current log file once if exists.
	if (ofs_.is_open()) {
		ofs_.close();
	}
	ofs_.clear();

	// Gets current time.
	auto const now = std::chrono::system_clock::now();
	std::tm	   lt{};
	get_local_now_(now, lt);

	// rotates previous log file if necessary.
	if (needs_rotation(lt)) {
		std::filesystem::rename(path_, get_previous_path_());
	}
	// Opens a new log file if the path is not empty.
	if (daily) {
		open_logfile_(path, lt);
	} else {
		open_logfile_(path, std::nullopt);
	}
}

std::once_flag																logger_once_s;
std::mutex																	loggers_mutex_s;
std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<logger_t>>> loggers_s;
// static instance of std::unordered_map might crash if this logger is used in another static instance.

void add_logger(std::string const& tag, level_t level, std::filesystem::path const& path, std::string_view const logger, bool console) {
	validate_argument(! tag.empty());

	std::lock_guard lock{loggers_mutex_s};

	std::call_once(logger_once_s, []() {
		std::lock_guard lock{loggers_mutex_s};
		if (! loggers_s) loggers_s = std::make_unique<std::unordered_map<std::string, std::unique_ptr<logger_t>>>();
		if (! loggers_s->contains("")) { loggers_s->insert(std::make_pair("", std::make_unique<logger_t>())); }
	});

	auto itr{loggers_s->find(tag)};
	validate_argument(itr == std::end(*loggers_s));

	loggers_s->insert(std::make_pair(tag, std::make_unique<logger_t>(level, path, logger, console)));
}

void remove_logger(std::string const& tag) {
	validate_argument(! tag.empty());

	std::call_once(logger_once_s, []() {
		std::lock_guard lock{loggers_mutex_s};
		if (! loggers_s) loggers_s = std::make_unique<std::unordered_map<std::string, std::unique_ptr<logger_t>>>();
		if (! loggers_s->contains("")) { loggers_s->insert(std::make_pair("", std::make_unique<logger_t>())); }
	});

	std::lock_guard lock{loggers_mutex_s};

	auto itr{loggers_s->find(tag)};
	validate_argument(itr != std::end(*loggers_s));
	loggers_s->erase(itr);
}

logger_t& logger(std::string const& tag) {
	std::call_once(logger_once_s, []() {
		std::lock_guard lock{loggers_mutex_s};
		if (! loggers_s) loggers_s = std::make_unique<std::unordered_map<std::string, std::unique_ptr<logger_t>>>();
		if (! loggers_s->contains("")) { loggers_s->insert(std::make_pair("", std::make_unique<logger_t>())); }
	});

	std::lock_guard lock{loggers_mutex_s};

	auto itr{loggers_s->find(tag)};
	validate_argument(itr != std::end(*loggers_s));
	return *itr->second;
}

#endif	  // xxx_no_logging

}
}	 // namespace xxx::log
