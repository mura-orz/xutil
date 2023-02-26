///	@file
///	@brief		Main entry of test.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved.

#include <xxx/xxx.hxx>
#include <xxx/exceptions.hxx>
#include <xxx/finally.hxx>
#include <xxx/logger.hxx>
#include <xxx/config.hxx>

#include <gtest/gtest.h>

#include <regex>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

TEST(test_cpp, Initialize) {
	EXPECT_NO_THROW(xxx::initialize_cpp());
	EXPECT_NO_THROW(xxx::initialize_cpp(""));
	EXPECT_NO_THROW(xxx::initialize_cpp("C"));
	EXPECT_NO_THROW(xxx::initialize_cpp("foobar"));
}

TEST(test_version, Version) {
	auto const	head_version	= xxx_version;
	auto const	body_version	= xxx::get_version();

	EXPECT_EQ(head_version, body_version);	
	EXPECT_EQ(xxx::get_major_version(head_version), xxx::get_major_version(body_version));
	EXPECT_EQ(xxx::get_minor_version(head_version), xxx::get_minor_version(body_version));
	EXPECT_EQ(xxx::get_revision(head_version), xxx::get_revision(body_version));
	EXPECT_EQ(xxx::get_extra_version(head_version), xxx::get_extra_version(body_version));

	auto const	bv
		= xxx::get_major_version(body_version) << 24
		| xxx::get_minor_version(body_version) << 16
		| xxx::get_revision(body_version) << 8
		| xxx::get_extra_version(body_version);
	EXPECT_EQ(body_version, bv);
	auto const	hv
		= xxx::get_major_version(head_version) << 24
		| xxx::get_minor_version(head_version) << 16
		| xxx::get_revision(head_version) << 8
		| xxx::get_extra_version(head_version);
	EXPECT_EQ(head_version, hv);
}

TEST(test_config, Empty) {
	using namespace std::string_literals;

	xxx::config::configurations_t	config;
	EXPECT_FALSE(config.contains("key"));
	EXPECT_THROW(config.get("key"), std::out_of_range);
	EXPECT_THROW(config.get_as<std::string>("key"), std::out_of_range);
	EXPECT_THROW(config.get_as<int>("key"), std::out_of_range);
	EXPECT_EQ("baz"s, config.get_as<std::string>("key", std::string{"baz"}));
	EXPECT_EQ("baz"s, config.get_as<std::string>("key", std::move(std::string{"baz"})));
}
TEST(test_config, CopiedOptions) {
	using namespace std::string_literals;

	std::unordered_map<std::string,std::string>	options{
		{"foo", "bar"}, {"one", "1"},
		{"t", "true"}, {"f", "false"},
		{"none", ""}, {"num-alpha", "1a"}
	};
	xxx::config::configurations_t	config{options};
	EXPECT_FALSE(config.contains("key"));
	EXPECT_TRUE(config.contains("foo"));
	EXPECT_TRUE(config.contains("one"));
	EXPECT_TRUE(config.contains("t"));
	EXPECT_TRUE(config.contains("f"));
	EXPECT_TRUE(config.contains("none"));
	EXPECT_EQ("bar"s, config.get("foo"));
	EXPECT_EQ("1"s, config.get("one"));
	EXPECT_EQ("true"s, config.get("t"));
	EXPECT_EQ("false"s, config.get("f"));
	EXPECT_EQ(""s, config.get("none"));
	EXPECT_THROW(config.get_as<int>("num-alpha"), std::bad_cast);
	EXPECT_THROW(config.get("key"), std::out_of_range);
	EXPECT_EQ("bar"s, config.get_as<std::string>("foo"));
	EXPECT_EQ(""s, config.get_as<std::string>("none"));
	EXPECT_TRUE(config.get_as<bool>("one"));
	EXPECT_TRUE(config.get_as<bool>("t"));
	EXPECT_FALSE(config.get_as<bool>("f"));
	EXPECT_FALSE(config.get_as<bool>("none"));
	EXPECT_THROW(config.get_as<std::string>("key"), std::out_of_range);
	EXPECT_THROW(config.get_as<int>("none"), std::bad_cast);
	EXPECT_EQ(1, config.get_as<int>("one"));
	EXPECT_THROW(config.get_as<int>("foo"), std::bad_cast);
	EXPECT_EQ("bar"s, config.get_as<std::string>("foo", std::string{"baz"}));
	EXPECT_EQ("baz"s, config.get_as<std::string>("key", std::string{"baz"}));
	EXPECT_EQ(10, config.get_as<int>("none", 10));
	EXPECT_EQ(0, config.get_as<int>("key", 0));
	EXPECT_EQ(0, config.get_as<int>("foo", 0));
	EXPECT_EQ("bar"s, config.get_as<std::string>("foo", std::move(std::string{"baz"})));
	EXPECT_EQ("baz"s, config.get_as<std::string>("key", std::move(std::string{"baz"})));
	EXPECT_EQ(10, config.get_as<int>("none", std::move(10)));
	EXPECT_EQ(0, config.get_as<int>("key", std::move(0)));
	EXPECT_EQ(0, config.get_as<int>("foo", std::move(0)));
	EXPECT_EQ("bar"s, config.get_as("foo", "baz"s));
	EXPECT_EQ("baz"s, config.get_as("key", "baz"s));
	EXPECT_EQ(0, config.get_as("foo", 0));
	EXPECT_EQ("bar"s, config.get_as("foo", std::move("baz"s)));
	EXPECT_EQ("baz"s, config.get_as("key", std::move("baz"s)));
	EXPECT_EQ(0, config.get_as("foo", std::move(0)));
}

