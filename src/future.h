#ifndef ADV_FUTURE_H
#define ADV_FUTURE_H

#include <exception>
#include <utility>
#include <vector>

namespace adv
{

/**
 * \brief This exception is thrown when the result of a future is retrieved but
 * the corresponding promise has already been deleted before completing the
 * future meaning the future would never be completed by the promise.
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

template <typename T>
class Try;

class Executor;

/**
 * \brief A non-shared future which can only be moved around and has read once
 * semantics. It can only get one callback.
 */
template <typename T, typename CoreType>
class Future : public CoreType
{
	public:
	using Parent = CoreType;
	using Type = T;
	using Self = Future<T, CoreType>;

	// Core methods:
	using Parent::Parent;

	Future(Self &&other) : Parent(std::move(other))
	{
	}
	Self &operator=(Self &&other)
	{
		return Parent::operator=(std::move(other));
	}

	Future(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	// Derived methods:
	template <typename Func>
	void onSuccess(Func &&f);
	template <typename Func>
	void onFailure(Func &&f);

	template <typename Func>
	Future<typename std::result_of<Func(Try<T>)>::type,
	       typename CoreType::template CoreType<
	           typename std::result_of<Func(Try<T>)>::type>>
	then(Func &&f);

	template <typename Func>
	typename std::result_of<Func(Try<T>)>::type thenWith(Func &&f);

	template <typename Func>
	Self guard(Func &&f)
	{
		return this->then([f{std::move(f)}](Try<T> && t) mutable {
			auto x = std::move(t).get();

			if (!f(x))
			{
				throw adv::PredicateNotFulfilled();
			}

			return x;
		});
	}

	Self orElse(Self &&other);

	Self first(Self &other);

	Self firstSucc(Self &other);

	static Self successful(T &&v);
	template <typename E>
	static Self failed(E &&e);
};

// Derived methods:
template <typename PromiseType, typename Executor, typename Func>
typename PromiseType::FutureType async(Executor *ex, Func &&f);
template <typename FutureType>
typename FutureType::CoreType::template FutureType<
    std::vector<std::pair<std::size_t, Try<typename FutureType::Type>>>>
firstN(std::vector<FutureType> &&c, std::size_t n);
template <typename FutureType>
typename FutureType::CoreType::template FutureType<
    std::vector<std::pair<std::size_t, typename FutureType::Type>>>
firstNSucc(std::vector<FutureType> &&c, std::size_t n);
}

#endif
