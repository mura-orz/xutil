///	@file
///	@brief		xxx common library.
///	@details	Utilities for logger.
///				This implementation is not better for performance because of standard I/O stream.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_LOGGER_HXX_
#define xxx_LOGGER_HXX_

#include <filesystem>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <stdexcept>
#include <fstream>
#include <string>
#include <string_view>
#include <optional>
#include <ctime>

#if 201703L <= __cplusplus && __has_include(<source_location>)
#include <source_location>
#if ! defined(__cpp_lib_source_location) || __cpp_lib_source_location < 201907L
#error "std::source_location required."
#endif
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#if defined(__cpp_lib_source_location) && 201907L <= __cpp_lib_source_location
namespace std {
	using std::experimental::source_location;
}	// namespace std
#else
#error "std::source_location required."
#endif
#else
#error "std::source_location required."
#endif

#if defined(xxx_no_logging)
#include <iosfwd>
#else
#include <sstream>
#include <mutex>
#endif	// xxx_no_logging

namespace xxx {

///	@brief	logger.
namespace log {

///	@brief	logging level.
enum class level_t
{
	Silent,		///< Silent (No logging).
	Fatal,		///< Fatal error.
	Error,		///< Normal error.
	Warn,		///< Warning.
	Notice,		///< Notice.
	Info,		///< Information.
	Debug,		///< Debug.
	Trace,		///< Trace.
	Verbose,	///< Verbose.
	All,		///< All (=Verbose).
};

constexpr inline bool
is_valid_level(int level) noexcept
{
	return static_cast<int>(xxx::log::level_t::Silent) <= level && level <= static_cast<int>(xxx::log::level_t::All);
}

#if ! defined(xxx_no_logging)

namespace impl {

//	Dumps arguments.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template<typename T, typename... Args>
inline void
dump_(std::ostream& os, T const& head, Args... args)
{
	os	<< head;
	if constexpr (0 < sizeof...(Args))
	{
		dump_(os, args...);
	}
}
//	Dumps vector arguments.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template <typename T, typename... Args>
inline void
dump_(std::ostream& os, std::vector<T> const& head, Args... args)
{
	os << "[";
	for (bool first {true}; auto const& arg : head)
	{
		if (!first) os << ",";
		first = false;
		dump_(os, arg);
	}
	os << "]";
	if constexpr (0 < sizeof...(Args))
	{
		dump_(os, args...);
	}
}

//	Dumps map arguments.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template <typename T, typename V, typename... Args>
inline void
dump_(std::ostream& os, std::map<T,V> const& head, Args... args)
{
	os << "{";
	for (bool first{ true }; auto const& arg : head)
	{
		if (!first) os << ",";
		first = false;
		dump_(os, arg.first);
		os << ":";
		dump_(os, arg.second);
	}
	os << "}";
	if constexpr (0 < sizeof...(Args))
	{
		dump_(os, args...);
	}
}
//	Dumps set arguments.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template <typename T, typename... Args>
inline void
dump_(std::ostream& os, std::set<T> const& head, Args... args)
{
	os << "{";
	for (bool first{ true }; auto const& arg : head)
	{
		if (!first) os << ",";
		first = false;
		dump_(os, arg);
	}
	os << "}";
	if constexpr (0 < sizeof...(Args))
	{
		dump_(os, args...);
	}
}
//	Dumps unordered map arguments.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template <typename T, typename V, typename... Args>
inline void
dump_(std::ostream& os, std::unordered_map<T, V> const& head, Args... args)
{
	os << "{";
	for (bool first{ true }; auto const& arg : head)
	{
		if (!first) os << ",";
		first = false;
		dump_(os, arg.first);
		os << ":";
		dump_(os, arg.second);
	}
	os << "}";
	if constexpr (0 < sizeof...(Args))
	{
		dump_(os, args...);
	}
}
//	Dumps unordered set arguments.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template <typename T, typename... Args>
inline void
dump_(std::ostream& os, std::unordered_set<T> const& head, Args... args)
{
	os << "{";
	for (bool first{ true }; auto const& arg : head)
	{
		if (!first) os << ",";
		first = false;
		dump_(os, arg);
	}
	os << "}";
	if constexpr (0 < sizeof...(Args))
	{
		dump_(os, args...);
	}
}

#if defined(__cpp_char8_t) && 201803 <= __cpp_char8_t

template <typename T, typename... Args>
inline void
dump_(std::ostream& os, char8_t ch, Args... args)
{
	if (ch < 0x80)
	{
		os << static_cast<char>(ch);
	}
	else
	{
		os << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << static_cast<unsigned>(ch);
	}
}

template <typename T, typename... Args>
inline void
dump_(std::ostream& os, std::u8string_view const& head, Args... args)
{
	std::for_each(head.cbegin(), head.cend(), [&](auto const ch){ dump_(os, ch); });
}

#endif	// __cpp_char8_t

//	Dumps arguments with separation comma.
//	@param[in,out]	os		Output stream.
//	@param[in]		head	Head of argruments.
//	@param[in]		args	Other argument(s).
template <typename T, typename... Args>
inline void
enclose_(std::ostream& os, T const& head, Args... args)
{
	dump_(os, head);
	if constexpr (0 < sizeof...(args))
	{
		os	<< ',';
		enclose_(os, args...);
	}
}

}	// namespace impl

#endif	// xxx_no_logging

///	@brief	Encloses arguments.
///	@tparam			Args		Arguments.
///	@param[in]		args		Arguments.
///	@return		Arguments separated with comma and enclosed by parenthesis.
template<typename... Args>
inline std::string
enclose([[maybe_unused]] Args... args)
{
#if !defined(xxx_no_logging)
	std::ostringstream	oss;
	oss << '(';
	if constexpr (0 < sizeof...(args))
	{
		impl::enclose_(oss, args...);
	}
	oss	<< ')';
	return oss.str();
#else	// xxx_no_logging
	return std::string();
#endif	// xxx_no_logging
}

///	@brief	Concatenates arguments.
///	@tparam			Args		Arguments.
///	@param[in]		args		Arguments.
///	@return		string concatenated by parenthesis.
template<typename... Args>
inline std::string
concatenate([[maybe_unused]] Args... args)
{
#if !defined(xxx_no_logging)
	std::ostringstream	oss;
	if constexpr (0 < sizeof...(args))
	{
		impl::dump_(oss, args...);
	}
	return oss.str();
#else	// xxx_no_logging
	return std::string();
#endif	// xxx_no_logging
}

#if defined(xxx_no_logging)

class logger_t
{
public:
	void	log(level_t, std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	oops(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	err(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	warn(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	notice(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	info(std::string_view const, std::source_location const& pos=std::source_location::current())  {}
	void	debug(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	trace(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
	void	verbose(std::string_view const, std::source_location const& pos=std::source_location::current()) {}
public:
	void	set_logger(std::string_view const) {}
	void	set_path(std::filesystem::path const) {}
	void	set_console(bool) {}

	auto	logger()const noexcept	{ return std::filesystem::path();	}
	auto	path()const noexcept	{ return std::string();	}
	auto	console()const noexcept	{ return false;	}
public:
	logger_t(level_t, std::filesystem::path const&, std::string_view const, bool) {}
	logger_t() {}
};

#else	// xxx_no_logging

///	@brief	Logger.
class logger_t
{
public:
	///	@brief	Dumps log.
	///	@param[in]		level		Logging level.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	log(level_t level, std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level, pos, message);
	}
	///	@brief	Dumps log as fatal error.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	oops(std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Fatal, pos, message);
	}
	///	@brief	Dumps log as normal error.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	err(std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Error, pos, message);
	}
	///	@brief	Dumps log as warning.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	warn(std::string_view const messsage, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Warn, pos, messsage);
	}
	///	@brief	Dumps log as notice.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	notice(std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Notice, pos, message);
	}
	///	@brief	Dumps log as information.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	info(std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Info, pos, message);
	}
	///	@brief	Dumps log as debug info.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	debug(std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Debug, pos, message);
	}
	///	@brief	Dumps log as trace.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	trace(std::string_view const message, std::source_location const& pos=std::source_location::current())
	{
		log_(level_t::Trace, pos, message);
	}
	///	@brief	Dumps log as verbose.
	///	@param[in]		message		Log message.
	///	@param[in]		pos			Position of source.
	void
	verbose(std::string_view const message, std::source_location const& pos= std::source_location::current())
	{
		log_(level_t::Verbose, pos, message);
	}
public:
	///	@brief	Sets the external logger name as the following:
	///		- [xxx_win32]	dump to debugger (in debug mode)
	///		- [xxx_posix]	dump to syslog
	///	@param[in]		logger		Logger name
	void	set_logger(std::string_view const logger)	{ std::lock_guard	l{ mutex_ };	logger_ = logger; }
	///	@brief	Sets log file.
	///	@param[in]		path		The path of log name.
	///	@param[in]		daily		daily file or single file.
	void	set_path(std::filesystem::path const& path, bool daily=false);
	///	@brief	Sets whether dump it to standard error or not.
	///	@param[in]		on		Whether dump it to standart error or not..
	void	set_console(bool on)						{ console_	= on;	}
	///	@brief	Sets loggihng level.
	///	@param[in]		level		Logger level.
	void	set_level(level_t level)					{ level_	= level;	}

