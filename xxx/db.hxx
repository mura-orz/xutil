/// @file
/// @brief  SQLite wrapper.
///	@pre	SQLite is required.
///	@author	Mura.

#ifndef xxx_DB_HXX_
#define xxx_DB_HXX_

#include "sqlite3.h"	// SQLite

#include <string_view>
#include <type_traits>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace xxx::db::sl3 {

template<typename>
struct is_tuple : std::false_type {};
template<typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

using timestamp_t = std::chrono::system_clock::time_point;

enum class error_code_t {
	Aborted		= SQLITE_ABORT,		   // Aborted
	Interrupted = SQLITE_INTERRUPT,	   // Interrupted
	Busy		= SQLITE_BUSY,		   // Busy

	Locked = SQLITE_LOCKED,	   // Locked

	Authorization = SQLITE_AUTH,		  // Unauthorized
	Constraint	  = SQLITE_CONSTRAINT,	  // Constraint violation

	InvalidType	 = SQLITE_MISMATCH,	   // Type mismatch
	InvalidUsage = SQLITE_MISUSE,	   // Invalid state or usage
	NoMemory	 = SQLITE_NOMEM,	   // No memory
	NotFound	 = SQLITE_NOTFOUND,	   // Not found

	InvalidDB  = SQLITE_NOTADB,		 // Invalid database
	CorruptDB  = SQLITE_CORRUPT,	 // Corrupt database
	OpenFailed = SQLITE_CANTOPEN,	 // Failed to open
	DiskFull   = SQLITE_FULL,		 // Disk full
	IOError	   = SQLITE_IOERR,		 // I/O error

	LargeFileUnsupported = SQLITE_NOLFS,	// Large File Unsupported
	PermissionDenied	 = SQLITE_PERM,		// Access mode is not provided.

	UnknownError  = SQLITE_ERROR,		// Generic error
	InternalError = SQLITE_INTERNAL,	// Internal error

	OK	 = SQLITE_OK,	   // Success
	Row	 = SQLITE_ROW,	   // Row of data
	Done = SQLITE_DONE,	   // End of data
};

enum class bind_type_t {
	Static,
	Transient
};

enum class transaction_type_t {
	Deferred,
	Immediate,
	Exclusive
};

enum class prepare_flag_t {
	Persistent	   = SQLITE_PREPARE_PERSISTENT,
	Normalize	   = SQLITE_PREPARE_NORMALIZE,
	NoVirtualTable = SQLITE_PREPARE_NO_VTAB,
};

enum class db_flag_t {
	//	URI			= SQLITE_OPEN_URI,
	//	Memory		= SQLITE_OPEN_MEMORY,
	ReadOnly		 = SQLITE_OPEN_READONLY,
	ReadWrite		 = SQLITE_OPEN_READWRITE,
	Create			 = SQLITE_OPEN_CREATE,
	DeleteOnClose	 = SQLITE_OPEN_DELETEONCLOSE,
	Exclusive		 = SQLITE_OPEN_EXCLUSIVE,
	AutoProxy		 = SQLITE_OPEN_AUTOPROXY,
	MainDB			 = SQLITE_OPEN_MAIN_DB,
	TemporaryDB		 = SQLITE_OPEN_TEMP_DB,
	TransientDB		 = SQLITE_OPEN_TRANSIENT_DB,
	MainJournal		 = SQLITE_OPEN_MAIN_JOURNAL,
	TemporaryJournal = SQLITE_OPEN_TEMP_JOURNAL,
	SubJournal		 = SQLITE_OPEN_SUBJOURNAL,
	SuperJournal	 = SQLITE_OPEN_SUPER_JOURNAL,
	NoMutex			 = SQLITE_OPEN_NOMUTEX,
	FullMutex		 = SQLITE_OPEN_FULLMUTEX,
	SharedCache		 = SQLITE_OPEN_SHAREDCACHE,
	PrivateCache	 = SQLITE_OPEN_PRIVATECACHE,
	WAL				 = SQLITE_OPEN_WAL,
	NoFollow		 = SQLITE_OPEN_NOFOLLOW,
	ExResCode		 = SQLITE_OPEN_EXRESCODE,
};

constexpr inline auto to(bind_type_t type) { return (type == bind_type_t::Static) ? SQLITE_STATIC : SQLITE_TRANSIENT; }

template<typename T>
constexpr inline auto const to(std::set<T> const& flags, unsigned initial = 0u) {
	using std::cbegin, std::cend;	 // std::range::begin,end
	return std::accumulate(cbegin(flags), cend(flags), std::move(initial), [](auto&& s, auto const& a) { s |= static_cast<unsigned>(a); return s; });
}

