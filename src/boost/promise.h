#ifndef ADV_BOOST_PROMISE_H
#define ADV_BOOST_PROMISE_H

#include "../promise.h"
#include "future.h"

namespace adv_boost
{

template <typename T, typename F>
class CorePromise
{
	public:
	using Self = CorePromise<T, F>;
	using FutureType = F;
	// Core methods:
	CorePromise()
	{
	}

	CorePromise(Self &&other) : _p(std::move(other._p))
	{
	}

	CorePromise(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	CorePromise(boost::promise<T> &&p) : _p(std::move(p))
	{
	}

	FutureType future()
	{
		return _p.get_future();
	}

	bool tryComplete(adv::Try<T> &&v)
	{
		try
		{
			this->_p.set_value(std::move(v).get());
		}
		catch (const boost::promise_already_satisfied &e)
		{
			return false;
		}
		catch (...)
		{
			this->_p.set_exception(std::move(boost::current_exception()));
		}

		return true;
	}

	private:
	boost::promise<T> _p;
};

template <typename T>
class Promise : public adv::Promise<T, CorePromise<T, Future<T>>>
{
	public:
	using Parent = adv::Promise<T, Future<T>, CorePromise<T, Future<T>>>;
	using Parent::Parent;
};
}

#endif