TEST(test_config, MovedOptions) {
	xxx::config::configurations_t	config{std::move(std::unordered_map<std::string,std::string>{{"foo", "bar"}})};
	EXPECT_TRUE(config.contains("foo"));
}

TEST(test_config, GoodOptions) {
	std::unordered_map<std::string,std::string>	options{{"1Aa.-_", "bar"}};
	EXPECT_NO_THROW(xxx::config::configurations_t	config{options});
}

TEST(test_config, BadOptions) {
	{
		std::unordered_map<std::string,std::string>	options{{" foo ", "bar"}};
		EXPECT_THROW(xxx::config::configurations_t	config{std::move(options)}, std::invalid_argument);
	}
	{
		std::unordered_map<std::string,std::string>	options{{"foo bar", "bar"}};
		EXPECT_THROW(xxx::config::configurations_t	config{std::move(options)}, std::invalid_argument);
	}
	{
		std::unordered_map<std::string,std::string>	options{{"", "bar"}};
		EXPECT_THROW(xxx::config::configurations_t	config{std::move(options)}, std::invalid_argument);
	}
	{
		std::unordered_map<std::string,std::string>	options{{"foo$bar", "bar"}};
		EXPECT_THROW(xxx::config::configurations_t	config{std::move(options)}, std::invalid_argument);
	}
	{
		std::unordered_map<std::string,std::string>	options{{"#foo$bar", "bar"}};
		EXPECT_THROW(xxx::config::configurations_t	config{std::move(options)}, std::invalid_argument);
	}
}

TEST(test_config, BadConf) {
	using namespace std::string_literals;
	EXPECT_THROW(xxx::config::configurations_t	config{"nothing.conf"}, std::ios::failure);
	{	std::ofstream	ofs{"test.conf"};	ofs	<< "a" << std::endl;	}
	EXPECT_THROW(xxx::config::configurations_t	config{"test.conf"}, std::runtime_error);
	{	std::ofstream	ofs{"test.conf"};	ofs	<< "$=a" << std::endl;	}
	EXPECT_THROW(xxx::config::configurations_t	config{"test.conf"}, std::runtime_error);
	{	std::ofstream	ofs{"test.conf"};	ofs	<< "=a" << std::endl;	}
	EXPECT_THROW(xxx::config::configurations_t	config{"test.conf"}, std::runtime_error);
	{	std::ofstream	ofs{"test.conf"};	ofs	<< " #" << std::endl;	}
	EXPECT_THROW(xxx::config::configurations_t	config{"test.conf"}, std::runtime_error);

	std::filesystem::remove("test.conf");
}

