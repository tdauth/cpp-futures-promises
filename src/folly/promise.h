#ifndef ADV_FOLLY_PROMISE_H
#define ADV_FOLLY_PROMISE_H

#include "../promise.h"
#include "../try.h"
#include "future.h"

namespace adv_folly
{

template <typename T>
class CorePromise
{
	public:
	using Type = T;
	using Self = CorePromise<T>;
	using FutureType = Future<T>;

	CorePromise(folly::Executor *executor) : executor(executor)
	{
	}

	CorePromise(Self &&other) : executor(other.executor), _p(std::move(other._p))
	{
	}

	Self &operator=(Self &&other)
	{
		this->executor = other.executor;
		this->_p = std::move(other._p);
		return *this;
	}

	CorePromise(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	folly::Executor *getExecutor() const
	{
		return executor;
	}

	FutureType future()
	{
		return FutureType(executor, _p.getFuture());
	}

	bool tryComplete(adv::Try<T> &&t)
	{
		try
		{
			_p.setTry(follyTryFromTry(std::move(t)));
		}
		catch (const folly::PromiseAlreadySatisfied &e)
		{
			return false;
		}

		return true;
	}

	private:
	folly::Executor *executor;
	folly::Promise<T> _p;
};

template <typename T>
class Promise : public adv::Promise<T, CorePromise<T>>
{
	public:
	using Parent = adv::Promise<T, CorePromise<T>>;
	using Parent::Parent;
};
}

#endif