///	@file
///	@brief		Signal handler.
///	@author		Mura.
///	@copyright	(C) 2018-, Mura.

#include <xxx/sig.hxx>

#include <thread>
#include <system_error>
#include <csignal>
#include <stdexcept>

///	@brief	The interrupted flag.
static std::sig_atomic_t volatile interrupted_s;	// std::atomic<bool>

extern "C" void handle_signal(int signal) noexcept {
	switch (signal) {
	case SIGINT:
#ifndef _WIN32
	case SIGHUP:
#endif
	case SIGTERM:
		interrupted_s = true;
		break;
	default:
		break;
	}
	std::signal(SIGINT, ::handle_signal);
	std::signal(SIGTERM, ::handle_signal);
#ifndef _WIN32
	std::signal(SIGHUP, ::handle_signal);
#endif
}

namespace xxx::sig {

void disable_signal_handlers() noexcept {
	// It uses original handler instead of SIGIGN
	// otherwise wait_for_signals does not work.
	std::signal(SIGINT, ::handle_signal);
	std::signal(SIGTERM, ::handle_signal);
#ifndef _WIN32
	std::signal(SIGHUP, ::handle_signal);
#endif
}

bool wait_for_signals(std::chrono::milliseconds const& timeout) {
#ifndef _WIN32
	using namespace std::chrono_literals;
	::sigset_t ss{};
	::sigemptyset(&ss);
	::sigaddset(&ss, SIGINT);
	::sigaddset(&ss, SIGTERM);
	::sigaddset(&ss, SIGHUP);
	::timespec ts{};
	ts.tv_sec  = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
	ts.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count() % std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count();
	for (;;) {
		::siginfo_t signal{};
		switch (::sigtimedwait(&ss, &signal, &ts)) {
		// interrupted by a signal.
		case SIGINT: [[fallthrough]];
		case SIGHUP: [[fallthrough]];
		case SIGTERM: return true;
		// something wrong happened.
		case -1:
			switch (errno) {
			case EAGAIN: return false;
			case EINTR: continue;
			case EINVAL: throw std::invalid_argument(__func__+ std::to_string(__LINE__));
			default:
				std::error_code const ec{errno, std::system_category()};
				throw std::system_error{ec, __func__+ std::to_string(__LINE__)};
			}
		default: throw std::runtime_error(__func__);
		}
	}
#else
	// Does not take care of exclusive control here.
	if (! interrupted_s) {
		std::this_thread::sleep_for(timeout);
	}
	auto const interrupted	  = interrupted_s;
	interrupted_s = false;
	return interrupted != false;
#endif
}

}	 // namespace xxx::sig