TEST(test_config, LoadOptions) {
	using namespace std::string_literals;
	{
		std::ofstream	ofs{"test.conf"};
		ofs	<< std::endl
			<< " " << std::endl
			<<	"\t" << std::endl
			<<	"#aaa" << std::endl
			<<	"a=b" << std::endl
			<<	" c = d" << std::endl
			<<	" e = f " << std::endl
			<<	"\tg\t=\th" << std::endl
			<<	"i=x" << std::endl
			<<	"i=" << std::endl
			<<	"j=\t" << std::endl;
	}
	EXPECT_NO_THROW(xxx::config::configurations_t	config{"test.conf"});
	xxx::config::configurations_t	config{"test.conf"};
	EXPECT_FALSE(config.contains("\t"));
	EXPECT_FALSE(config.contains(""));
	EXPECT_FALSE(config.contains("#aaa"));
	EXPECT_FALSE(config.contains("aaa"));
	EXPECT_EQ("b"s, config.get("a"));
	EXPECT_EQ("d"s, config.get("c"));
	EXPECT_EQ("f "s, config.get("e"));
	EXPECT_EQ("h"s, config.get("g"));
	EXPECT_EQ(""s, config.get("i"));
	EXPECT_EQ(""s, config.get("j"));

	std::filesystem::remove("test.conf");
}

TEST(test_config, Copy) {
	using namespace std::string_literals;
	xxx::config::configurations_t	config{{std::make_pair("a"s,"b"s)}};
	EXPECT_EQ("b"s, config.get("a"));
	auto const	copied	= config;
	EXPECT_EQ("b"s, copied.get("a"));
}

TEST(test_config, OptionsAndFile) {
	using namespace std::string_literals;
	{
		std::ofstream	ofs{"test.conf"};
		ofs	<<	"A=B" << std::endl;
		ofs	<<	"a=b" << std::endl;
	}
	std::unordered_map<std::string,std::string>	options{{"a"s, "c"s}, {"x"s, "z"s}};

	xxx::config::configurations_t	config1{"test.conf", options};
	EXPECT_EQ("B", config1.get("A"));
	EXPECT_EQ("c", config1.get("a"));
	EXPECT_EQ("z", config1.get("x"));

	xxx::config::configurations_t	config2{"test.conf", std::move(options)};
	EXPECT_EQ("B", config2.get("A"));
	EXPECT_EQ("c", config2.get("a"));
	EXPECT_EQ("z", config2.get("x"));

	std::filesystem::remove("test.conf");
}

namespace {

bool
contains(std::string const& s, std::regex const& re) {
	return std::regex_match(s.cbegin(), s.cend(), re);
};

}	// namespace