	///	@brief	Gets the external logger name.
	///	@return		External logger name.
	auto const&		logger()const noexcept	{ return logger_;	}
	///	@brief	Gets the path of log file.
	///	@return		The path of log file.
	auto const&		path()const noexcept	{ return path_;		}
	///	@brief	Gets whether dump it to standard error or not.
	///	@return		If standard error is available, it returns true;
	///				otherwise, it return false.
	auto			console()const noexcept	{ return console_;	}
	///	@brief	Gets whether log file is daily or not.
	auto			is_logfile_daily()const noexcept { return daily_; }
public:
	///	@brief	Constructor.
	///	@param[in]		level		Logger level.
	///	@param[in]		path		The path of log file.
	///	@param[in]		logger		External logger name.
	///	@param[in]		console		Whether dump it to standard error or not.
	///	@param[in]		daily		log file is daily or not..
	logger_t(level_t level, std::filesystem::path const& path, std::string_view const logger, bool console, bool daily=false);
	///	@brief	Constructor.
	logger_t() : level_{level_t::Info}, path_{}, logger_{}, console_{true}, daily_{}, ofs_{}, mutex_{}, file_mutex_{}, console_mutex_{}
	{}
private:
	void	log_(level_t level, std::optional<std::source_location> const& pos, std::string_view const message);
	void	get_local_now_(std::chrono::system_clock::time_point const& now, std::tm& tm) const;
	void	open_logfile_(std::filesystem::path const& path, std::optional<std::tm> const& lt);
	bool	needs_rotation(std::tm const& lt) const {
		return daily_ && (daily_->tm_year != lt.tm_year || daily_->tm_yday != lt.tm_yday) && std::filesystem::exists(path_);
	}
	std::filesystem::path	get_previous_path_() const;
private:
	level_t					level_;		///< Logger level.
	std::filesystem::path	path_;		///< The path of log file.
	std::string				logger_;	///< External logger name.
	bool					console_;	///< Whether dump it to standard error or not.
	std::optional<std::tm>	daily_;		///< Whether log file is daily or not.
	std::ofstream			ofs_;		///< Output file stream.
	mutable std::mutex		mutex_;		///< Mutex.
	mutable std::mutex		file_mutex_;	///< Mutex.
	mutable std::mutex		console_mutex_;	///< Mutex.
};

#endif	// xxx_no_logging

#if defined(xxx_no_logging)

inline void			add_logger(std::string_view const, level_t, std::filesystem::path const&, std::string_view const, bool) {}
inline void			remove_logger(std::string_view const) {}
inline logger_t		logger(std::string_view const) { return logger_t();	}

#else	// xxx_no_logging

///	@brief	Adds a new logger.
///	@param[in]		tag			New tag of logger.
///	@param[in]		level		Filter level.
///	@param[in]		path		The path of log file.
///	@param[in]		logger		External logger name.
///	@param[in]		console		Whether dump it to standard error or not.
void		add_logger(std::string const& tag, level_t level, std::filesystem::path const& path, std::string_view const logger, bool console);
///	@brief	Removes existing logger.
///	@param[in]		tag			Tag of logger to remove.
void		remove_logger(std::string const& tag);
///	@brief	Gets the logger.
///	@param[in]		tag			Tag of logger.
///	@return			Logger.
logger_t&	logger(std::string const& tag);

#endif	// xxx_no_logging

#if defined(xxx_no_logging)

class tracer_t
{
public:
	tracer_t(logger_t& logger, std::string const& message=std::string(), level_t level=level_t::Trace, std::source_location const& pos=std::source_location::current()) {}

