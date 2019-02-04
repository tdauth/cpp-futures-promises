#ifndef ADV_PROMISE_H
#define ADV_PROMISE_H

#include <exception>

#include <folly/Executor.h>

#include "core.h"
#include "try.h"

namespace adv
{

template <typename T>
class Future;

/**
 * \brief A shared promise with multiple writesemantics. It allows to get one
 * corresponding shared future.
 */
template <typename T>
class Promise
{
	public:
	using Type = T;
	using Self = Promise<T>;
	using FutureType = Future<T>;
	using CoreType = std::shared_ptr<Core<T>>;

	// Core methods:
	Promise(CoreType s) : _s(s->incrementPromiseCounts(s))
	{
	}

	Promise(Self &&other) : _s(std::move(other._s))
	{
	}

	~Promise()
	{
		/*
		 * TODO break promise if necessary
		if (_s->getPromiseCountsAndDecrement() == 1)
		{
		 tryComplete(Try<T>(std::make_exception_ptr(BrokenPromise())));
		}
		 */
	}

	Self &operator=(Self &&other)
	{
		_s = std::move(other._s);

		return *this;
	}

	Promise(const Self &other)
	{
		this->_s = other._s;
	}

	Self &operator=(const Self &other)
	{
		this->_s = other._s;
		return *this;
	}

	Future<T> future();

	bool tryComplete(Try<T> &&v)
	{
		return _s->tryComplete(std::move(v));
	}

	// Derived methods:
	bool trySuccess(T &&v)
	{
		return _s->tryComplete(Try<T>(std::move(v)));
	}

	bool tryFailure(std::exception_ptr e)
	{
		return _s->tryComplete(Try<T>(std::move(e)));
	}

	template <typename Exception>
	bool tryFailure(Exception &&e)
	{
		return tryFailure(std::make_exception_ptr(std::move(e)));
	}

	void tryCompleteWith(FutureType &f)
	{
		Promise<T> p(*this);
		f.onComplete([p](const Try<T> &t) mutable { p.tryComplete(Try<T>(t)); });
	}

	void trySuccessWith(FutureType &f)
	{
		Promise<T> p(*this);
		f.onComplete([p](const Try<T> &t) mutable {
			if (t.hasValue())
			{
				p.tryComplete(Try<T>(t));
			}
		});
	}

	void tryFailureWith(FutureType &f)
	{
		Promise<T> p(*this);
		f.onComplete([p](const Try<T> &t) mutable {
			if (t.hasException())
			{
				p.tryComplete(Try<T>(t));
			}
		});
	}

	private:
	CoreType _s;
};
} // namespace adv

#endif