TEST(test_exception, Dump) {
	// To use << operator; otherwise, xxx::dump_exception() is available instead.
	using namespace	xxx::exception_iostream;
	using namespace	std::string_literals;

	// Plain
	std::exception const	e;
	std::regex const		re{R"((.|[\r\n])* \[0\] [^(]+\(.*exception(.|[\r\n])*)"s, std::regex_constants::ECMAScript | std::regex_constants::multiline};

	std::ostringstream	oss1;
	xxx::dump_exception(oss1, e);
	EXPECT_TRUE(contains(oss1.str(), re));

	std::ostringstream	oss2;
	oss2	<< e;
	EXPECT_EQ(oss1.str(), oss2.str());
	EXPECT_TRUE(contains(oss2.str(), re));
}
TEST(test_exception, Messaged) {
	// To use << operator; otherwise, xxx::dump_exception() is available instead.
	using namespace	xxx::exception_iostream;
	using namespace	std::string_literals;

	std::ostringstream	oss;
	std::runtime_error	e{"test"};
	xxx::dump_exception(oss, e);
	EXPECT_TRUE(contains(oss.str(), std::regex{"^test(.|[\r\n])*"s, std::regex::multiline}));
	EXPECT_TRUE(contains(oss.str(), std::regex{R"((.|[\r\n])* \[0\] test \(.*runtime_error(.|[\r\n])*)", std::regex::multiline}));
}
TEST(test_exception, Nest) {
	// To use << operator; otherwise, xxx::dump_exception() is available instead.
	using namespace	xxx::exception_iostream;
	using namespace	std::string_literals;

	// Nested
	try {
		try {
			try {
				(void)std::string().at(1);	// Raise an exception forcefully.
			} catch(std::exception const& e) {
				std::throw_with_nested(e);
			}
		} catch(std::exception const& e) {
			std::throw_with_nested(e);
		}
	} catch(std::exception const& e) {
		std::ostringstream	oss;
		xxx::dump_exception(oss, e);
		EXPECT_TRUE(contains(oss.str(), std::regex{R"((.|[\r\n])* \[0\] [^(]+\(.*nested_exception(.|[\r\n])*)", std::regex::multiline}));
		EXPECT_TRUE(contains(oss.str(), std::regex{R"((.|[\r\n])* \[1\] [^(]+\(.*nested_exception(.|[\r\n])*)", std::regex::multiline}));
		EXPECT_TRUE(contains(oss.str(), std::regex{R"((.|[\r\n])* \[2\] [^(]+\(.*out_of_range(.|[\r\n])*)", std::regex::multiline}));
	}
}

TEST(test_exception, Handling) {
	int		f;
	
	f	= 0;
	xxx::ignore_exceptions([&]() {
		f	= 1;
		throw std::runtime_error(__func__);
	});
	EXPECT_EQ(1, f);
	
	f	= 0;
	xxx::ignore_exceptions([&]() {
		f	+= 1;
		throw std::runtime_error(__func__);
	}, [&f](std::exception const&) {
		f	+= 10;
		xxx::ignore_exceptions([&]() {
			f	+= 100;
		});
		f	+= 1000;
	});
	EXPECT_EQ(1111, f);

	f	= 0;
	auto	e	= xxx::suppress_exceptions([&]() {
		f	+= 1;
		throw std::runtime_error("any exception");
	});
	EXPECT_TRUE(e);
	EXPECT_FALSE(!e);
	if(e) {
		try {
			f	+= 10;
			std::rethrow_exception(e);
		} catch(std::exception const&) {
			f	+= 100;
		}
		f	+= 1000;
	}
	EXPECT_EQ(1111, f);

	f	= 0;
	e	= xxx::suppress_exceptions([&]() {
		f	+= 1;
	});
	EXPECT_TRUE(!e);
	EXPECT_FALSE(e);
	if( ! e) {
		f	+= 10;
	}
	EXPECT_EQ(11, f);
}

TEST(test_finally, finally) {
	std::filesystem::path const		path{"test2.log"};

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	{
		xxx::finally_t	finally([=](){ std::ofstream	ofs{path}; });
		EXPECT_FALSE(std::filesystem::exists(path));
	}
	EXPECT_TRUE(std::filesystem::exists(path));

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	{
		xxx::finally_t	finally(std::move([=](){ std::ofstream	ofs{path}; }));
		EXPECT_FALSE(std::filesystem::exists(path));
	}
	EXPECT_TRUE(std::filesystem::exists(path));

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	try {
		xxx::finally_t	finally([=](){ std::ofstream	ofs{path}; });
		EXPECT_FALSE(std::filesystem::exists(path));
		throw std::runtime_error("an exception occurred");
	} catch(std::exception const&) {}
	EXPECT_TRUE(std::filesystem::exists(path));

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}
}

