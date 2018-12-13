#ifndef ADV_BOOST_FUTURE_H
#define ADV_BOOST_FUTURE_H

#include <mutex>

#include <boost/thread.hpp>

#include "../future.h"
#include "util.h"

namespace adv_boost
{

template <typename T>
class SharedFuture;

template <typename T>
class Promise;

template <typename T>
class Future;

template <typename T>
class Core
{
	public:
	using Self = Core<T>;
	using Type = T;

	template <typename S>
	using CoreType = Core<S>;
	template <typename S>
	using PromiseType = Promise<S>;

	template <typename S>
	using FutureType = Future<S>;

	Core(folly::Executor *executor) : executor(executor)
	{
	}

	Core(Self &&other) : executor(other.executor), _f(std::move(other._f))
	{
	}

	Self &operator=(Self &&other)
	{
		this->executor = other.executor;
		this->_f = std::move(other._f);
		return *this;
	}

	Core(folly::Executor *executor, boost::future<T> &&f)
	    : executor(executor), _f(std::move(f))
	{
	}

	Core(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	SharedFuture<T> share();

	folly::Executor *getExecutor() const
	{
		return executor;
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
		this->_callbackFuture = this->_f.then([
			f = std::move(f), executor = this->executor
		](boost::future<T> && future) mutable {
			executor->add([ f = std::move(f), future = std::move(future) ]() mutable {
				f(convertFutureIntoTry(std::move(future)));
			});
		});
		_isCallbackSet = true;
	}

	template <typename S>
	static Promise<S> createPromise(folly::Executor *executor);

	private:
	folly::Executor *executor;
	boost::future<T> _f;
	// TODO Do we have to store the callbackFuture? We could use a shared future.
	// Use a boolean atomic variable.
	std::mutex _callbackFutureMutex;
	bool _isCallbackSet = false;
	boost::future<void> _callbackFuture;
};

template <typename T>
class Future : public adv::Future<T, Core<T>>
{
	public:
	using Self = Future<T>;
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

	Self &operator=(Parent &&p)
	{
		return Parent::operator=(std::move(p));
	}
};
}

#endif
