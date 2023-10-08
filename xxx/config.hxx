///	@file
///	@brief		xxx common library.
///	@details	Configurations.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_CONFIG_HXX_
#define xxx_CONFIG_HXX_

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace xxx::config {

/// @brief	Options.
using options_t = std::unordered_map<std::string, std::vector<std::string>>;

/// @brief 	Configurations.
///		This implementation allows only one value for a key.
///		This class is immutable.
class configurations_t {
public:
	/// @brief	Whether the key exists or not.
	/// @param[in]	key				A key to check .
	/// @return	It returns true if the key exists; otherwise it returns false.
	bool contains(std::string const& key) const noexcept;

	std::unordered_set<std::string> keys() const;

	/// @brief 	Gets a value.
	/// @param[in]	key				A key to get.
	/// @return		The value that related with the @p key.
	///	@throw		It throws an exception if the @o key does not exist.
	auto const& get(std::string const& key) const { return config_.at(key); }

	/// @brief 	Gets a value as specific type.
	///	@tparam		T				Type of result.
	///	@param[in]	key				A key to get
	///	@param[in]	index			Index of values
	/// @return		The value that related with the @p key.
	///	@throw		It throws an exception if the @o key does not exist.
	///	@throw		It throws an exception if the value cannot be converted to the @p T type.
	template<typename T>
	T get_as(std::string const& key, std::size_t index = 0u) const {
		if (auto const itr = config_.find(key); itr != config_.cend()) {
			T				   t;
			std::istringstream iss{itr->second.at(index)};
			iss >> t;
			if (! itr->second.at(index).empty() && iss.eof()) {
				return t;
			} else {
				throw std::bad_cast();
			}
		} else {
			throw std::out_of_range(key);
		}
	}
	/// @brief 	Gets a value as specific type.
	///	@tparam		T				Type of result.
	///	@param[in]	key				A key to get
	///	@param[in]	alternative		Default value that is used if the @p key does not exist
	///	@param[in]	index			Index of values
	/// @return		The value that related with the @p key.
	///				This method returns the @p alternative value if the @p key does not exist or its value cannot be converted to the @p T type.
	template<typename T>
	T get_as(std::string const& key, T const& alternative, std::size_t index = 0u) const {
		if (auto const itr = config_.find(key); itr != config_.cend() && index < itr->second.size() && ! itr->second.at(index).empty()) {
			T				   t = alternative;
			std::istringstream iss{itr->second.at(index)};
			iss >> t;
			return iss.eof() ? t : alternative;
		} else {
			return alternative;
		}
	}
	/// @brief 	Gets a value as specific type.
	///	@tparam		T				Type of result.
	///	@param[in]	key				A key to get
	///	@param[in]	alternative		Default value that is used if the @p key does not exist
	///	@param[in]	index			Index of values
	/// @return		The value that related with the @p key.
	///				It returns the @p alternative value if the @p key does not exist.
	template<typename T>
	T get_as(std::string const& key, T&& alternative, std::size_t index = 0u) const {
		if (auto const itr = config_.find(key); itr != config_.cend() && index < itr->second.size() && ! itr->second.at(index).empty()) {
			T				   t = alternative;
			std::istringstream iss{itr->second.at(index)};
			iss >> t;
			return iss.eof() ? t : alternative;
		} else {
			return std::move(alternative);
		}
	}

	/// @brief	Constructor that loads from the @p path.
	///	Configuration file format is the following:
	///	-	A line that starts with the '#' is comment and ignored.
	///	-	Empty lines are ignored, too.
	///	-	Valid line is formatted as the following:
	///			Key = Value
	/// 	- Key: One or more following characters: alphabets and digits, hyphen, underscore, and dot.
	///			Whitespaces before and after the key are ignored.
	///		- Value: Zero or more any characters that includes the '#' and whitespaces,
	///		 	excepted whitespaces following the = separator.
	///			Hence, if there are only whitespaces on right side of the = separator,
	///			it means a value is empty.
	/// @param[in]	path		The path of a configuration file.
	///	@throw	It throws an exception if failed to load or syntax error exists in the file.
	explicit configurations_t(std::filesystem::path const& path);
	/// @brief	Constructor that gets the options.
	/// @param[in]	options		Pairs of a key and its value.
	explicit configurations_t(options_t const& options);
	/// @brief	Constructor that gets the options.
	/// @param[in]	options		Pairs of a key and its value.
	explicit configurations_t(options_t&& options);
	/// @copydoc	configurations_t(std::filesystem::path const&)
	///			If there are same keys in both the @p options and the file of the @p path, it uses a value of the key in the @p options.
	/// @param[in]	path		The path of a configuration file.
	/// @param[in]	options		Pairs of a key and its value.
	explicit configurations_t(std::filesystem::path const& path, options_t const& options);
	/// @copydoc	configurations_t(std::filesystem::path const&)
	///			If there are same keys in both the @p options and the file of the @p path, it uses a value of the key in the @p options.
	/// @param[in]	path		The path of a configuration file.
	/// @param[in]	options		Pairs of a key and its value.
	explicit configurations_t(std::filesystem::path const& path, options_t&& options);
	/// @brief	Constructor.
	configurations_t() noexcept :
		config_{} {}

private:
	///	@brief	Loads configurations from the @p path.
	/// @param[in]	config		Configurations.
	/// @param[in]	path		The path of a configuration file.
	static void load_file(options_t& config, std::filesystem::path const& path);
	///	@brief	Validates all the keys in the @p options.
	/// @param[in]	options		Options to validate.
	static void validate_keys(options_t const& options);

private:
	options_t config_;
};

template<>
inline std::string configurations_t::get_as(std::string const& key, std::size_t index) const {
	return config_.at(key).at(index);
}
template<>
inline std::string configurations_t::get_as(std::string const& key, std::string const& alternative, std::size_t index) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return itr->second.at(index);
	} else {
		return alternative;
	}
}
template<>
inline std::string configurations_t::get_as(std::string const& key, std::string&& alternative, std::size_t index) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return itr->second.at(index);
	} else {
		return std::move(alternative);
	}
}