namespace {

std::string		read_and_clear_log(std::filesystem::path const& path) {
	std::ostringstream	oss;
	{
		std::ifstream	ifs;
		ifs.exceptions(std::ios::badbit|std::ios::failbit);
		ifs.open(path);
		ifs.exceptions(std::ios::badbit);
		std::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(oss));
	}
	std::filesystem::remove(path);
	return oss.str();
}

std::regex const	oops_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[F\]\{[^:]+:[0-9_]{5}\}.*oops\n?$)"};
std::regex const	err_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[E\]\{[^:]+:[0-9_]{5}\}.*err\n?$)"};
std::regex const	warn_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[W\]\{[^:]+:[0-9_]{5}\}.*warn\n?$)"};
std::regex const	notice_re	{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[N\]\{[^:]+:[0-9_]{5}\}.*notice\n?$)"};
std::regex const	info_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[I\]\{[^:]+:[0-9_]{5}\}.*info\n?$)"};
std::regex const	trace_re	{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*trace\n?$)"};
std::regex const	debug_re	{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[D\]\{[^:]+:[0-9_]{5}\}.*debug\n?$)"};
std::regex const	verbose_re	{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[V\]\{[^:]+:[0-9_]{5}\}.*verbose\n?$)"};

}	// namespace

TEST(test_logger, Default_logger) {
	using namespace std::string_literals;
	std::filesystem::path const		path{"test.log"};
	auto& logger = xxx::log::logger("");
	logger.set_console(false);
	logger.set_path("");

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	logger.set_path(path);
	logger.oops("oops");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
	logger.set_path(path);
	logger.err("err");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), err_re));
	logger.set_path(path);
	logger.warn("warn");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), warn_re));
	logger.set_path(path);
	logger.notice("notice");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), notice_re));
	logger.set_path(path);
	logger.info("info");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), info_re));
	logger.set_path(path);
	logger.trace("trace");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.debug("debug");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.verbose("verbose");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Fatal, "oops");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Error, "err");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), err_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Warn, "warn");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), warn_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Notice, "notice");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), notice_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Info, "info");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), info_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Trace, "trace");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Debug, "debug");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Verbose, "verbose");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
}

TEST(test_logger, Default_logger_all) {
	std::filesystem::path const		path{"test.log"};
	auto& logger = xxx::log::logger("");
	logger.set_level(xxx::log::level_t::All);
	logger.set_path("");
	logger.set_console(false);

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	logger.set_path(path);
	logger.oops("oops");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
	logger.set_path(path);
	logger.err("err");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), err_re));
	logger.set_path(path);
	logger.warn("warn");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), warn_re));
	logger.set_path(path);
	logger.notice("notice");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), notice_re));
	logger.set_path(path);
	logger.info("info");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), info_re));
	logger.set_path(path);
	logger.trace("trace");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), trace_re));
	logger.set_path(path);
	logger.debug("debug");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), debug_re));
	logger.set_path(path);
	logger.verbose("verbose");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), verbose_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Fatal, "oops");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Error, "err");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), err_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Warn, "warn");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), warn_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Notice, "notice");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), notice_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Info, "info");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), info_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Trace, "trace");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), trace_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Debug, "debug");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), debug_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Verbose, "verbose");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), verbose_re));
}

TEST(test_logger, Logger_level) {
	using namespace std::string_literals;
	std::filesystem::path const		path{"test.log"};
	auto& logger = xxx::log::logger("");
	logger.set_level(xxx::log::level_t::Fatal);
	logger.set_path("");
	logger.set_console(false);

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	logger.set_path(path);
	logger.oops("oops");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
	logger.set_path(path);
	logger.err("err");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.warn("warn");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.notice("notice");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.info("info");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.trace("trace");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.debug("debug");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.verbose("verbose");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Fatal, "oops");
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Error, "err");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Warn, "warn");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Notice, "notice");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Info, "info");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Trace, "trace");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Debug, "debug");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
	logger.set_path(path);
	logger.log(xxx::log::level_t::Verbose, "verbose");
	logger.set_path("");
	EXPECT_EQ(""s, read_and_clear_log(path));
}

