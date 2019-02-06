#ifndef ADV_FUTURE_IMPL_H
#define ADV_FUTURE_IMPL_H

#include <type_traits>

#include "future.h"
#include "promise.h"

namespace adv
{

template <typename T>
template <typename Func>
void Future<T>::onSuccess(Func &&f)
{
	this->onComplete([f = std::move(f)](const Try<T> &t) mutable {
		if (t.hasValue())
		{
			f(t.get());
		}
	});
}

template <typename T>
template <typename Func>
void Future<T>::onFailure(Func &&f)
{
	this->onComplete([f = std::move(f)](const Try<T> &t) mutable {
		if (t.hasException())
		{
			try
			{
				t.get();
			}
			catch (...)
			{
				f(std::current_exception());
			}
		}
	});
}

template <typename T>
template <typename Func>
Future<typename std::result_of<Func(const Try<T> &)>::type>
Future<T>::then(Func &&f)
{
	using S = typename std::result_of<Func(const Try<T> &)>::type;

	auto p = createPromise<S>();

	this->onComplete([f = std::move(f), p](const Try<T> &t) mutable {
		try
		{
			p.trySuccess(S(f(t)));
		}
		catch (...)
		{
			p.tryFailure(std::current_exception());
		}
	}); /* TODO after onComplete p is destructed twice, the second time its shared
	     * pointer is already null which leads to a memory access violation at
	     * address: 0x00000010: no mapping at fault address Why is it deleted
	     * twice?!
	     */

	auto r = p.future();
	return r;
}

template <typename T>
template <typename Func>
typename std::result_of<Func(const Try<T> &)>::type
Future<T>::thenWith(Func &&f)
{
	using FutureS = typename std::result_of<Func(const Try<T> &)>::type;
	using S = typename FutureS::Type;

	auto p = createPromise<S>();

	this->onComplete([f = std::move(f), p](const Try<T> &t) mutable {
		// The future will stay alive until it is completed.
		Future<S> future = f(t);
		future.onComplete([future](const Try<S> &) {});
		p.tryCompleteWith(future);
	});

	return p.future();
}

template <typename T>
Future<T> Future<T>::orElse(Future<T> other)
{
	return this->thenWith([other](const Try<T> &t) mutable -> Future<T> {
		if (t.hasException())
		{
			return other.template then([&t](const Try<T> &tt) mutable {
				if (tt.hasException())
				{
					return t.get();
				}
				else
				{
					return tt.get();
				}
			});
		}
		else
		{
			auto p = other.template createPromise<T>();
			p.tryComplete(Try<T>(t));
			return p.future();
		}
	});
}

template <typename T>
Future<T> Future<T>::first(Future<T> other)
{
	auto p = createPromise<T>();
	this->onComplete([p](const Try<T> &t) mutable { p.tryComplete(Try<T>(t)); });
	other.onComplete([p](const Try<T> &t) mutable { p.tryComplete(Try<T>(t)); });

	return p.future();
}

template <typename T>
Future<T> Future<T>::firstSucc(Future<T> other)
{
	auto p = createPromise<T>();
	struct Context
	{
		explicit Context(Promise<T> &&p) : p(p)
		{
		}

		Promise<T> p;
		std::atomic<int> failCounter{0};
	};
	auto ctx = std::make_shared<Context>(std::move(p));
	this->onSuccess([ctx](const T &v) { ctx->p.trySuccess(T(v)); });
	other.onSuccess([ctx](const T &v) { ctx->p.trySuccess(T(v)); });

	this->onFailure([ctx](const std::exception_ptr &e) {
		auto c = ++ctx->failCounter;

		if (c == 2)
		{
			ctx->p.tryFailure(e);
		}
	});
	other.onFailure([ctx](const std::exception_ptr &e) {
		auto c = ++ctx->failCounter;

		if (c == 2)
		{
			ctx->p.tryFailure(e);
		}
	});

	return ctx->p.future();
}

template <typename Func>
Future<typename std::result_of<Func()>::type> async(folly::Executor *ex,
                                                    Func &&f)
{
	using T = typename std::result_of<Func()>::type;
	adv::Promise<T> p(ex);

	ex->add([f = std::move(f), p]() mutable {
		try
		{
			p.trySuccess(f());
		}
		catch (...)
		{
			p.tryFailure(std::current_exception());
		}
	});

	return p.future();
}

template <typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>>
firstN(folly::Executor *ex, std::vector<Future<T>> &&futures, std::size_t n)
{
	using V = std::vector<std::pair<size_t, Try<T>>>;

	struct FirstNContext
	{
		FirstNContext(folly::Executor *ex, std::size_t n) : p(ex)
		{
			/*
			 * Reserve enough space for the vector, so emplace_back won't modify the
			 * whole vector and stays thread-safe.
			 */
			v.reserve(n);
		}

		V v;
		std::atomic<std::size_t> vectorSize = {0};
		std::atomic<std::size_t> completed = {0};
		Promise<V> p;
	};

	auto ctx = std::make_shared<FirstNContext>(ex, n);
	const std::size_t total = futures.size();

	if (total < n)
	{
		ctx->p.tryFailure(std::runtime_error("Not enough futures"));
	}
	else
	{
		std::size_t i = 0;

		for (auto it = futures.begin(); it != futures.end(); ++it, ++i)
		{
			it->onComplete([ctx, n, total, i](const Try<T> &t) {
				auto c = ++ctx->completed;

				if (c <= n)
				{
					/*
					 * This is only thread-safe if it does not reallocate the whole vector.
					 * Since we allocated enough space, it should never happen and therefore we
					 * don't need a mutex to protect it from data races.
					 */
					ctx->v.emplace_back(i, std::move(t));
					/**
					 * Compare to the actual size of the vector, after adding the element, to
					 * prevent possible data races which would occur if we had used c instead.
					 */
					auto s = ++ctx->vectorSize;

					if (s == n)
					{
						ctx->p.trySuccess(std::move(ctx->v));
					}
				}
			});
		}
	}

	return ctx->p.future();
}

template <typename T>
Future<std::vector<std::pair<std::size_t, T>>>
firstNSucc(folly::Executor *ex, std::vector<Future<T>> &&futures, std::size_t n)
{
	using V = std::vector<std::pair<size_t, T>>;

	struct FirstNSuccContext
	{
		FirstNSuccContext(folly::Executor *ex, std::size_t n) : p(ex)
		{
			/*
			 * Reserve enough space for the vector, so emplace_back won't modify the
			 * whole vector and stays thread-safe.
			 */
			v.reserve(n);
		}

		V v;
		std::atomic<std::size_t> vectorSize = {0};
		std::atomic<std::size_t> succeeded = {0};
		std::atomic<std::size_t> failed = {0};
		Promise<V> p;
	};

	auto ctx = std::make_shared<FirstNSuccContext>(ex, n);
	const std::size_t total = futures.size();

	if (total < n)
	{
		ctx->p.tryFailure(std::runtime_error("Not enough futures"));
	}
	else
	{
		std::size_t i = 0;

		for (auto it = futures.begin(); it != futures.end(); ++it, ++i)
		{
			it->onComplete([ctx, n, total, i](const Try<T> &t) {
				// ignore exceptions until as many futures failed that n futures cannot be
				// completed successfully anymore
				if (t.hasException())
				{
					auto c = ++ctx->failed;

					/*
					 * Since the local variable can never have the counter incremented by more
					 * than one, we can check for the exact final value and do only one
					 * setException call.
					 */
					if (total - c + 1 == n)
					{
						try
						{
							std::move(t).get();
						}
						catch (...)
						{
							ctx->p.tryFailure(std::current_exception());
						}
					}
				}
				else
				{
					auto c = ++ctx->succeeded;

					if (c <= n)
					{
						/*
						 * This is only thread-safe if it does not reallocate the whole vector.
						 * Since we allocated enough space, it should never happen and therefore
						 * we don't need a mutex to protect it from data races.
						 */
						ctx->v.emplace_back(i, std::move(t).get());
						/**
						 * Compare to the actual size of the vector, after adding the element, to
						 * prevent possible data races which would occur if we had used c instead.
						 */
						auto s = ++ctx->vectorSize;

						if (s == n)
						{
							ctx->p.trySuccess(std::move(ctx->v));
						}
					}
				}
			});
		}
	}

	return ctx->p.future();
}
} // namespace adv

#endif
