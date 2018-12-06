#ifndef ADV_FOLLY_FUTURE_H
#define ADV_FOLLY_FUTURE_H

#include <type_traits>

#include <folly/futures/Future.h>

#include "../future.h"
#include "../../extensions.h"

namespace adv_folly
{

template<typename T>
class Try
{
	public:
		Try()
		{
		}

		Try(T &&v) : _t(std::move(v))
		{
		}

		Try(std::exception_ptr &&e)
		{
			/*
			 * Do this manually to avoid the deprecated warning.
			 */
			try
			{
				std::rethrow_exception(e);
			}
			catch (std::exception& e)
			{
				_t = folly::Try<T>(folly::exception_wrapper(std::current_exception(), e));
			}
			catch (...)
			{
				_t = folly::Try<T>(folly::exception_wrapper(std::current_exception()));
			}
		}

		// Only required for shared futures:
		Try(const Try<T> &other) : _t(other._t)
		{
		}

		Try(Try<T> &&other) : _t(std::move(other._t))
		{
		}

		Try(folly::Try<T> &&other) : _t(std::move(other))
		{
		}

		// Only required for multi-callback support:
		Try(const folly::Try<T> &other) : _t(other)
		{
		}

		// Only required for shared futures:
		Try<T>& operator=(const Try<T> &other)
		{
			_t = other._t;

			return *this;
		}

		T get()
		{
			if (!_t.hasValue() && !_t.hasException())
			{
				throw adv::UsingUninitializedTry();
			}

			_t.throwIfFailed();

			return _t.value();
		}

		// Only required for shared futures and multi-callback support:
		const T& get() const
		{
			if (!_t.hasValue() && !_t.hasException())
			{
				throw adv::UsingUninitializedTry();
			}

			_t.throwIfFailed();

			return _t.value();
		}

		bool hasValue()
		{
			return _t.hasValue();
		}

		bool hasException()
		{
			return _t.hasException();
		}

	private:
		template<typename S>
		friend class Promise;

		folly::Try<T> _t;
};

template<typename T>
class Future;

class Executor
{
	public:
		Executor(folly::Executor *ex) : _e(ex)
		{
		}

		template<typename Func>
		void submit(Func &&f)
		{
			this->_e->add(std::move(f));
		}

	private:
		template<typename Func>
		friend Future<typename std::result_of<Func()>::type> async(Executor *ex, Func &&f);

		folly::Executor *_e;
};

template<typename T>
class SharedFuture;

template<typename T>
class Future
{
	public:
		using type = T;

		// Core methods:
		Future() : _f(folly::Future<T>::makeEmpty())
		{
		}

		Future(Future<T> &&other) : _f(std::move(other._f))
		{
		}

		Future(const Future<T> &other) = delete;
		Future<T>& operator=(const Future<T> &other) = delete;

		Future(folly::Future<T> &&f) : _f(std::move(f))
		{
		}

		SharedFuture<T> share();

		T get()
		{
			try
			{
				return std::move(std::move(_f).get());
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
		 * @throws adv::OnlyOneCallbackPerFuture If there is already a callback registered for this future.
		 */
		template<typename Func>
		void onComplete(Func &&f)
		{
		    try {
                this->_f.setCallback_([f = std::move(f)](folly::Try<T> &&t) mutable {
                                          f(Try<T>(std::move(t)));
                                      }
                );
            } catch (const folly::FutureAlreadyContinued &) {
		        throw adv::OnlyOneCallbackPerFuture();
		    }
		}

		// Derived methods:
		template<typename Func>
        Future<typename std::result_of<Func(Try<T>)>::type> then(Func &&f);

		template<typename Func>
        typename std::result_of<Func(Try<T>)>::type thenWith(Func &&f);

		template<typename Func>
		Future<T> guard(Func &&f)
		{
			return std::move(*this).then([f = std::move(f)] (Try<T> &&t) mutable
			{
				auto x = std::move(t).get();

				if (!f(x))
				{
					throw adv::PredicateNotFulfilled();
				}

				return x;
			});
		}

		Future<T> orElse(Future<T> &&other);

		Future<T> first(Future<T> &other);

		Future<T> firstSucc(Future<T> &other);

	private:
		folly::Future<T> _f;
};

// TODO Derived from a constructor with an executor.
template<typename Func>
Future<typename std::result_of<Func()>::type> async(Executor *ex, Func &&f)
{
	using T = typename std::result_of<Func()>::type;

	return Future<T>(folly::via(ex->_e, std::move(f)));
}

template<typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n);

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n);

}

#endif