TEST(test_logger, Concatenate) {
	using namespace std::string_literals;
	std::filesystem::path const		path{"test.log"};
	EXPECT_EQ("arg2"s, xxx::log::concatenate("arg", 2));
}
TEST(test_logger, Enclose) {
	using namespace std::string_literals;
	EXPECT_EQ("(arg,2)"s, xxx::log::enclose("arg", 2));
}

TEST(test_logger, Trace) {
	using namespace std::string_literals;
	std::filesystem::path const		path{"test.log"};
	auto& logger = xxx::log::logger("");
	logger.set_level(xxx::log::level_t::All);

	std::regex const	info_tracer_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[I\]\{[^:]+:[0-9_]{5}\}.*>>>\n[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[I\]\{[^:]+:[0-9_]{5}\}.*<<<\n?$)"};
	std::regex const	tracer_re			{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*>>>\n[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*<<<\n?$)"};
	std::regex const	tracer_arg_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*>>>ARG\n[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*<<<\n?$)"};
	std::regex const	tracer_arg_1_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*>>>ARG\n[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*<<<\(1\)\n?$)"};
	std::regex const	tracer_arg_2_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*>>>ARG\n[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*<<<\(2\)\n?$)"};
	std::regex const	tracer_arg_3_re		{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*>>>ARG\n[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]+\+[0-9]{4}\[T\]\{[^:]+:[0-9_]{5}\}.*<<<\(3\)\n?$)"};

	logger.set_path("");
	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}
	logger.set_path(path);
	{
		xxx::log::tracer_t	l(logger);
	}
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), tracer_re));
	logger.set_path(path);
	{
		xxx::log::tracer_t	l(logger, "", xxx::log::level_t::Info);
	}
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), info_tracer_re));
	logger.set_path(path);
	{
		xxx::log::tracer_t	l(logger, "ARG");
	}
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), tracer_arg_re));
	logger.set_path(path);
	{
		xxx::log::tracer_t	l(logger, "ARG");
		l.set_result("1");
	}
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), tracer_arg_1_re));
	logger.set_path(path);
	{
		xxx::log::tracer_t	l(logger, "ARG");
		l.set_result("2"s);
	}
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), tracer_arg_2_re));
	logger.set_path(path);
	{
		xxx::log::tracer_t	l(logger, "ARG");
		l.set_result(3);
	}
	logger.set_path("");
	EXPECT_TRUE(contains(read_and_clear_log(path), tracer_arg_3_re));
	logger.set_path(path);

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}
}

TEST(test_logger, Another_logger) {
	std::filesystem::path const		path{"test2.log"};
	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}

	xxx::log::add_logger("tag", xxx::log::level_t::All, path, "", false);
	{
		auto&	logger	= xxx::log::logger("tag");

		logger.set_path(path);
		logger.oops("oops");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
		logger.set_path(path);
		logger.err("err");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), err_re));
		logger.set_path(path);
		logger.warn("warn");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), warn_re));
		logger.set_path(path);
		logger.notice("notice");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), notice_re));
		logger.set_path(path);
		logger.info("info");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), info_re));
		logger.set_path(path);
		logger.trace("trace");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), trace_re));
		logger.set_path(path);
		logger.debug("debug");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), debug_re));
		logger.set_path(path);
		logger.verbose("verbose");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), verbose_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Fatal, "oops");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), oops_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Error, "err");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), err_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Warn, "warn");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), warn_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Notice, "notice");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), notice_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Info, "info");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), info_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Trace, "trace");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), trace_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Debug, "debug");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), debug_re));
		logger.set_path(path);
		logger.log(xxx::log::level_t::Verbose, "verbose");
		logger.set_path("");
		EXPECT_TRUE(contains(read_and_clear_log(path), verbose_re));
	}
	xxx::log::remove_logger("tag");
	EXPECT_THROW(xxx::log::logger("tag"), std::invalid_argument);

	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}
}
