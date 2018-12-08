#ifndef ADV_PROMISE_H
#define ADV_PROMISE_H

#include <exception>

#include "try.h"

namespace adv
{

template <typename T, typename CoreType>
class Future;

/**
 * \brief A non-shared promise with write once semantics. It allows to get one
 * corresponding non-shared future.
 */
template <typename T, typename CoreType>
class Promise : public CoreType
{
	public:
	using Parent = CoreType;
	using Type = T;
	using Self = Promise<T, CoreType>;
	using FutureType = typename Parent::FutureType;

	// Core methods:
	using Parent::Parent;

	Promise()
	{
	}
	Promise(Self &&other)
	{
	}
	Promise(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	// Derived methods:
	bool trySuccess(T &&v)
	{
		return Parent::tryComplete(Try<T>(std::move(v)));
	}

	bool tryFailure(std::exception_ptr e)
	{
		return Parent::tryComplete(Try<T>(std::move(e)));
	}

	template <typename Exception>
	bool tryFailure(Exception &&e)
	{
		return tryFailure(std::make_exception_ptr(std::move(e)));
	}

	void tryCompleteWith(FutureType &f) &&
	{
		f.onComplete([p = std::move(*this)](Try<T> t) mutable {
			p.tryComplete(std::move(t));
		});
	}

	/**
	 * Keeps the passed future alive until it is completed.
	 */
	void tryCompleteWithSafe(FutureType &&f) &&
	{
		auto ctx = std::make_shared<FutureType>(std::move(f));
		ctx->onComplete([ p = std::move(*this), ctx ](Try<T> t) mutable {
			p.tryComplete(std::move(t));
		});
	}

	void trySuccessWith(FutureType &f) &&
	{
		f.onComplete([p = std::move(*this)](Try<T> t) mutable {
			if (t.hasValue())
			{
				p.tryComplete(std::move(t));
			}
		});
	}

	void tryFailureWith(FutureType &f) &&
	{
		f.onComplete([p = std::move(*this)](Try<T> t) mutable {
			if (t.hasException())
			{
				p.tryComplete(std::move(t));
			}
		});
	}
};
}

#endif