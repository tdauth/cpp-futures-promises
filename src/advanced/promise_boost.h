#ifndef ADV_PROMISEBOOST_H
#define ADV_PROMISEBOOST_H

#include "future_boost.h"

namespace adv_boost
{

template<typename T>
class Promise
{
	public:
		Future<T> future()
		{
			return _p.get_future();
		}

		bool tryComplete(Try<T> &&v)
		{
			if (!v.hasValue() && !v.hasException())
			{
				return false;
			}

			// TODO add a tryComplete for boost::promise to the file "extensions.h" and use it here? Otherwise, we won't show that only this extension is required.
			try
			{
				this->_p.set_value(std::move(v.get()));
			}
			catch (const boost::promise_already_satisfied &e)
			{
				return false;
			}
			catch (...)
			{
				this->_p.set_exception(std::current_exception());
			}

			return true;
		}

		bool trySuccess(T &&v)
		{
			return tryComplete(Try<T>(std::move(v)));
		}

		bool tryFailure(std::exception_ptr &&e)
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

		Promise(boost::promise<T> &&p) : _p(std::move(p))
		{
		}

	private:
		boost::promise<T> _p;
};

}

#endif
