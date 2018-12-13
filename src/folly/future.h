#ifndef ADV_FOLLY_FUTURE_H
#define ADV_FOLLY_FUTURE_H

#include <folly/futures/Future.h>

#include "../future.h"
#include "../try.h"
#include "util.h"

namespace adv_folly
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

	Core(folly::Executor *executor)
	    : executor(executor), _f(folly::Future<T>::makeEmpty())
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

	Core(folly::Executor *executor, folly::Future<T> &&f)
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
			return std::move(_f).get();
		}
		catch (const folly::BrokenPromise &e)
		{
			throw adv::BrokenPromise();
		}
	}

	bool isReady() const
	{
		return _f.isReady();
	}

	/**
	 * Each future can have only one callback due to the limitations of Folly.
	 * @param The callback function which gets a adv_folly::Try<T>.
	 * @throws adv::OnlyOneCallbackPerFuture If there is already a callback
	 * registered for this future.
	 */
	template <typename Func>
	void onComplete(Func &&f)
	{
		try
		{
			this->_f.setCallback_([f = std::move(f)](folly::Try<T> && t) mutable {
				f(tryFromFollyTry(std::move(t)));
			});
		}
		catch (const folly::FutureAlreadyContinued &)
		{
			throw adv::OnlyOneCallbackPerFuture();
		}
	}

	template <typename S>
	static Promise<S> createPromise(folly::Executor *ex);

	private:
	folly::Executor *executor;
	folly::Future<T> _f;
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
	 * into adv_folly::Future<T>.
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
