#ifndef ADV_PROMISE_H
#define ADV_PROMISE_H

#include <exception>

#include "core.h"
#include "try.h"

namespace adv
{

template <typename T>
class Future;

/**
 * A shared promise with write once semantics. It allows to get one
 * corresponding shared future.
 */
template <typename T>
class Promise
{
	public:
	using Type = T;
	using Self = Promise<T>;
	using FutureType = Future<T>;
	using CoreType = typename Core<T>::SharedPtr;

	// Core methods:
	Promise() = delete;

	explicit Promise(
	    Executor *ex,
	    typename Core<T>::Implementation implementation = Core<T>::MVar)
	    : core(Core<T>::template createShared<T>(ex, implementation))
	{
	}

	~Promise()
	{
		core->decrementPromiseCounter();
	}

	Promise(const Self &other)
	{
		core = other.core;
		// TODO possible data race here?
		core->incrementPromiseCounter();
	}

	Self &operator=(const Self &other)
	{
		core = other.core;
		// TODO possible data race here?
		other.core->incrementPromiseCounter();
		return *this;
	}

	Future<T> future();

	bool tryComplete(Try<T> &&v)
	{
		return core->tryComplete(std::move(v));
	}

	bool tryComplete(const Try<T> &v)
	{
		return core->tryComplete(Try<T>(v));
	}

	// Derived methods:
	bool trySuccess(T &&v)
	{
		return core->tryComplete(Try<T>(std::move(v)));
	}

	bool tryFailure(std::exception_ptr e)
	{
		return core->tryComplete(Try<T>(std::move(e)));
	}

	template <typename Exception>
	bool tryFailure(Exception &&e)
	{
		return tryFailure(std::make_exception_ptr(std::move(e)));
	}

	void tryCompleteWith(FutureType f)
	{
		f.onComplete([p = *this](const Try<T> &t) mutable { p.tryComplete(t); });
	}

	void trySuccessWith(FutureType f)
	{
		f.onComplete([p = *this](const Try<T> &t) mutable {
			if (t.hasValue())
			{
				p.tryComplete(t);
			}
		});
	}

	void tryFailureWith(FutureType f)
	{
		f.onComplete([p = *this](const Try<T> &t) mutable {
			if (t.hasException())
			{
				p.tryComplete(t);
			}
		});
	}

	private:
	CoreType core;
};
} // namespace adv

#endif