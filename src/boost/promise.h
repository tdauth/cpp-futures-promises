#ifndef ADV_BOOST_PROMISE_H
#define ADV_BOOST_PROMISE_H

#include "../promise.h"
#include "future.h"

namespace adv_boost
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
		this->_p = std::move(other.p);
		return *this;
	}

	CorePromise(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	FutureType future()
	{
		return FutureType(executor, _p.get_future());
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
	folly::Executor *executor;
	boost::promise<T> _p;
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