constexpr inline auto const from_error_code(int code) {
	using enum error_code_t;
	switch (code) {
	case SQLITE_ABORT: return Aborted;
	case SQLITE_AUTH: return Authorization;
	case SQLITE_BUSY: return Busy;
	case SQLITE_CANTOPEN: return OpenFailed;
	case SQLITE_CONSTRAINT: return Constraint;
	case SQLITE_CORRUPT: return CorruptDB;
	case SQLITE_DONE: return Done;
	case SQLITE_FULL: return DiskFull;
	case SQLITE_INTERNAL: return InternalError;
	case SQLITE_INTERRUPT: return Interrupted;
	case SQLITE_IOERR: return IOError;
	case SQLITE_LOCKED: return Locked;
	case SQLITE_MISMATCH: return InvalidType;
	case SQLITE_MISUSE: return InvalidUsage;
	case SQLITE_NOLFS: return LargeFileUnsupported;
	case SQLITE_NOMEM: return NoMemory;
	case SQLITE_NOTADB: return InvalidDB;
	case SQLITE_NOTFOUND: return NotFound;
	case SQLITE_OK: return OK;
	case SQLITE_ROW: return Row;
	case SQLITE_PERM: return PermissionDenied;
	case SQLITE_ERROR: [[fallthrough]];
	default: return UnknownError;
	};
}

class exception_t : public std::runtime_error {
public:
	exception_t(::sqlite3* db, std::string_view message) :
		std::runtime_error(std::string{message} + " : " + ::sqlite3_errmsg(db)), code_{from_error_code(::sqlite3_errcode(db))} {}

	explicit exception_t(::sqlite3* db) :
		std::runtime_error(::sqlite3_errmsg(db)), code_{from_error_code(::sqlite3_errcode(db))} {}

	explicit exception_t(::sqlite3_stmt* statement) :
		exception_t(::sqlite3_db_handle(statement)) {}

	auto const code() const noexcept { return code_; }

private:
	error_code_t code_;
};

template<typename T, typename Enable = void>
struct binds_t {
	static int	bind(::sqlite3_stmt* statement, int index, T const& argument, bind_type_t type);
	static void column(::sqlite3_stmt* statement, int column, T& argument);
};

class statement_t {
public:
	statement_t(::sqlite3* db, std::string_view sql, std::set<prepare_flag_t> const& flags = {prepare_flag_t::Persistent}) {
		if (db == nullptr) { throw std::invalid_argument(__func__); }
		if (sql.empty()) { throw std::invalid_argument(__func__); }
		using std::cbegin, std::cend;
		if (auto const result = ::sqlite3_prepare_v3(db, sql.data(), static_cast<int>(sql.size()), to(flags), &statement_, nullptr); result != SQLITE_OK) { throw exception_t{db, sql}; }
	}
	statement_t(statement_t&& rhs) {
		using std::swap;	// std::ranges::swap
		swap(statement_, rhs.statement_);
		swap(fetch_, rhs.fetch_);
	}
	statement_t& operator=(statement_t&& rhs) {
		using std::swap;	// std::ranges::swap
		swap(statement_, rhs.statement_);
		swap(fetch_, rhs.fetch_);
		return *this;
	}
	statement_t(statement_t const&)			   = delete;
	statement_t& operator=(statement_t const&) = delete;

	~statement_t() {
		if (statement_) { ::sqlite3_finalize(statement_); }
	}

	template<typename... Arguments>
	void execute(Arguments const&... args) {
		reset();
		bind(bind_type_t::Transient, args...);
		step_row();
	}

	template<typename... Arguments>
	void execute(bind_type_t type, Arguments const&... arguments) {
		reset();
		bind(type, arguments...);
		step_row();
	}

	template<typename... Arguments>
	bool fetch(Arguments&... arguments) {
		if (! fetch_) { step_row(); }
		if (fetch_) {
			column(arguments...);
			fetch_ = false;	   // fetched.
			return true;
		}
		return false;
	}

private:
	void reset() {
		if (auto const res = ::sqlite3_reset(statement_); res != SQLITE_OK) { throw exception_t(statement_); }
	}
	void step_row() {
		if (auto const res = ::sqlite3_step(statement_); res != SQLITE_ROW && res != SQLITE_DONE) {
			throw exception_t(statement_);
		} else {
			fetch_ = (res == SQLITE_ROW);
		}
	}

	template<int Index = 1>
	void bind(bind_type_t) {}

	template<int Index = 1, typename T, typename... Arguments>
	void bind(bind_type_t type, T const& argument, Arguments const&... arguments) {
		if (auto const res = binds_t<T>::bind(statement_, Index, argument, type); res != SQLITE_OK) { throw exception_t(statement_); }
		bind<Index + 1>(type, arguments...);
	}

