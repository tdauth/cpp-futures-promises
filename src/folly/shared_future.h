#ifndef ADV_FOLLY_SHAREDFUTURE_H
#define ADV_FOLLY_SHAREDFUTURE_H

#include <folly/futures/SharedPromise.h>

#include "future.h"

namespace adv_folly
{

/**
 * Folly has no shared futures but they can be implemented with the help of a
 * shared promise. Whenever the shared future is copied it just passes a copy of
 * its internal shared pointer. Every instance sharing the same shared promise
 * contains a single future instance. This allows to treat it as separate future
 * sharing the same result.
 *
 * Besides a get() call does not invalidate the future. Therefore, get() has
 * multiple read semantics by default. It does also allow registering multiple
 * callbacks instead of only one per future. This is not the same for
 * folly::Future (only when value is used).
 *
 * TODO Try to make it more abstract and use `adv::SharedFuture<T>`.
 */
template <typename T>
class SharedFuture
{
	public:
	using SharedPtr = std::shared_ptr<folly::SharedPromise<T>>;
	using Self = SharedFuture<T>;
	using Type = T;
	template <typename S>
	using PromiseType = Promise<S>;
	using FutureType = Future<T>;
	static constexpr bool IsShared = true;

	/**
	 * Allow conversion from adv::Future<T, ...> into adv_folly::SharedFuture<T>.
	 */
	SharedFuture(typename FutureType::Parent &&x)
	    : ex(x.getExecutor()), me(new folly::SharedPromise<T>())
	{
		struct SharedContext
		{
			SharedContext(SharedPtr ptr, FutureType &&f) : ptr(ptr), f(std::move(f))
			{
			}

			SharedPtr ptr;
			Future<T> f;
		};

		auto ctx = std::make_shared<SharedContext>(this->me, std::move(x));

		ctx->f.onComplete([ctx](adv::Try<T> t) {
			try
			{
				ctx->ptr->setValue(std::move(t).get());
			}
			catch (const std::exception &e)
			{
				ctx->ptr->setException(
				    folly::exception_wrapper(std::current_exception(), e));
			}
			catch (...)
			{
				ctx->ptr->setException(folly::exception_wrapper(std::current_exception()));
			}
		});
	}

	// TODO use the constructor above?
	SharedFuture(FutureType &&x)
	    : ex(x.getExecutor()), me(new folly::SharedPromise<T>())
	{
		struct SharedContext
		{
			SharedContext(SharedPtr ptr, FutureType &&f) : ptr(ptr), f(std::move(f))
			{
			}

			SharedPtr ptr;
			Future<T> f;
		};

		auto ctx = std::make_shared<SharedContext>(this->me, std::move(x));

		ctx->f.onComplete([ctx](adv::Try<T> t) {
			try
			{
				ctx->ptr->setValue(std::move(t).get());
			}
			catch (const std::exception &e)
			{
				ctx->ptr->setException(
				    folly::exception_wrapper(std::current_exception(), e));
			}
			catch (...)
			{
				ctx->ptr->setException(folly::exception_wrapper(std::current_exception()));
			}
		});
	}

	SharedFuture(const Self &other) : ex(other.ex), me(other.me)
	{
	}

	Self &operator=(const Self &other)
	{
		ex = other.ex;
		me = other.me;

		return *this;
	}

	/**
	 * Supports read many semantics.
	 * Does not move the result value out, neither does copy it to keep the
	 * performance. This is similar to get() of std::shared_future.
	 */
	T get()
	{
		folly::Future<T> f = getFollyFuture();
		f.wait();

		f.getTry().throwIfFailed();

		// TODO Does this move out the whole shared state of the shared promise?? Only
		// read with const!
		return std::move(f).value();
	}

	/**
	 * Supports multiple callbacks.
	 * Creates a new future for each callback which deletes itself when the callback has been executed.
	 * TODO The returned future seems to already have a callback if one has been registered?
	 */
	template <typename Func>
	void onComplete(Func &&f)
	{
		auto ctx = std::make_shared<FutureType>(getFuture());
		/*
		 * The future deletes itself when it is completed.
		 */
		ctx->onComplete(
		    [ f = std::move(f), ctx ](adv::Try<T> t) { f(std::move(t)); });
	}

	bool isReady()
	{
		return getFuture().isReady();
	}

	template <typename Func>
	Self guard(Func &&f)
	{
		return getFuture().guard(std::move(f));
	}

	template <typename Func>
	SharedFuture<typename std::result_of<Func(adv::Try<T>)>::type> then(Func &&f)
	{
		using S = typename std::result_of<Func(adv::Try<T>)>::type;
		using CoreS = Core<S>;
		return getFuture().template then<CoreS>(std::move(f));
	}

	Self orElse(SharedFuture<T> other)
	{
		return getFuture().orElse(other.getFuture());
	}

	Self first(SharedFuture<T> other)
	{
		auto otherF = std::make_shared<FutureType>(other.getFuture());
		using CoreT = Core<T>;

		return getFuture().first(*otherF).template then<CoreT>(
		    [otherF](adv::Try<T> &&t) { return t.get(); });
	}

	Self firstSucc(SharedFuture<T> other)
	{
		auto otherF = std::make_shared<FutureType>(other.getFuture());
		using CoreT = Core<T>;

		return getFuture().firstSucc(*otherF).template then<CoreT>(
		    [otherF](adv::Try<T> &&t) { return t.get(); });
	}

	static Self successful(folly::Executor *ex, T &&v);
	template <typename E>
	static Self failed(folly::Executor *ex, E &&e);

	SharedFuture() = delete;
	SharedFuture(folly::Executor *ex, Self &&other)
	    : ex(ex), me(std::move(other.me))
	{
	}

	template <typename S>
	static Promise<S> createPromise(folly::Executor *ex);

	private:
	folly::Executor *ex;
	SharedPtr me;

	folly::Future<T> getFollyFuture()
	{
		return this->me->getFuture();
	}

	Future<T> getFuture()
	{
		return Future<T>(ex, getFollyFuture());
	}

	static adv::Try<T> tryFromFollyTry(folly::Try<T> &&t);

	template <typename U>
	friend SharedFuture<std::vector<std::pair<std::size_t, adv::Try<U>>>>
	firstN(std::vector<SharedFuture<U>> &&c, std::size_t n);

	template <typename U>
	friend SharedFuture<std::vector<std::pair<std::size_t, U>>>
	firstNSucc(std::vector<SharedFuture<U>> &&c, std::size_t n);
};

template <typename T>
SharedFuture<std::vector<std::pair<std::size_t, adv::Try<T>>>>
firstN(std::vector<SharedFuture<T>> &&c, std::size_t n)
{
	std::vector<Future<T>> mapped;

	for (auto it = c.begin(); it != c.end(); ++it)
	{
		mapped.push_back(it->me->getFuture());
	}

	using VectorType = std::vector<std::pair<std::size_t, adv::Try<T>>>;

	return SharedFuture<VectorType>(firstN(std::move(mapped), n));
}

template <typename T>
SharedFuture<std::vector<std::pair<std::size_t, T>>>
firstNSucc(std::vector<SharedFuture<T>> &&c, std::size_t n)
{
	std::vector<Future<T>> mapped;

	for (auto it = c.begin(); it != c.end(); ++it)
	{
		mapped.push_back(it->me->getFuture());
	}

	using VectorType = std::vector<std::pair<std::size_t, T>>;

	return SharedFuture<VectorType>(firstNSucc(std::move(mapped), n));
}
}

#endif