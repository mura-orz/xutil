
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
	/// @brief 	Enqueues an event.
	/// @param[in]	t	The event to push.
	/// @return	It returns true if queued; otherwise, it returns false.
	bool enqueue(T const& t) {
		std::lock_guard lock{mutex_};
		if (finished_) return false;
		queue_.push_back(t);
		condition_.notify_one();
		return true;
	}
	/// @brief 	Enqueues an event.
	/// @param[in]	t	The event to push.
	/// @return	It returns true if queued; otherwise, it returns false.
	bool enqueue(T&& t) {
		std::lock_guard lock{mutex_};
		if (finished_) return false;
		queue_.emplace_back(std::move(t));
		condition_.notify_one();
		return true;
	}
	/// @brief 	Dequeues an event.
	///		If this queue is empty, this method waits a new event.
	/// @param[in]	t	Next event.
	/// @return	It returns true if dequeued; otherwise, it returns false.
	bool dequeue(T& t) {
		std::unique_lock lock{mutex_};
		while (queue_.empty()) {
			if (finished_) return false;
			condition_.wait(lock);
		}
		if (finished_) return false;
		t = queue_.front();	   // might cause an exception.
		queue_.pop_front();
		return true;
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
