#ifndef ADV_PROMISEFOLLY_H
#define ADV_PROMISEFOLLY_H

#include "future_folly.h"

namespace adv
{

template<typename T>
class Promise
{
	public:
		Future<T> future()
		{
			return _p.getFuture();
		}

		bool tryComplete(Try<T> &&v)
		{
			return ::tryComplete(this->_p, std::move(v._t));
		}

		bool trySuccess(T &&v)
		{
			return tryComplete(Try<T>(std::move(v)));
		}

		bool tryFailure(std::exception_ptr e)
		{
			return tryComplete(Try<T>(std::move(e)));
		}

		template<typename Exception>
		bool tryFailure(Exception e)
		{
			return tryFailure(std::make_exception_ptr(std::move(e)));
		}

		void tryCompleteWith(Future<T> &&f)
		{
			auto ctx = std::make_shared<Future<T>>(std::move(f));

			ctx->onComplete([this, ctx] (Try<T> t)
				{
					this->tryComplete(std::move(t));
				}
			);
		}

		void trySuccessWith(Future<T> &&f)
		{
			auto ctx = std::make_shared<Future<T>>(std::move(f));

			ctx->onComplete([this, ctx] (Try<T> t)
				{
					if (t.hasValue())
					{
						this->tryComplete(std::move(t));
					}
				}
			);
		}

		void tryFailureWith(Future<T> &&f)
		{
			auto ctx = std::make_shared<Future<T>>(std::move(f));

			ctx->onComplete([this, ctx] (Try<T> t)
				{
					if (t.hasException())
					{
						this->tryComplete(std::move(t));
					}
				}
			);
		}

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

	private:
		folly::Promise<T> _p;
};

}

#endif