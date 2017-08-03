#ifndef ADV_FUTUREFOLLY_H
#define ADV_FUTUREFOLLY_H

#include <type_traits>

#include <folly/futures/Future.h>

#include "extensions.h"

namespace adv
{

template<typename T>
class Try
{
	public:
		T get()
		{
			_t.throwIfFailed();

			return std::move(_t.value());
		}

		bool hasValue()
		{
			return _t.hasValue();
		}

		bool hasException()
		{
			return _t.hasException();
		}

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

		Try(folly::Try<T> &&other) : _t(std::move(other))
		{
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
		template<typename Func>
		void submit(Func &&f)
		{
			this->_e->add(std::move(f));
		}

		Executor(folly::Executor *ex) : _e(ex)
		{
		}
	private:
		template<typename Func>
		friend Future<typename std::result_of<Func()>::type> async(Executor *ex, Func &&f);

		folly::Executor *_e;
};

class PredicateNotFulfilled : public std::exception
{
};

template<typename T>
class SharedFuture;

template<typename T>
class Future
{
	public:
		T get()
		{
			return std::move(_f.get());
		}

		template<typename Func>
		void onComplete(Func &&f)
		{
			this->then([f = std::move(f)] (Try<T> t) mutable
				{
					f(t);

					return folly::unit; // workaround to not provide a future with void
				}
			);
		}

		bool isReady()
		{
			return _f.isReady();
		}

		template<typename Func>
		Future<T> guard(Func &&f)
		{
			return this->then([f = std::move(f)] (Try<T> t) mutable
			{
				auto x = t.get();

				if (!f(x))
				{
					throw PredicateNotFulfilled();
				}

				return x;
			});
		}

		template<typename Func>
		Future<typename std::result_of<Func(Try<T>)>::type> then(Func &&f)
		{
			using S = typename std::result_of<Func(Try<T>)>::type;

			folly::Future<S> r = _f.then([f = std::move(f)] (folly::Try<T> t) mutable
				{
					return S(f(std::move(t)));
				}
			);

			return Future<S>(std::move(r));
		}

		Future<T> orElse(Future<T> &&other)
		{
			return this->then([other = std::move(other)] (Try<T> t) mutable
				{
					if (t.hasException())
					{
						try
						{
							return other.get();
						}
						catch (...)
						{
						}
					}

					return t.get(); // will rethrow if failed
				}
			);
		}

		Future<T> first(Future<T> &&other);

		Future<T> firstSucc(Future<T> &&other);

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

	private:
		folly::Future<T> _f;
};

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