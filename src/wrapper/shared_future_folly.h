#ifndef SHAREDFUTURE_H
#define SHAREDFUTURE_H

#include <folly/futures/SharedPromise.h>

#include "extensions.h"

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
		SharedFuture(folly::Future<T> &&x) : me(new folly::SharedPromise<T>()), f(me->getFuture())
		{
			auto ctx = this->me;
			tryCompleteWith(*ctx, std::move(x), [ctx] () {});
		}

		SharedFuture(const SharedFuture<T> &other) : me(other.me), f(me->getFuture())
		{
		}

		SharedFuture<T>& operator=(const SharedFuture<T> &other)
		{
			me = other.me;
			f = me->getFuture();

			return *this;
		}

		/**
		 * Supports read many semantics.
		 * Does not move the result value out, neither does copy it to keep the performance.
		 * This is similar to get() of std::shared_future.
		 */
		const T& get()
		{
			this->f.wait();

			this->f.getTry().throwIfFailed();

			return this->f.value();
		}

	private:
		std::shared_ptr<folly::SharedPromise<T>> me;
		folly::Future<T> f;
};

}

#endif