	template<int Column = 0>
	void column() {}

	template<int Column = 0, typename T, typename... Arguments>
	void column(T& argument, Arguments&... arguments) {
		binds_t<T>::column(statement_, Column, argument);
		column<Column + 1>(arguments...);
	}

private:
	::sqlite3_stmt* statement_ = nullptr;
	bool			fetch_	   = false;
};

class uri_t {
public:
	explicit uri_t(std::string_view uri) :
		uri_{uri} {}
	auto const& uri() const noexcept { return uri_; }

private:
	std::string uri_;
};

class db_t {
public:
	db_t(std::set<db_flag_t> const& flags = {db_flag_t::ReadWrite, db_flag_t::Create}) {
		if (auto const res = ::sqlite3_open_v2(":memory:", &db_, to(flags, SQLITE_OPEN_MEMORY), nullptr); res != SQLITE_OK) {
			::sqlite3_close_v2(db_);
			throw exception_t(db_);
		}
	}
	db_t(std::filesystem::path const& path, std::set<db_flag_t> const& flags = {db_flag_t::ReadWrite, db_flag_t::Create}) {
		if (path.empty()) { throw std::invalid_argument(__func__); }
		if (auto const res = ::sqlite3_open_v2(path.string().c_str(), &db_, to(flags), nullptr); res != SQLITE_OK) {
			::sqlite3_close_v2(db_);
			throw exception_t(db_);
		}
	}
	db_t(uri_t const& uri, std::set<db_flag_t> const& flags = {db_flag_t::ReadWrite, db_flag_t::Create}) {
		if (uri.uri().empty()) { throw std::invalid_argument(__func__); }
		if (auto const res = ::sqlite3_open_v2(uri.uri().c_str(), &db_, to(flags, SQLITE_OPEN_URI), nullptr); res != SQLITE_OK) {
			::sqlite3_close_v2(db_);
			throw exception_t(db_);
		}
	}
	db_t(db_t&& rhs) {
		using std::swap;
		swap(db_, rhs.db_);
	}
	db_t& operator=(db_t&& rhs) {
		using std::swap;
		swap(db_, rhs.db_);
		return *this;
	}
	db_t(db_t const&)			 = delete;
	db_t& operator=(db_t const&) = delete;

	~db_t() {
		if (db_) { ::sqlite3_close_v2(db_); }
	}

	::sqlite3* native_handle() { return db_; }

	void begin(transaction_type_t type = transaction_type_t::Deferred) {
		switch (type) {
		default: [[fallthrough]];
		case transaction_type_t::Deferred: execute("BEGIN DEFERRED TRANSACTION"); break;
		case transaction_type_t::Immediate: execute("BEGIN IMMEDIATE TRANSACTION"); break;
		case transaction_type_t::Exclusive: execute("BEGIN EXCLUSIVE TRANSACTION"); break;
		}
	}

	void commit() { execute("COMMIT TRANSACTION"); }
	void rollback() { execute("ROLLBACK TRANSACTION"); }

	[[nodiscard]] statement_t prepare(std::string_view sql, std::set<prepare_flag_t> const& flags = {prepare_flag_t::Persistent}) { return statement_t(db_, sql, flags); }	  // throws std::invalid_argument if the sql is empty.

	template<typename... Arguments>
	statement_t execute(std::string_view sql, Arguments const&... args) {
		statement_t s(db_, sql);	// throws std::invalid_argument if the sql is empty.
		s.execute(args...);
		return s;
	}

	template<typename T>
	std::optional<T> select_one(std::string_view sql) {
		auto s = execute(sql);
		s.execute();
		if (T v{}; s.fetch(v)) { return v; }
		return std::nullopt;
	}

private:
	::sqlite3* db_ = nullptr;
};

template<>
struct binds_t<bool> {
	static int	bind(::sqlite3_stmt* statement, int index, bool argument, bind_type_t) { return ::sqlite3_bind_int(statement, index, argument ? 1 : 0); }
	static void column(::sqlite3_stmt* statement, int column, bool& argument) { argument = ::sqlite3_column_int(statement, column) != 0; }
};

template<typename T>
struct binds_t<T, std::enable_if_t<(std::is_integral_v<T> || std::is_unsigned_v<T>) && sizeof(T) <= 4>> {
	static int	bind(::sqlite3_stmt* statement, int index, T argument, bind_type_t) { return ::sqlite3_bind_int(statement, index, static_cast<int>(argument)); }
	static void column(::sqlite3_stmt* statement, int column, T& argument) { argument = static_cast<T>(::sqlite3_column_int(statement, column)); }
};

