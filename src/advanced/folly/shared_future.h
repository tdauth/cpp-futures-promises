#ifndef ADV_FOLLY_SHAREDFUTURE_H
#define ADV_FOLLY_SHAREDFUTURE_H

#include <folly/futures/SharedPromise.h>

#include "future.h"

namespace adv_folly
{

/**
 * Folly has no shared futures but they can be implemented with the help of a shared promise.
 * Whenever the shared future is copied it just passes a copy of its internal shared pointer.
 * Every instance sharing the same shared promise contains a single future instance.
 * This allows to treat it as separate future sharing the same result.
 *
 * Besides a get() call does not invalidate the future. Therefore, get() has multiple read semantics by default.
 * It does also allow registering multiple callbacks instead of only one per future.
 * This is not the same for folly::Future (only when value is used).
 */
template<typename T>
class SharedFuture
{
	public:
		typedef std::shared_ptr<folly::SharedPromise<T>> SharedPtr;

		SharedFuture(Future<T> &&x) : me(new folly::SharedPromise<T>())
		{
			struct SharedContext
			{
				SharedContext(SharedPtr ptr, Future<T> &&f) : ptr(ptr), f(std::move(f))
				{}

				SharedPtr ptr;
				Future<T> f;
			};


			auto ctx = std::make_shared<SharedContext>(this->me, std::move(x));

			ctx->f.onComplete([ctx] (Try<T> t)
			{
				try
				{
					ctx->ptr->setValue(std::move(t).get());
				}
				catch (const std::exception &e)
				{
					ctx->ptr->setException(folly::exception_wrapper(std::current_exception(), e));
				}
				catch (...)
				{
					ctx->ptr->setException(folly::exception_wrapper(std::current_exception()));
				}
			});
		}

		SharedFuture(const SharedFuture<T> &other) : me(other.me)
		{
		}

		SharedFuture<T>& operator=(const SharedFuture<T> &other)
		{
			me = other.me;

			return *this;
		}

		/**
		 * Supports read many semantics.
		 * Does not move the result value out, neither does copy it to keep the performance.
		 * This is similar to get() of std::shared_future.
		 */
		T get()
		{
			folly::Future<T> f = this->me->getFuture();
			f.wait();

			f.getTry().throwIfFailed();

			// TODO Does this move out the whole shared state of the shared promise?? Only read with const!
			return std::move(f).value();
		}

		template<typename Func>
		void onComplete(Func &&f)
		{
			auto ctx = std::make_shared<Future<T>>(Future<T>(this->me->getFuture()));
			/*
			 * The future deletes itself when it is completed.
			 */
			ctx->onComplete([f = std::move(f), ctx] (Try<T> t) { f(std::move(t)); });
		}

		bool isReady()
		{
			return Future<T>(this->me->getFuture()).isReady();
		}

		template<typename Func>
		SharedFuture<T> guard(Func &&f)
		{
			return Future<T>(this->me->getFuture()).guard(std::move(f));
		}

		template<typename Func>
		SharedFuture<typename std::result_of<Func(Try<T>)>::type> then(Func &&f)
		{
			return Future<T>(this->me->getFuture()).then(std::move(f));
		}

		SharedFuture<T> orElse(SharedFuture<T> other)
		{
			return Future<T>(this->me->getFuture()).orElse(Future<T>(other.me->getFuture()));
		}

		SharedFuture<T> first(SharedFuture<T> other)
		{
			auto otherF = std::make_shared<Future<T>>(other.me->getFuture());
			return Future<T>(this->me->getFuture()).first(*otherF).then([otherF] (Try<T> &&t) { return t.get(); });
		}

		SharedFuture<T> firstSucc(SharedFuture<T> other)
		{
			auto otherF = std::make_shared<Future<T>>(other.me->getFuture());
			return Future<T>(this->me->getFuture()).firstSucc(*otherF).then([otherF] (Try<T> &&t) { return t.get(); });
		}

		SharedFuture() = delete;
		SharedFuture(SharedFuture<T> &&other) : me(std::move(other.me))
		{
		}

	private:
		SharedPtr me;

		template<typename U>
		friend SharedFuture<std::vector<std::pair<std::size_t, Try<U>>>> firstN(std::vector<SharedFuture<U>> &&c, std::size_t n);

		template<typename U>
		friend SharedFuture<std::vector<std::pair<std::size_t, U>>> firstNSucc(std::vector<SharedFuture<U>> &&c, std::size_t n);
};

template<typename T>
SharedFuture<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<SharedFuture<T>> &&c, std::size_t n)
{
	std::vector<Future<T>> mapped;

	for (auto it = c.begin(); it != c.end(); ++it)
	{
		mapped.push_back(it->me->getFuture());
	}

	using VectorType = std::vector<std::pair<std::size_t, Try<T>>>;

	return SharedFuture<VectorType>(firstN(std::move(mapped), n));
}

template<typename T>
SharedFuture<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<SharedFuture<T>> &&c, std::size_t n)
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