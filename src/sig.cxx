///	@file
///	@brief		Signal handler.
///	@author		Mura.
///	@copyright	(C) 2018-, Mura.

#include <xxx/sig.hxx>

namespace xxx {

std::sig_atomic_t volatile		signal_handler_t::interrupted_s;

void
signal_handler_t::handle_signal(int signal)noexcept {
	switch(signal) {
	case SIGINT:
	case SIGTERM:
		interrupted_s	= true;
		break;
	default:
		break;
	}
	std::signal(SIGINT,  handle_signal);
	std::signal(SIGTERM, handle_signal);
}

signal_handler_t::signal_handler_t()noexcept {
	std::signal(SIGINT,  handle_signal);
	std::signal(SIGTERM, handle_signal);
}

signal_handler_t::~signal_handler_t()noexcept {
	std::signal(SIGINT,  SIG_DFL);
	std::signal(SIGTERM, SIG_DFL);
}

}	// namespace xxx

