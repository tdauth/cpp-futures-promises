#ifndef ADV_FUTURE_H
#define ADV_FUTURE_H

#include <vector>
#include <utility>
#include <exception>

namespace adv
{

class UsingUninitializedTry : public std::exception
{
};

template<typename T>
class Try
{
	public:
		// Core methods:
		Try();
		Try(T &&v);
		Try(std::exception_ptr &&e);
		Try(Try<T> &&other);
		T get() &&;
		bool hasValue();
		bool hasException();
};

class Executor
{
	public:
		// Core methods:
		template<typename Func>
		void submit(Func &&f);
};

/**
 * \brief This exception is thrown when the result of a future is retrieved but the corresponding promise has already been deleted before completing the future meaning the future would never be completed by the promise.
 */
class BrokenPromise : public std::exception
{
};

class PredicateNotFulfilled : public std::exception
{
};

class OnlyOneCallbackPerFuture : public std::exception
{
};

template<typename T>
class SharedFuture;

template<typename T>
class Promise;

/**
 * \brief A non-shared future which can only be moved around and has read once semantics. It can only get one callback.
 */
template<typename T>
class Future
{
	public:
        using type = T;

		// Core methods:
		Future() {}
		Future(Future<T> &&other) {}
		Future(const Future<T> &other) = delete;
		Future<T>& operator=(const Future<T> &other) = delete;
		SharedFuture<T> share();
		T get();
		bool isReady() const;
		template<typename Func>
		void onComplete(Func &&f);

		// Derived methods:
		template<typename Func, typename S>
		Future<S> then(Func &&f);
		template<typename Func, typename S>
		Future<S> thenWith(Func &&f);
		template<typename Func>
		Future<T> guard(Func &&f);
		Future<T> orElse(Future<T> &&other);
		Future<T> first(Future<T> &other);
		Future<T> firstSucc(Future<T> &other);
};

// Core methods:
// TODO Derived from a constructor with an executor.
template<typename Func, typename T>
Future<T> async(Executor *ex, Func &&f);

// Derived methods:
template<typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n);
template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n);

}

#endif
