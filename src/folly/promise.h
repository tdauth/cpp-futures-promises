#ifndef ADV_PROMISEFOLLY_H
#define ADV_PROMISEFOLLY_H

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

	// Core methods:
	CorePromise()
	{
	}

	CorePromise(Self &&other) : _p(std::move(other._p))
	{
	}

	Self &operator=(Self &&other)
	{
		this->_p = std::move(other._p);
		return *this;
	}

	CorePromise(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	CorePromise(folly::Promise<T> &&p) : _p(std::move(p))
	{
	}

	FutureType future()
	{
		return _p.getFuture();
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