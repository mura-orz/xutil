///	@file
///	@brief		xxx common library.
///	@details	Utilities.
///	@pre		ISO/IEC 14882:2017
///	@author		Mura
///	@copyright	(C) 2011-2014, 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_FINALLY_HXX_
#define xxx_FINALLY_HXX_

namespace xxx {

///	@brief	The @b finally handler called when scoped-out.
///	@tparam		P	function object of procedure
template<typename P>
class finally_t {
public:
	///	@brief	Sets the @p finally handler.
	///	@param[in]		procedure	Procedure
	explicit finally_t(P const& procedure) :
		procedure_(procedure) {}
	///	@brief	Sets the @p finally handler.
	///	@param[in]		procedure	Procedure
	explicit finally_t(P const&& procedure) :
		procedure_(procedure) {}
	///	@brief	Destructor.
	~finally_t() {
		if constexpr (noexcept(procedure_())) {
			procedure_();
		} else {
			try {
				procedure_();
			} catch (...) {}
		}
	}

private:
	finally_t(finally_t const&)					 = delete;
	finally_t const& operator=(finally_t const&) = delete;

private:
	P const& procedure_;
};

}	 // namespace xxx

#endif	  // xxx_FINALLY_HXX_
