#ifndef ADV_BOOST_FUTURE_H
#define ADV_BOOST_FUTURE_H

#include <mutex>

#include <boost/thread.hpp>

#include "../future.h"

namespace adv_boost
{

template <typename T>
class Promise;

template <typename T>
class Core
{
	public:
	using Self = Core<T>;
	using Type = T;
	Core()
	{
	}

	Core(Self &&other) : _f(std::move(other._f))
	{
	}

	Core(const Self &other) = delete;

	Self &operator=(const Self &other) = delete;

	Core(boost::future<T> &&f) : _f(std::move(f))
	{
	}

	T get()
	{
		try
		{
			return std::move(_f.get());
		}
		catch (const boost::broken_promise &e)
		{
			throw adv::BrokenPromise();
		}
	}

	bool isReady() const
	{
		return _f.is_ready();
	}

	template <typename Func>
	void onComplete(Func &&f)
	{
		// non-shared futures become invalid in Boost.Thread after registering one
		// callback.
		std::scoped_lock<std::mutex> lock(_callbackFutureMutex);
		if (_isCallbackSet)
		{
			throw adv::OnlyOneCallbackPerFuture();
		}
		this->_callbackFuture =
		    this->_f.then([f = std::move(f)](boost::future<T> && future) mutable {
			    f(convertFutureIntoTry(std::move(future)));
		    });
	}

	template <typename S>
	static Promise<S> createPromise();

	private:
	boost::future<T> _f;
	// TODO Do we have to store the callbackFuture? We could use a shared future.
	std::mutex _callbackFutureMutex;
	bool _isCallbackSet = false;
	boost::future<void> _callbackFuture;

	static adv::Try<T> convertFutureIntoTry(boost::future<T> &&f);
};

template <typename T>
class Future : public adv::Future<T, Core<T>>
{
	public:
	using CoreType = Core<T>;
	using Parent = adv::Future<T, Core<T>>;
	using Parent::Parent;

	/**
	 * Converting constructor allows implicit conversion from adv::Future<T, ...>
	 * into adv_boost::Future<T>.
	 */
	Future(Parent &&p) : Parent(std::move(p))
	{
	}
};
}

#endif
