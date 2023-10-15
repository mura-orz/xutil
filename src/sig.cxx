///	@file
///	@brief		Signal handler.
///	@author		Mura.
///	@copyright	(C) 2018-, Mura.

#include <xxx/sig.hxx>

#include <system_error>
#include <csignal>
#include <stdexcept>

#ifdef _WIN32
#define SIGHUP (0)	  // for dummy
#endif

///	@brief	The interrupted flag.
static std::sig_atomic_t volatile interrupted_s;	// std::atomic<bool>

extern "C" void handle_signal(int signal) noexcept {
	switch (signal) {
	case SIGINT:
	case SIGHUP:
	case SIGTERM:
		interrupted_s = true;
		break;
	default:
		break;
	}
	std::signal(SIGINT, handle_signal);
	std::signal(SIGTERM, handle_signal);
#ifndef _WIN32
	std::signal(SIGHUP, handle_signal);
#endif
}

namespace xxx::sig {

///	@brief 	Disable the following signals: Interrupt, Terminate, Hangup.
void disable_signal_handlers_in_current_thread() {
#ifndef _WIN32
	::sigset_t ss{};
	::sigemptyset(&ss);
	::sigaddset(&ss, SIGINT);
	::sigaddset(&ss, SIGTERM);
	::sigaddset(&ss, SIGHUP);
	if (::pthread_sigmask(SIG_BLOCK, &ss, nullptr) != 0) {
		std::error_code const ec{errno, std::system_category()};
		throw std::system_error{ec, __func__};
	}
#else
	std::signal(SIGINT, ::interrupting_signal_handler);
	std::signal(SIGTERM, ::interrupting_signal_handler);
#endif
}

///	@brief 	Wait for one of the following signals: Interrupt, Terminate, Hangup.
///	@param[in]	timeout		The timeout.
///	@return		It returns @true if the signal occurred; otherwise, it returns false.
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
	do {
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
			case EINVAL: throw std::invalid_argument(__func__);
			default:
				std::error_code const ec{errno, std::system_category()};
				throw std::system_error{ec, __func__};
			}
		default: throw std::runtime_error(__func__);
		}
	} while (false);
	throw std::logic_error(__func__);
#else
	// Does not take care of exclusive control here.
	if (::xxx::sig::interrupted_s) return true;
	std::this_thread::sleep_for(timeout);
	auto const interrupted	  = ::xxx::sig::interrupted_s;
	::xxx::sig::interrupted_s = false;
	return interrupted != false;
#endif
}

}	 // namespace xxx::sig