template<>
inline bool configurations_t::get_as(std::string const& key, std::size_t index) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return ! itr->second.at(index).empty() && itr->second.at(index) != "0" && itr->second.at(index) != "false";
	} else {
		throw std::out_of_range(key);
	}
}
template<>
inline bool configurations_t::get_as(std::string const& key, bool const& alternative, std::size_t index) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return ! itr->second.at(index).empty() && itr->second.at(index) != "0" && itr->second.at(index) != "false";
	} else {
		return alternative;
	}
}
template<>
inline bool configurations_t::get_as(std::string const& key, bool&& alternative, std::size_t index) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return ! itr->second.at(index).empty() && itr->second.at(index) != "0" && itr->second.at(index) != "false";
	} else {
		return std::move(alternative);
	}
}

///	@brief	Gets options of application.
///	@param[in]	ac		Argument count that also contains name of the application.
///	@param[in]	av		Argument values. av[0] is the name.
///	@return		The first integer is result of parsing.
///				If the "-h" or "--help" options exists, it is a positive value;
///				if invalid option exists, it is negative value; otherwise, it is zero.
///				The second value is options and other arguments.
///				The 'option' is an argument starts with the '-' character excluding single '-' character only.
///				Index of the option is its name excluding the '-' characters.
///				Value of the option is any characters following the '=' or ':' in the argument.
///				Other arguments excluding the options have empty("") index of the second container.
///				Note that the second value is empty if the first integer is negative value.
std::tuple<int, options_t> get_options(int ac, char* av[]);

}	 // namespace xxx::config

#endif	  // xxx_CONFIG_HXX_
