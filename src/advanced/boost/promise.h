#ifndef ADV_BOOST_PROMISE_H
#define ADV_BOOST_PROMISE_H

#include "future.h"
#include "../promise.h"

namespace adv_boost
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

		Promise(boost::promise<T> &&p) : _p(std::move(p))
		{
		}

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
				this->_p.set_value(std::move(v).get());
			}
			catch (const boost::promise_already_satisfied &e)
			{
				return false;
			}
			catch (...)
			{
				this->_p.set_exception(std::move(boost::current_exception()));
			}

			return true;
		}

		// Derived methods:
		bool trySuccess(T &&v)
		{
			return tryComplete(Try<T>(std::move(v)));
		}

		bool tryFailure(boost::exception_ptr &&e)
		{
			return tryComplete(Try<T>(std::move(e)));
		}

		template<typename Exception>
		bool tryFailure(Exception &&e)
		{
			return tryFailure(boost::copy_exception(std::move(e)));
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

	private:
		boost::promise<T> _p;
};

}

#endif
