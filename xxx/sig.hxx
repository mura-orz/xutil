///	@file
///	@brief		Signal handler.
///	@author		Mura.
///	@copyright	(C) 2018-, Mura.

#ifndef xxx_SIG_HPP_
#define xxx_SIG_HPP_

#include <atomic>
#include <csignal>

namespace xxx {

///	@brief	Signal handler.
class signal_handler_t {
private:
	static std::sig_atomic_t volatile		interrupted_s;

	///	@brief	Signal handler.
	///	@param[in]	signal		Occurred signal.
	static void		handle_signal(int signal)noexcept;
public:
	///	@brief	Is signal interrupted.
	///	@return	It returns true if interrupted; otherwise, it returns false.
	bool			interrupted()const volatile noexcept	{	return interrupted_s != false;	}
	/// @brief  Clears interruption.
	void			clear() noexcept { interrupted_s = false; }
	///	@brief	Constructor.
	///		Sets SIGINT and SIGTERM handlers to interrupt it.
	signal_handler_t()noexcept;
	///	@brief	Destructor.
	///		Resets SIGINT and SIGTERM handlers.
	~signal_handler_t()noexcept;
};

}	// namespace xxx

#endif	//xxx_SIG_HPP_
