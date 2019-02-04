#ifndef ADV_FUTURE_H
#define ADV_FUTURE_H

#include <exception>
#include <utility>
#include <vector>

#include <folly/Executor.h>

#include "state.h"
#include "try.h"

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

template <typename T>
class Promise;

/**
 * \brief A shared future which can be copied around and has multiple read
 * semantics. It can get multiple callbacks.
 */
template <typename T>
class Future
{
	public:
	using Type = T;
	using Self = Future<T>;
	using StateType = std::shared_ptr<State<T>>;

	// Core methods:

	Future(StateType s) : _s(s)
	{
	}

	Future(Self &&other) : _s(std::move(other._s))
	{
	}

	Self &operator=(Self &&other)
	{
		this->_s = std::move(other._s);

		return *this;
	}

	Future(const Self &other)
	{
		this->_s = other._s;
	}

	Self &operator=(const Self &other)
	{
		this->_s = other._s;
		return *this;
	}

	folly::Executor *getExecutor() const
	{
		return _s->getExecutor();
	}

	T get()
	{
		return _s->get();
	}

	bool isReady() const
	{
		return _s->isReady();
	}

	void onComplete(typename State<T>::Callback &&h)
	{
		return _s->onComplete(std::move(h));
	}

	// Derived methods:
	template <typename Func>
	void onSuccess(Func &&f);
	template <typename Func>
	void onFailure(Func &&f);

	template <typename Func>
	Future<typename std::result_of<Func(const Try<T> &)>::type> then(Func &&f);

	template <typename Func>
	typename std::result_of<Func(const Try<T> &)>::type thenWith(Func &&f);

	/**
	 *
	 * @tparam Func
	 * @param f
	 * @return
	 * @throw PredicateNotFulfilled When the function returns false, this exception
	 * will be thrown.
	 */
	template <typename Func>
	Self guard(Func &&f)
	{
		return this->then([f{std::move(f)}](const Try<T> &t) mutable {
			auto x = t.get();

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

	private:
	StateType _s;

	template <typename S>
	Promise<S> createPromise()
	{
		return Promise<S>(State<T>::template createShared<S>(getExecutor()));
	}
};

// Derived methods:
template <typename Func>
Future<typename std::result_of<Func()>::type> async(folly::Executor *ex,
                                                    Func &&f);
template <typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>>
firstN(folly::Executor *ex, std::vector<Future<T>> &&c, std::size_t n);
template <typename T>
Future<std::vector<std::pair<std::size_t, T>>>
firstNSucc(folly::Executor *ex, std::vector<T> &&c, std::size_t n);
} // namespace adv

#endif
