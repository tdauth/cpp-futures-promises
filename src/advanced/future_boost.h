#ifndef ADV_FUTUREBOOST_H
#define ADV_FUTUREBOOST_H

#include <type_traits>
#include <variant>
#include <optional>
#include <exception>

#include <boost/thread.hpp>

#include "extensions.h"

namespace adv_boost
{

template<typename T>
class Try
{
	public:
		T get()
		{
			if (!_v.has_value())
			{
				// TODO replace type, Folly uses throwIfFailed().
				throw;
			}

			if (std::holds_alternative<std::exception_ptr>(_v.value()))
			{
				std::rethrow_exception(std::get<std::exception_ptr>(_v.value()));
			}

			return std::move(std::get<T>(_v.value()));
		}

		bool hasValue()
		{
			if (_v.has_value())
			{
				return std::holds_alternative<T>(_v.value());
			}

			return false;
		}

		bool hasException()
		{
			if (_v.has_value())
			{
				return std::holds_alternative<std::exception_ptr>(_v.value());
			}

			return false;
		}

		Try()
		{
		}

		Try(T &&v) : _v(std::move(v))
		{
		}

		Try(std::exception_ptr &&e) : _v(std::move(e))
		{
		}

	private:
		template<typename S>
		friend class Promise;

		std::optional<std::variant<T, std::exception_ptr>> _v;
};

template<typename T>
class Future;

template<typename Ex>
class Executor
{
	public:
		template<typename Func>
		void submit(Func &&f)
		{
			this->_e->submit(std::move(f));
		}

		Executor(Ex *ex) : _e(ex)
		{
		}
	private:
		template<typename Func>
		friend Future<typename std::result_of<Func()>::type> async(Executor *ex, Func &&f);

		Ex *_e;
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
			return this->then([f = std::move(f)] (boost::future<T> future) mutable
			{
				auto x = future.get();

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

			boost::future<S> r = _f.then([f = std::move(f)] (boost::future<T> future) mutable
				{
					try
					{
						T value = future.get();

						return S(f(Try<T>(std::move(value))));
					}
					catch (...)
					{
						return S(f(Try<T>(std::move(std::current_exception()))));
					}
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

		Future()
		{
		}

		Future(Future<T> &&other) : _f(std::move(other._f))
		{
		}

		Future(const Future<T> &other) = delete;
		Future<T>& operator=(const Future<T> &other) = delete;

		Future(boost::future<T> &&f) : _f(std::move(f))
		{
		}

		SharedFuture<T> share();

	private:
		boost::future<T> _f;
};

template<typename Ex, typename Func>
Future<typename std::result_of<Func()>::type> async(Executor<Ex> *ex, Func &&f)
{
	using T = typename std::result_of<Func()>::type;

	return Future<T>(boost::async(ex->_e, std::move(f)));
}

template<typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n);

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n);

}

#endif
