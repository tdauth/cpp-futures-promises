#ifndef ADV_BOOST_FUTURE_H
#define ADV_BOOST_FUTURE_H

#include <type_traits>

#include <boost/thread.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/exception/all.hpp>

#include "../future.h"
#include "../../extensions.h"

namespace adv_boost
{

template<typename T>
class Try
{
	public:
		Try()
		{
		}

		Try(T &&v) : _v(std::move(v))
		{
		}

		Try(boost::exception_ptr &&e) : _v(std::move(e))
		{
		}

		Try(const Try<T> &other) : _v(other._v)
		{
		}

		Try(Try<T> &&other) : _v(std::move(other._v))
		{
		}

		Try<T>& operator=(const Try<T> &other)
		{
			_v = other._v;

			return *this;
		}

		T get()
		{
			if (_v == boost::none)
			{
				throw adv::UsingUninitializedTry();
			}

			if (_v.value().which() != 0)
			{
				boost::rethrow_exception(std::move(boost::get<boost::exception_ptr>(std::move(_v.get()))));
			}

			return std::move(boost::get<T>(std::move(_v.value())));
		}

		const T& get() const
		{
			if (_v == boost::none)
			{
				throw adv::UsingUninitializedTry();
			}

			if (_v.value().which() != 0)
			{
				boost::rethrow_exception(boost::get<boost::exception_ptr>(_v.get()));
			}

			return boost::get<T>(_v.value());
		}

		bool hasValue()
		{
			if (_v  != boost::none)
			{
				return _v.value().which() == 0;
			}

			return false;
		}

		bool hasException()
		{
			if (_v != boost::none)
			{
				return _v.value().which() == 1;
			}

			return false;
		}

	private:
		template<typename S>
		friend class Promise;

		boost::optional<boost::variant<T, boost::exception_ptr>> _v;
};

template<typename T>
class Future;

// TODO No template argument please, only in the constructor if possible? Otherwise, it does not fulfill the defined interface.
template<typename Ex>
class Executor
{
	public:
		Executor(Ex *ex) : _e(ex)
		{
		}

		template<typename Func>
		void submit(Func &&f)
		{
			this->_e->submit(std::move(f));
		}

	public:
		// TODO make private and fix friend
		template<typename Func>
		friend Future<typename std::result_of<Func()>::type> async(Executor<Ex> *ex, Func &&f);

		Ex *_e;
};

template<typename T>
class SharedFuture;

template<typename T>
class Future
{
	public:
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

		T get()
		{
			try
			{
				return std::move(_f.get());
			}
			catch (const boost::broken_promise &e)
			{
				throw adv::BrokenPromise();
			}
		}

		bool isReady()
		{
			return _f.is_ready();
		}

		template<typename Func>
		Future<typename std::result_of<Func(Try<T>)>::type> then(Func &&f)
		{
			using S = typename std::result_of<Func(Try<T>)>::type;

			boost::future<S> r = _f.then([f = std::move(f)] (boost::future<T> future) mutable
				{
					try
					{
						T value = std::move(future.get());

						return S(f(Try<T>(std::move(value))));
					}
					catch (...)
					{
						return S(f(Try<T>(std::move(boost::current_exception()))));
					}
				}
			);

			return Future<S>(std::move(r));
		}

		template<typename Func>
		void onComplete(Func &&f)
		{
			this->then([f = std::move(f)] (Try<T> t) mutable
				{
					f(std::move(t));
				}
			);
		}

		template<typename Func>
		Future<T> guard(Func &&f)
		{
			return this->then([f = std::move(f)] (Try<T> t) mutable
			{
				auto x = std::move(t.get());

				if (!f(x))
				{
					throw adv::PredicateNotFulfilled();
				}

				return x;
			});
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

	private:
		boost::future<T> _f;
};

template<typename Ex, typename Func>
Future<typename std::result_of<Func()>::type> async(Executor<Ex> *ex, Func &&f)
{
	using T = typename std::result_of<Func()>::type;

	return Future<T>(boost::async(*ex->_e, std::move(f)));
}

template<typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n);

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n);

}

#endif