template<typename T>
struct binds_t<T, std::enable_if_t<(std::is_integral_v<T> || std::is_unsigned_v<T>) && sizeof(T) == 8>> {
	static int	bind(::sqlite3_stmt* statement, int index, T argument, bind_type_t) { return ::sqlite3_bind_int64(statement, index, static_cast<int64_t>(argument)); }
	static void column(::sqlite3_stmt* statement, int column, T& argument) { argument = static_cast<T>(::sqlite3_column_int64(statement, column)); }
};

template<typename T>
struct binds_t<T, std::enable_if_t<std::is_enum_v<T>>> {
	static int	bind(::sqlite3_stmt* statement, int index, T argument, bind_type_t type) { return binds_t<std::underlying_type_t<T>>::bind(statement, index, static_cast<std::underlying_type_t<T>>(argument), type); }
	static void column(::sqlite3_stmt* statement, int column, T& argument) {
		std::underlying_type_t<T> a{};
		binds_t<std::underlying_type_t<T>>::column(statement, column, a);
		argument = static_cast<T>(a);
	}
};

template<>
struct binds_t<double> {
	static int	bind(::sqlite3_stmt* statement, int index, double argument, bind_type_t) { return ::sqlite3_bind_double(statement, index, argument); }
	static void column(::sqlite3_stmt* statement, int column, double& argument) { argument = ::sqlite3_column_double(statement, column); }
};

template<>
struct binds_t<char const*> {
	static int bind(::sqlite3_stmt* statement, int index, char const* argument, bind_type_t type) {
		return ::sqlite3_bind_text(statement, index, argument, -1, to(type));
	}
};

template<>
struct binds_t<std::string> {
	static int bind(::sqlite3_stmt* statement, int index, std::string const& argument, bind_type_t type) {
		return ::sqlite3_bind_text(statement, index, argument.data(), static_cast<int>(argument.size()), to(type));
	}
	static void column(::sqlite3_stmt* statement, int column, std::string& argument) {
		auto const data = ::sqlite3_column_text(statement, column);
		auto const size = ::sqlite3_column_bytes(statement, column);
		argument.assign(reinterpret_cast<char const*>(data), size);
	}
};

template<int Size>
struct binds_t<char[Size]> {
	static int	bind(::sqlite3_stmt* statement, int index, char const (&argument)[Size], bind_type_t type) { return ::sqlite3_bind_text(statement, index, argument, Size - 1, to(type)); }
	static void column(::sqlite3_stmt* statement, int column, char (&argument)[Size]) {
		if (auto const data = ::sqlite3_column_text(statement, column); data) { std::copy_n(argument, reinterpret_cast<char const*>(data), Size - 1); }
	}
};

template<typename T>
struct binds_t<std::optional<T>> {
	static int bind(::sqlite3_stmt* statement, int index, std::optional<T> const& argument, bind_type_t type) {
		if (argument) {
			return binds_t<T>::bind(statement, index, *argument, type);
		} else {
			return ::sqlite3_bind_null(statement, index);
		}
	}
	static void column(::sqlite3_stmt* statement, int column, std::optional<T>& argument) {
		if (::sqlite3_column_type(statement, column) != SQLITE_NULL) {
			binds_t<T>::column(statement, column, argument.emplace());
		} else {
			argument = std::nullopt;
		}
	}
};

template<>
struct binds_t<timestamp_t> {
	static int bind(::sqlite3_stmt* statement, int index, timestamp_t const& argument, bind_type_t) {
		auto const data = std::chrono::system_clock::to_time_t(argument);

		return ::sqlite3_bind_int64(statement, index, data);
	}
	static void column(::sqlite3_stmt* statement, int column, timestamp_t& argument) {
		auto const data = ::sqlite3_column_int64(statement, column);

		argument = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(data));
	}
};

template<>
struct binds_t<std::nullopt_t> {
	static int	bind(::sqlite3_stmt* statement, int index, std::nullopt_t const&, bind_type_t) { return ::sqlite3_bind_null(statement, index); }
	static void column(::sqlite3_stmt*, int, std::nullopt_t& argument) { argument = std::nullopt; }
};

class transaction_t {
public:
	void commit(bool committed = true) noexcept { committed_ = committed; }

	transaction_t(db_t& db, transaction_type_t type = transaction_type_t::Deferred) :
		db_{db}, committed_{} {
		db_.begin(type);
	}
	transaction_t(transaction_t&& rhs)		= delete;
	transaction_t(transaction_t const& rhs) = delete;
	~transaction_t() {
		if (committed_) {
			db_.commit();
		} else {
			db_.rollback();
		}
	}

private:
	db_t& db_;
	bool  committed_;
};

}	 // namespace xxx::db::sl3

#endif	  // xxx_DB_HXX_
