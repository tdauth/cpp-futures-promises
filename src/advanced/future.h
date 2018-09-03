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
		Try();
		Try(T &&v);
		Try(std::exception_ptr &&e);
		Try(Try<T> &&other);

		T get();
		const T& get() const;
		bool hasValue();
		bool hasException();
};

class Executor
{
	public:
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

template<typename T>
class SharedFuture;

/**
 * \brief A non-shared future which can only be moved around and has read once semantics. It can only get one callback.
 */
template<typename T>
class Future
{
	public:
		Future() {}
		Future(Future<T> &&other) {}
		Future(const Future<T> &other) = delete;
		Future<T>& operator=(const Future<T> &other) = delete;

		SharedFuture<T> share();

		T get();

		bool isReady();

		template<typename Func, typename S>
		Future<S> then(Func &&f);

		template<typename Func>
		void onComplete(Func &&f); // (D)

		template<typename Func>
		Future<T> guard(Func &&f); // (D)

		Future<T> orElse(Future<T> &&other); // (D)

		Future<T> first(Future<T> &&other); // (D)

		Future<T> firstSucc(Future<T> &&other); // (D)
};

template<typename Func, typename T>
Future<T> async(Executor *ex, Func &&f);

template<typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n); // (D)

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n); // (D)

}

#endif