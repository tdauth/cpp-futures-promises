#ifndef ADV_FUTURE_H
#define ADV_FUTURE_H

#include <exception>
#include <utility>
#include <vector>

#include <folly/Executor.h>

#include "core.h"
#include "try.h"

namespace adv
{

class PredicateNotFulfilled : public std::exception
{
};

template <typename T>
class Promise;

/**
 * A shared future which can be copied around and has multiple read semantics.
 * It can get multiple callbacks.
 */
template <typename T>
class Future
{
	public:
	using Type = T;
	using Self = Future<T>;
	using CoreType = typename Core<T>::SharedPtr;

	// Core methods:
	Future() = delete;

	~Future()
	{
	}

	Future(const Self &other)
	{
		this->_s = other._s;
	}

	Self &operator=(const Self &other)
	{
		return *this;
	}

	folly::Executor *getExecutor() const
	{
		return _s->getExecutor();
	}

	const Try<T> &get()
	{
		return _s->get();
	}

	bool isReady() const
	{
		return _s->isReady();
	}

	void onComplete(typename Core<T>::Callback &&h)
	{
		_s->onComplete(std::move(h));
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

	Self orElse(Self other);

	Self first(Self other);

	/**
	 * @return A new future which is completed with the first successful future of
	 * this and other. If both futures fail, it will be comdpleted with \ref
	 * BrokenPromise.
	 */
	Self firstSucc(Self other);

	private:
	CoreType _s;

	template <typename S>
	friend class Promise;

	explicit Future(CoreType s) : _s(s)
	{
	}

	template <typename S>
	Promise<S> createPromise()
	{
		return Promise<S>(getExecutor());
	}
};

// Derived methods:
template <typename Func>
Future<typename std::result_of<Func()>::type> async(folly::Executor *ex,
                                                    Func &&f);

template <typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>>
firstN(folly::Executor *ex, std::vector<Future<T>> &&futures, std::size_t n);

template <typename T>
Future<std::vector<std::pair<std::size_t, T>>>
firstNSucc(folly::Executor *ex, std::vector<T> &&futures, std::size_t n);
} // namespace adv

#endif
