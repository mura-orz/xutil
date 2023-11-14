///	@file
///	@brief		Signal handler.
///	@author		Mura.
///	@copyright	(C) 2018-, Mura.

#ifndef xxx_SIG_HPP_
#define xxx_SIG_HPP_

#include <chrono>

namespace xxx::sig {

/// @brief	Disable several signal handlers in this program.
///		It disables the following signals: Interrupt, Terminate, Hangup.
void disable_signal_handlers();

///	@brief 	Wait for one of the following signals: Interrupt, Terminate, Hangup.
///	@param[in]	timeout		The timeout.
///	@return		It returns @true if the signal occurred; otherwise, it returns false.
bool wait_for_signals(std::chrono::milliseconds const& timeout);

}	 // namespace xxx::sig

#endif	  // xxx_SIG_HPP_
