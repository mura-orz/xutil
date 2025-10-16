
///	@file
///	@brief		xxx common library.
///	@details	Redux-like framework.
///	@pre		ISO/IEC 14882:2017 or higher
///	@author		Mura
///	@copyright	(C) 2018-, Mura. All rights reserved. (MIT License)

#ifndef xxx_QUEUE_HXX_
#define xxx_QUEUE_HXX_

#include <condition_variable>
#include <deque>
#include <mutex>
#include <stdexcept>

namespace xxx {

///	@brief	Event queue.
template<typename T>
class queue {
public:
	///	@brief	Terminated
	class terminated_error : public std::runtime_error {
	public:
		///	@brief	Constructor.
		terminated_error() :
			std::runtime_error("terminated") {}
	};

	/// @brief 	Enqueues an event.
	/// @param[in]	t	The event to push.
	void enqueue(T const& t) {
		std::lock_guard lock{mutex_};
		if (finished_) throw terminated_error{};
		queue_.push_back(t);
		condition_.notify_one();
	}
	/// @brief 	Enqueues an event.
	/// @param[in]	t	The event to push.
	void enqueue(T&& t) {
		std::lock_guard lock{mutex_};
		if (finished_) throw terminated_error{};
		queue_.emplace_back(std::move(t));
		condition_.notify_one();
	}
	/// @brief 	Dequeues an event.
	///		If this queue is empty, this method waits a new event.
	/// @param[in]	t	Next event.
	void dequeue(T& t) {
		std::unique_lock lock{mutex_};
		while (queue_.empty()) {
			if (finished_) throw terminated_error{};
			condition_.wait(lock);
		}
		if (finished_) throw terminated_error{};
		t = queue_.front();	   // might cause an exception.
		queue_.pop_front();
	}
	/// @brief 	Dequeues an event.
	///		If this queue is empty, this method waits a new event.
	/// @param[in]	t	Next event.
	void terminate() noexcept {
		std::lock_guard lock{mutex_};
		finished_ = true;
		queue_.clear();
		condition_.notify_all();
	}
	/// @brief 	Is this queue empty?
	/// @return		It returns true if the queue is empty; otherwise, it returns false.
	bool empty() const noexcept {
		std::lock_guard lock{mutex_};
		return queue_.empty();
	}
	/// @brief 	Is this queue terminated?
	/// @return		It returns true if the queue has been terminated; otherwise, it returns false.
	bool terminated() const noexcept { return finished_; }

	/// @brief 	Constructor.
	queue() noexcept :
		mutex_{}, queue_{}, condition_{}, finished_{} {}
	/// @brief 	Destructor.
	~queue() noexcept { terminate(); }

private:
	mutable std::mutex		mutex_;		   ///< Mutex.
	std::deque<T>			queue_;		   ///< Queue.
	std::condition_variable condition_;	   ///< Condition to wait.
	bool					finished_;	   ///< Finished flag.
};

}	 // namespace xxx

#endif	  // xxx_QUEUE_HXX_
