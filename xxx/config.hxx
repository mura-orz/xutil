///	@file
///	@brief		xxx common library.
///	@details	Configurations.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_CONFIG_HXX_
#define xxx_CONFIG_HXX_

#include <filesystem>
#include <unordered_map>
#include <string>
#include <sstream>
#include <stdexcept>

namespace xxx::config {

/// @brief	Options.
using options_t	= std::unordered_map<std::string, std::string>;

/// @brief 	Configurations.
///		This implementation allows only one value for a key.
///		This class is immutable.
class configurations_t {
public:
	/// @brief	Whether the key exists or not.
	/// @param[in]	key				A key to check .
	/// @return	It returns true if the key exists; otherwise it returns false.
	bool	contains(std::string const& key) const noexcept;

	/// @brief 	Gets a value.
	/// @param[in]	key				A key to get.
	/// @return		The value that related with the @p key.
	///	@throw		It throws an exception if the @o key does not exist.
	std::string const&		get(std::string const& key) const { return config_.at(key);		}

	/// @brief 	Gets a value as specific type.
	///	@tparam		T				Type of result.
	///	@param[in]	key				A key to get
	/// @return		The value that related with the @p key.
	///	@throw		It throws an exception if the @o key does not exist.
	///	@throw		It throws an exception if the value cannot be converted to the @p T type.
	template<typename T>	T		get_as(std::string const& key) const	{
		if (auto const itr = config_.find(key); itr != config_.cend()) {
			T	t;
			std::istringstream	iss{itr->second};	iss	>> t;
			if (!itr->second.empty() && iss.eof()) {
				return t;
			} else { throw std::bad_cast();	}
		} else { throw std::out_of_range(key); }
	}
	/// @brief 	Gets a value as specific type.
	///	@tparam		T				Type of result.
	///	@param[in]	key				A key to get
	///	@param[in]	alternative		A key to get
	/// @return		The value that related with the @p key.
	///				This method returns the @p alternative value if the @p key does not exist or its value cannnot be converted to the @p T type.
	template<typename T>	T		get_as(std::string const& key, T const& alternative) const {
		if (auto const itr = config_.find(key); itr != config_.cend()) {
			T	t	= alternative;
			std::istringstream	iss{itr->second};	iss	>> t;
			return !itr->second.empty() && iss.eof() ? t : alternative;
		} else { return alternative; }
	}
	/// @brief 	Gets a value as specific type.
	///	@tparam		T				Type of result.
	///	@param[in]	key				A key to get
	///	@param[in]	alternative		A key to get
	/// @return		The value that related with the @p key.
	///				It returns the @p alternative value if the @p key does not exist.
	template<typename T>	T		get_as(std::string const& key, T&& alternative) const {
		if (auto const itr = config_.find(key); itr != config_.cend()) {
			T	t	= alternative;
			std::istringstream	iss{itr->second};	iss	>> t;
			return !itr->second.empty() && iss.eof() ? t : alternative;
		} else { return std::move(alternative); }
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
	///			Hense, if there are only whitespaces on right side of the = separator,
	///			it means a value is empty.
	/// @param[in]	path		The path of a configuration file.
	///	@throw	It throws an exception if failed to load or syntax error exists in the file.
	explicit	configurations_t(std::filesystem::path const& path);
	/// @brief	Constructor that gets the options.
	/// @param[in]	options		Pairs of a key and its value.
	explicit	configurations_t(options_t const& options);
	/// @brief	Constructor that gets the options.
	/// @param[in]	options		Pairs of a key and its value.
	explicit	configurations_t(options_t&& options);
	/// @copydoc	configurations_t(std::filesystem::path const&)
	///			If there are same keys in both the @p options and the file of the @p path, it uses a value of the key in the @p options.
	/// @param[in]	path		The path of a configuration file.
	/// @param[in]	options		Pairs of a key and its value.
	explicit	configurations_t(std::filesystem::path const& path, options_t const& options);
	/// @copydoc	configurations_t(std::filesystem::path const&)
	///			If there are same keys in both the @p options and the file of the @p path, it uses a value of the key in the @p options.
	/// @param[in]	path		The path of a configuration file.
	/// @param[in]	options		Pairs of a key and its value.
	explicit	configurations_t(std::filesystem::path const& path, options_t&& options);
	/// @brief	Constructor.
	configurations_t() noexcept : config_{} {}
private:
	///	@brief	Loads configurations from the @p path.
	/// @param[in]	config		Configurations.
	/// @param[in]	path		The path of a configuration file.
	static void		load_file(options_t& config, std::filesystem::path const& path);
	///	@brief	Validates all the keys in the @p options.
	/// @param[in]	options		Options to validate.
	static void		validate_keys(options_t const& options);
private:
	options_t	config_;
};

template<>	inline std::string		configurations_t::get_as(std::string const& key) const {
	return config_.at(key);
}
template<>	inline std::string		configurations_t::get_as(std::string const& key, std::string const& alternative) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return itr->second;
	} else { return alternative; }
}
template<>	inline std::string		configurations_t::get_as(std::string const& key, std::string&& alternative) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return itr->second;
	} else { return std::move(alternative); }
}

template<>	inline bool				configurations_t::get_as(std::string const& key) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return !itr->second.empty() && itr->second != "0" && itr->second != "false" && itr->second != "nullptr";
	} else { throw std::out_of_range(key); }
}
template<>	inline bool				configurations_t::get_as(std::string const& key, bool const& alternative) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return !itr->second.empty() && itr->second != "0" && itr->second != "false" && itr->second != "nullptr";
	} else { return alternative; }
}
template<>	inline bool				configurations_t::get_as(std::string const& key, bool&& alternative) const {
	if (auto const itr = config_.find(key); itr != config_.cend()) {
		return !itr->second.empty() && itr->second != "0" && itr->second != "false" && itr->second != "nullptr";
	} else { return std::move(alternative); }
}


}	// namespace xxx::config

#endif	// xxx_CONFIG_HXX_
