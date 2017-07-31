#ifndef FUTUREFOLLY_H
#define FUTUREFOLLY_H

#include <type_traits>

#include <folly/futures/Future.h>

#include "extensions.h"

namespace xtn
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

		Try(std::exception_ptr &&e) : _t(std::move(e))
		{
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

/**
 * Match our defined extended future.
 * Only use custom extensions of Folly if really necessary!
 */
template<typename T>
class Future
{
	public:
		typedef Future<T> self;
		typedef Try<T> try_type;

		T get()
		{
			return std::move(_f.get());
		}

		template<typename Func>
		void onComplete(Func &&f)
		{
			_f.setCallback_([f = std::move(f)] (folly::Try<T> t) mutable
				{
					try_type myTry(std::move(t));
					f(std::move(myTry));
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
			folly::Future<T> r = _f.filter([f = std::move(f)] (T v)
				{
					return f(v);
				}
			);

			return self(std::move(r));
		}

		template<typename Func>
		Future<typename std::result_of<Func(Try<T>)>::type> then(Func &&f)
		{
			using S = typename std::result_of<Func(Try<T>)>::type;

			folly::Future<S> r = _f.then([f = std::move(f)] (folly::Try<T> t) mutable
				{
					return S(f(try_type(std::move(t))));
				}
			);

			return Future<S>(std::move(r));
		}

		Future<T> orElse(Future<T> &&other)
		{
			return self(::orElse(std::move(this->_f), std::move(other._f)));
		}

		Future<T> first(Future<T> &&other)
		{
			std::vector<folly::Future<T>> futures;
			futures.push_back(std::move(this->_f));
			futures.push_back(std::move(other._f));

			return self(folly::collectAny(futures.begin(), futures.end())
				.then([] (std::pair<std::size_t, folly::Try<T>> t)
				{
					t.second.throwIfFailed();

					return t.second.value();
				}
			));
		}

		Future<T> firstSucc(Future<T> &&other)
		{
			std::vector<folly::Future<T>> futures;
			futures.push_back(std::move(this->_f));
			futures.push_back(std::move(other._f));

			return self(folly::collectAnyWithoutException(futures.begin(), futures.end())
				.then([] (std::pair<std::size_t, T> t)
				{
					return t.second;
				}
			));
		}

		Future()
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
	private:
		template<typename S>
		friend Future<std::vector<std::pair<std::size_t, Future<S>>>> firstN(std::vector<Future<S>> &&c, std::size_t n);

		template<typename S>
		friend Future<std::vector<std::pair<std::size_t, S>>> firstNSucc(std::vector<Future<S>> &&c, std::size_t n);

		template<typename S>
		friend class Promise;

		folly::Future<T> _f;
};

template<typename Func>
Future<typename std::result_of<Func()>::type> async(Executor *ex, Func &&f)
{
	using T = typename std::result_of<Func()>::type;

	return Future<T>(folly::via(ex->_e, std::move(f)));
}

template<typename T>
Future<std::vector<std::pair<std::size_t, Future<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n)
{
	std::vector<folly::Future<T>> futures;

	for (auto it = c.begin(); it != c.end(); ++it)
	{
		futures.push_back(std::move(it->_f));
	}

	using VectorType = std::vector<std::pair<std::size_t, folly::Try<T>>>;

	folly::Future<VectorType> results = folly::collectN(futures.begin(), futures.end(), n);

	using PairType = std::pair<std::size_t, Future<T>>;

	Future<std::vector<PairType>> r = results.then([] (VectorType v)
		{
			std::vector<PairType> r;

			for (auto it = v.begin(); it != v.end(); ++it)
			{
				r.push_back(std::make_pair(it->first, Future<T>(folly::Future<T>(it->second))));
			}

			return r;
		}
	);

	return r;
}

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n)
{
	std::vector<folly::Future<T>> futures;

	for (auto it = c.begin(); it != c.end(); ++it)
	{
		futures.push_back(std::move(it->_f));
	}

	using VectorType = std::vector<std::pair<std::size_t, T>>;

	folly::Future<VectorType> results = collectNWithoutException(futures.begin(), futures.end(), n);

	using PairType = std::pair<std::size_t, T>;

	Future<std::vector<PairType>> r = results.then([] (VectorType v)
		{
			std::vector<PairType> r;

			for (auto it = v.begin(); it != v.end(); ++it)
			{
				r.push_back(std::make_pair(it->first, std::move(it->second)));
			}

			return r;
		}
	);

	return r;
}

template<typename T>
class SharedFuture
{
};

}

#endif