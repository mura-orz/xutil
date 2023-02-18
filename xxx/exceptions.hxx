///	@file
///	@brief		xxx common library.
///	@details	Utilities for exceptions.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_EXCEPTIONS_HXX_
#define xxx_EXCEPTIONS_HXX_

#include <exception>

#include <iostream>
#include <string>
#include <typeinfo>

namespace xxx {

///	@name	exception handling
///	@{

///	@brief	Suppresses any exception.
///	@tparam		P	function object of procedure
///	@param[in]		procedure	Procedure
///	@return		If an exception occurred, it returns it as std::exception_ptr;
///				otherwise, it returns nullptr as std::exception_ptr.
template<typename P>
inline std::exception_ptr
suppress_exceptions(P const& procedure) noexcept
{
	if constexpr (noexcept(procedure())) {
		procedure();
		return nullptr;
	} else {
		try {
			procedure();
			return nullptr;
		} catch(...) {
			return std::current_exception();
		}
	}
}

///	@brief	Ignores any exception.
///	@tparam		P	function object of procedure
///	@param[in]		procedure	Procedure
template<typename P>
inline void
ignore_exceptions(P const& procedure) noexcept
{
	if constexpr (noexcept(procedure())) {
		procedure();
	} else {
		try {
			procedure();
		} catch(...) {}
	}
}

///	@copydoc	ignore_exceptions()
///	@tparam		H	function object of exception handler
///	@param[in]		handler		Exception handler,
///								which requires only one const reference of std::exception parameter.
template<typename P, typename H>
inline void
ignore_exceptions(P const& procedure, H const& handler) noexcept
{
	//	This method is just for typical wrapper.
	//	The suppress_exceptions and ignore_exceptions use constexpr inside.
	
	auto const	ep	= suppress_exceptions(procedure);
	if(ep) {
		try {
			std::rethrow_exception(ep);
		} catch(std::exception const& e) {
			ignore_exceptions([&handler, &e]() {
				handler(e);
			});
		} catch(...) {
			ignore_exceptions([&handler]() {
				//	If actual exception's information is required,
				//	use suppress_exceptions directly, instead.
				//	This method is just for typical wrapper.
				handler(std::exception());
			});
		}
	}
}

///	@}

///	@name	nested exception
///	@{

namespace impl {

inline static void
dump_exception_(std::ostream& os, std::exception const& e, unsigned nested)
{
	// The nested_exception type would not be simple.
	auto const	p		= dynamic_cast<std::nested_exception const*>(&e);
	auto const	name	= p == nullptr ? typeid(e).name() : "std::nested_exception";

	if(0u == nested) {
		os		<< e.what() << " (" << name << ")" << std::endl;
	}
	os		<< " [" << nested << "] " << e.what() << " (" << name << ")" << std::endl;

	try {
		std::rethrow_if_nested(e);
	} catch(std::exception const& ee) {
		dump_exception_(os, ee, nested + 1u);
	}
}

}	// namespace impl

///	@brief	Dumps exceptions.
///	@param[in,out]	os		Output stream
///	@param[in]		e		Exception to dump
inline void
dump_exception(std::ostream& os, std::exception const& e) noexcept
{
	impl::dump_exception_(os, e, 0u);
}

///	@brief	Checks whether argument is valid or not.
///	@param[in]	valid		The result of argument validation.
///	@param[in]	message		Message set to exception if thrown.
inline void
validate_argument(bool valid, char const* message=nullptr)
{
	if( ! valid) {
		throw std::invalid_argument(message == nullptr ? "" : message);
	}
}

///	@name	dump exception to iostream.
namespace exception_iostream {

///	@brief	Dumps exceptions.
///	@param[in,out]	os		Output stream
///	@param[in]		e		Exception to dump
///	@return		It returns the @p os output stream.
///	@pre	Using the exception_iostream namespace is required.
///			e.g., @code
///				using namespace xxx::exception_iostream;
///			@endcode
inline std::ostream&
operator <<(std::ostream& os, std::exception const& e) noexcept
{
	dump_exception(os, e);
	return os;
}

}	// namespace exception_iostream
	
///	@}

}	// namespace xxx

#endif	// xxx_EXCEPTIONS_HXX_
