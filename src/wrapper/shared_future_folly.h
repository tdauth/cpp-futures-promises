#ifndef SHAREDFUTURE_H
#define SHAREDFUTURE_H

#include <folly/futures/SharedPromise.h>

#include "extensions.h"
#include "future_folly.h"

namespace xtn
{

/**
 * Folly has no shared futures but they can be implemented with the help of a shared promise.
 * Whenever the shared future is copied it just passes a copy of its internal shared pointer.
 * Every instance sharing the same shared promise contains a single future instance.
 * This allows to treat it as separate future sharing the same result.
 *
 * Besides a get() call does not invalidate the future. Therefore, get() has multiple read semantics by default.
 * This is not the same for folly::Future (only when value is used).
 */
template<typename T>
class SharedFuture
{
	public:
		SharedFuture(Future<T> &&x) : me(new folly::SharedPromise<T>())
		{
			auto ctx = this->me;
			tryCompleteWith(*ctx, std::move(x._f), [ctx] () {});
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
		const T& get()
		{
			folly::Future<T> f = this->me->getFuture();
			f.wait();

			f.getTry().throwIfFailed();

			return f.value();
		}

		SharedFuture<T> orElse(SharedFuture<T> other)
		{
			return SharedFuture<T>(Future<T>(this->me->getFuture()).orElse(Future<T>(other.me->getFuture()))._f);
		}

		SharedFuture<T> first(SharedFuture<T> other)
		{
			return SharedFuture<T>(Future<T>(this->me->getFuture()).first(Future<T>(other.me->getFuture()))._f);
		}

	private:
		std::shared_ptr<folly::SharedPromise<T>> me;

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

	SharedFuture<VectorType> f(
			firstNSucc(std::move(mapped), n)
			.then([] (Try<std::vector<std::pair<std::size_t, T>>> t)
			{
				std::vector<std::pair<std::size_t, T>> vector = t.get(); // will rethrow the exception
				VectorType mapped;

				for (auto it = vector.begin(); it != vector.end(); ++it)
				{
					mapped.push_back(std::make_pair(it->first, std::move(it->second)));
				}

				return mapped;
			}
		)
	);

	return f;
}

}

#endif