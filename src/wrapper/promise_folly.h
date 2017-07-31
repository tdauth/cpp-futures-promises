#ifndef PROMISEFOLLY_H
#define PROMISEFOLLY_H

#include "future_folly.h"

namespace wish
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
			return ::tryCompleteSuccess(this->_p, std::move(v));
		}

		template<typename Exception>
		bool tryFailure(Exception &&e)
		{
			return ::tryCompleteFailure(this->_p, std::move(e));
		}

		void tryCompleteWith(Future<T> &&f)
		{
			::tryCompleteWith(this->_p, std::move(f._f));
		}

		void trySuccessWith(Future<T> &&f)
		{
			::tryCompleteSuccessWith(this->_p, std::move(f._f));
		}

		void tryFailureWith(Future<T> &&f)
		{
			::tryCompleteFailureWith(this->_p, std::move(f._f));
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