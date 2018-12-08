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

template<typename T>
class Promise;

template <typename T>
class Core
{
	public:
	using Type = T;

	template<typename S>
	using CoreType = Core<S>;
	template<typename S>
	using PromiseType = Promise<S>;

	// Core methods:
	Core() : _f(folly::Future<T>::makeEmpty())
	{
	}

	Core(Core<T> &&other) : _f(std::move(other._f))
	{
	}

	Core(const Core<T> &other) = delete;
	Core<T> &operator=(const Core<T> &other) = delete;

	Core(folly::Future<T> &&f) : _f(std::move(f))
	{
	}

	SharedFuture<T> share();

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

    template<typename S>
    static Promise<S> createPromise();

	private:
	folly::Future<T> _f;
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
	 * into adv_folly::Future<T>.
	 */
	Future(Parent &&p) : Parent(std::move(p))
	{
	}
};
}

#endif
