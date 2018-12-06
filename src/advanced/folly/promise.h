#ifndef ADV_PROMISEFOLLY_H
#define ADV_PROMISEFOLLY_H

#include "future.h"

namespace adv_folly
{

template<typename T>
class Promise
{
	public:
		// Core methods:
		Promise()
		{
		}

		Promise(Promise<T> &&other) : _p(std::move(other._p))
		{
		}

		Promise(const Promise<T> &other) = delete;
		Promise<T>& operator=(const Promise<T> &other) = delete;

		Promise(folly::Promise<T> &&p) : _p(std::move(p))
		{
		}

		Future<T> future()
		{
			return _p.getFuture();
		}

		bool tryComplete(Try<T> &&v)
		{
			return ::tryComplete(this->_p, std::move(v._t));
		}

		// Derived methods:
		bool trySuccess(T &&v)
		{
			return tryComplete(Try<T>(std::move(v)));
		}

		bool tryFailure(std::exception_ptr e)
		{
			return tryComplete(Try<T>(std::move(e)));
		}

		template<typename Exception>
		bool tryFailure(Exception &&e)
		{
			return tryFailure(std::make_exception_ptr(std::move(e)));
		}

		void tryCompleteWith(Future<T> &f) &&
		{
			f.onComplete([p = std::move(*this)] (Try<T> t) mutable
				{
					p.tryComplete(std::move(t));
				}
			);
		}

		/**
		 * Keeps the passed future alive until it is completed.
		 */
		void tryCompleteWithSafe(Future<T> &&f) &&
		{
			auto ctx = std::make_shared<Future<T>>(std::move(f));
            ctx->onComplete([p = std::move(*this), ctx] (Try<T> t) mutable
						 {
							 p.tryComplete(std::move(t));
						 }
			);
		}

		void trySuccessWith(Future<T> &f) &&
		{
			f.onComplete([p = std::move(*this)] (Try<T> t) mutable
				{
					if (t.hasValue())
					{
						p.tryComplete(std::move(t));
					}
				}
			);
		}

		void tryFailureWith(Future<T> &f) &&
		{
			f.onComplete([p = std::move(*this)] (Try<T> t) mutable
				{
					if (t.hasException())
					{
						p.tryComplete(std::move(t));
					}
				}
			);
		}

	private:
		folly::Promise<T> _p;
};

}

#endif