///	@file
///	@brief		xxx common library.
///	@details	Redux-like framework.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2022-, Mura. All rights reserved. (MIT License)

#ifndef xxx_REDUX_HXX_
#define xxx_REDUX_HXX_

#include <shared_mutex>
#include <algorithm>
#include <any>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace xxx::redux {

///	@brief	Action
///	@tparam	Id	Identifier of the action type.
template<typename Id>
class action {
public:
	///	@brief	Gets the identifier of the action type.
	///	@return	The identifier.
	auto id() const noexcept { return id_; }
	///	@brief	Gets the options.
	///	@tparam	T	Type of the @p option.
	///	@return	Constant reference to the @p option.
	template<typename T>
	T const& option() const { return std::any_cast<T const&>(option_); }

	///	@brief	Constructor.
	action() :
		id_{}, option_{} {}
	///	@brief	Constructor.
	///	@param[in]	id		Identifier of the action type.
	explicit action(Id id) :
		id_{id}, option_{} {}
	///	@brief	Constructor.
	///	@param[in]	id		Identifier of the action type.
	///	@param[in]	optino		Option of the @p action.
	template<typename P>
	action(Id id, P const& option) :
		id_{id}, option_{option} {}
	///	@brief	Constructor.
	///	@param[in]	id		Identifier of the action type.
	///	@param[in]	optino		Option of the @p action.
	template<typename P>
	action(Id id, P&& option) :
		id_{id}, option_{std::move(option)} {}

private:
	Id		 id_;		 ///< Identifier of the action type.
	std::any option_;	 ///< Option of the action
};

///	@brief	Store.
///	@tparam	State	Type of state.
///	@tparam	Action	Type of action.
template<typename State, typename Action>
class store {
public:
	using reducer_t	 = std::function<State(State const&, Action const&)>;	 ///< Type of reducer.
	using listener_t = std::function<void(State const&, Action const&)>;	 ///< Type of listener.

	///	@brief	Refers the current state with readers lock.
	///		It does not return the state simply to enforce readers lock.
	///		But it does not copy the state.
	///	@tparam	F	Type of @p functor.
	///	@param[in]	functor		Functor to refer the current state.
	template<typename F>
	void state(F const& functor) const {
		std::shared_lock lock{mutex_};
		functor(state_);
	}
	///	@brief	Gets a full copy of the current state.
	///		It is a simple interface but needs deep copy of the state.
	///	@return	The full copy of the current state.
	State fullcopied_state() const {
		std::shared_lock lock{mutex_};
		return state_;
	}

	///	@brief	Dispacthes the @p action.
	///	 	It assumes that dispatch is always called sequentially; otherwise,
	///	@param[in]	action		Action to dispatch.
	void dispatch(Action const& action) {
		{
			std::lock_guard lock{mutex_};
			auto			state = reducer_(state_, action);

			using std::swap;
			swap(state_, state);
		}
		// It assumes that dispatch is always called sequentially; otherwise,
		// if another dispatch is interrupted here, each listerner of the dispatches refers same the latest state.
		{
			std::shared_lock lock{mutex_};
			std::ranges::for_each(listeners_, [&](auto const& listener) { listener(state_, action); });
		}
	}

	///	@brief	Adds all the listeiners if exist.
	///	@param[in]	listener	Listener to set.
	///	@return		Handling identifier of the @p listener.
	std::size_t add_listener(listener_t listener) {
		std::lock_guard lock{mutex_};
		listeners_.push_back(listener);
		return listeners_.size() - 1;
	}
	///	@brief	Removes the specific listener.
	///	@param[in]	listener	Handling identifier of the @p listener.
	void remove_listener(std::size_t listener) {
		std::lock_guard lock{mutex_};
		// It uses soft-delete to keep index.
		// It just replaces the listener by empty placeholder.
		// The clear_listeners uses hard-delete but always clears all the listeners.
		listeners_.at(listener) = [](State const&, Action const&) {};
	}
	///	@brief	Clears all the listeiners if exist.
	void clear_listeners() {
		std::lock_guard lock{mutex_};
		listeners_.clear();
	}

	///	@brief	Constructor.
	///	@param[in]	state		Iniitial state.
	///	@param[in]	reducer		Reducer of the @p state.
	store(State const& state, reducer_t reducer) :
		mutex_{}, state_{state}, reducer_{reducer}, listeners_{} {
		if (! state_) throw std::invalid_argument(__func__);
	}
	///	@brief	Constructor.
	///	@param[in]	state		Iniitial state.
	///	@param[in]	reducer		Reducer of the @p state.
	store(State&& state, reducer_t reducer) :
		mutex_{}, state_{std::move(state)}, reducer_{reducer}, listeners_{} {
		if (! state_) throw std::invalid_argument(__func__);
	}
	virtual ~store() {}

private:
	mutable std::shared_mutex mutex_;		 ///< Readers-writer mutex.
	State					  state_;		 ///< Statw.
	reducer_t				  reducer_;		 ///< Reducer.
	std::vector<listener_t>	  listeners_;	 ///< Listerners.
};

}	 // namespace xxx::redux

#endif	  // xxx_REDUX_HXX_