	template <typename... Args>	void	trace(Args... args){}
	template<typename T>		void	set_result(T) {}
};

#else	// xxx_no_logging

///	@brief	Log tracer.
class tracer_t
{
public:
	///	@brief	Dumps log at entering into the scope.
	///	@param[in]		logger		Logger to dump.
	///	@param[in]		message		Log message.
	///	@param[in]		level		Logger level.
	///	@param[in]		pos			Position of source.
	tracer_t(logger_t& logger, std::string const& message=std::string(), level_t level=level_t::Trace, std::source_location const& pos=std::source_location::current()) :
		logger_{ logger },
		pos_{ pos },
		result_{},
		level_{ level }
	{
		logger_.log(level, ">>>" + message, pos_);
	}
	///	@brief	Dumps log at leaving from the scope.
	~tracer_t()
	{
		logger_.log(level_, "<<<" + result_, pos_);
	}
	///	@brief	Dumps log as trace level.
	///	@tparam			Args		arguments
	///	@param[in]		args		More arguments to log
	template <typename... Args>
	void
	trace(Args... args)
	{
		logger_.log(level_, pos_, "--- ", concatenate(args...));
	}
	///	@brief	Sets result of the method.
	///	@param[in]		result		Result of the method.
	template<typename T>	void	set_result(T result){	result_		= enclose(result);	}
private:
	logger_t&				logger_;	///< Logger.
	std::source_location		pos_;		///< Position of source.
	std::string				result_;	///< Result.
	level_t					level_;		///< Trace level.
private:
	tracer_t(tracer_t const&)						= delete;
	tracer_t const&		operator =(tracer_t const&)	= delete;
};

#endif	// xxx_no_logging

}	// namespace log
}	// namespace xxx

#endif	// xxx_LOGGER_HXX_
