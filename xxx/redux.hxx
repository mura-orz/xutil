///	@file
///	@brief		xxx common library.
///	@details	Redux-like framework.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2022-, Mura. All rights reserved. (MIT License)

#ifndef xxx_REDUX_HXX_
#define xxx_REDUX_HXX_

#include <ranges>
#include <algorithm>
#include <functional>
#include <shared_mutex>
#include <vector>
#include <any>
#include <stdexcept>
#include <memory>

namespace xxx::redux {

template<typename Id>
class action {
public:
	auto		id() const noexcept			{ return id_;	}
	template<typename T>	T const&	option() const { return std::any_cast<T const&>(option_);	}

	action() : id_{}, option_{} {}
	explicit	action(Id id) : id_{ id }, option_{} {}
	template<typename P>	action(Id id, P const& option) : id_{ id }, option_{ option } {}
	template<typename P>	action(Id id, P&& option) : id_{ id }, option_{ std::move(option) } {}
private:
	Id			id_;
	std::any	option_;
};

template<typename State, typename Action>
class store {
public:
	using	reducer_t	= std::function<State(State const&, Action const&)>;
	using	listener_t	= std::function<void(State const&, Action const&)>;

	void			state(std::function<void(State const&)> functor) const {
		std::shared_lock		lock{ mutex_ };
		functor(state_);
	}

	void			dispatch(Action const& action) {
		{
			std::lock_guard		lock{ mutex_ };
			state_	= reducer_(state_, action);
		}
		{
			std::shared_lock	lock{ mutex_ };
			std::ranges::for_each(listeners_, [&](auto const& listener){ listener(state_,action); });
		}
	}

	void	add_listener(listener_t listener) { listeners_.push_back(listener); }
	void	clear_listeners() { listeners_.clear(); }

	store(State const& state, reducer_t reducer) : mutex_{}, state_{ state }, reducer_{ reducer }, listeners_{}			{ if (!state_) throw std::invalid_argument(__func__); }
	store(State&& state, reducer_t reducer) : mutex_{}, state_{ std::move(state) }, reducer_{ reducer }, listeners_{}	{ if (!state_) throw std::invalid_argument(__func__); }
	virtual ~store() {}
private:
	mutable std::shared_mutex		mutex_;
	State							state_;
	reducer_t						reducer_;
	std::vector<listener_t>			listeners_;
};

}	// namespace xxx::redux

#endif	// xxx_REDUX_HXX_
