#ifndef ADV_PROMISE_H
#define ADV_PROMISE_H

#include <exception>

namespace adv
{

template<typename T>
class Future;

template<typename T>
class Try;

/**
 * \brief A non-shared promise with write once semantics. It allows to get one corresponding non-shared future.
 */
template<typename T>
class Promise
{
	public:
		// Core methods:
		Promise();
		Promise(Promise<T> &&other);
		Promise(const Promise<T> &other) = delete;
		Promise<T>& operator=(const Promise<T> &other) = delete;
		Future<T> future();
		bool tryComplete(Try<T> &&v);

		// Derived methods:
		bool trySuccess(T &&v);
		bool tryFailure(std::exception_ptr &&e);
		template<typename Exception>
		bool tryFailure(Exception &&e);
		void tryCompleteWith(Future<T> &&f);
		void trySuccessWith(Future<T> &&f);
		void tryFailureWith(Future<T> &&f);
};

}

#endif