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
	Promise(folly::Executor *ex) : _s(Core<T>::template createShared<T>(ex))
	{
	}

	explicit Promise(CoreType s) : _s(s)
	{
	}

	Promise(Self &&other) noexcept : _s(std::move(other._s))
	{
	}

	~Promise()
	{
		_s->decrementPromiseCounter();
	}

	Self &operator=(Self &&other) noexcept
	{
		_s = std::move(other._s);

		return *this;
	}

	Promise(const Self &other)
	{
		other._s->incrementPromiseCounter();
		// TODO possible data race here?
		this->_s = other._s;
	}

	Self &operator=(const Self &other)
	{
		other._s->incrementPromiseCounter();
		// TODO possible data race here?
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