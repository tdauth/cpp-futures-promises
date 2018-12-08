#ifndef ADV_FUTURE_IMPL_H
#define ADV_FUTURE_IMPL_H

#include <type_traits>

#include "executor.h"
#include "future.h"
#include "promise.h"

namespace adv
{

template <typename T, typename CoreType>
template <typename Func>
Future<typename std::result_of<Func(Try<T>)>::type,
       typename CoreType::template CoreType<
           typename std::result_of<Func(Try<T>)>::type>>
Future<T, CoreType>::then(Func &&f)
{
	using S = typename std::result_of<Func(Try<T>)>::type;
	using CoreTypeS = typename CoreType::template CoreType<S>;
	using PromiseS =
	    typename CoreTypeS::template PromiseType<typename CoreTypeS::Type>;

	auto p = std::make_shared<PromiseS>();

	this->onComplete([ f = std::move(f), p ](Try<T> t) mutable {
		try
		{
			S result = S(f(std::move(t)));
			p->trySuccess(std::move(result));
		}
		catch (...)
		{
			p->tryFailure(std::current_exception());
		}
	});

	return p->future();
}

template <typename FutureType>
struct is_adv_future : std::false_type
{
};
// TODO Define for Boost and Folly types.

template <typename T, typename CoreType>
template <typename Func>
typename std::result_of<Func(Try<T>)>::type
Future<T, CoreType>::thenWith(Func &&f)
{
	using FutureS = typename std::result_of<Func(Try<T>)>::type;
	// TODO Fix it!
	//	static_assert(is_adv_future<FutureS>::value,
	//              "Return type must be of Future<S>");
	using S = typename FutureS::Type;

	auto p = CoreType::template createPromise<S>();
	auto r = p.future();

	this->onComplete([ f = std::move(f), p = std::move(p) ](Try<T> && t) mutable {
		// The future will stay alive until it is completed.
		std::move(p).tryCompleteWithSafe(f(std::move(t)));
	});

	return std::move(r);
}

template <typename T, typename CoreType>
Future<T, CoreType> Future<T, CoreType>::orElse(Future<T, CoreType> &&other)
{
	// This requires that the concrete future type can be converted into
	// adv::Future<T, CoreType>.
	return this->thenWith([other = std::move(other)](
	                          Try<T> && t) mutable->Future<T, CoreType> {
		if (t.hasException())
		{
			return other.template then([t = std::move(t)](Try<T> && tt) mutable {
				if (tt.hasException())
				{
					return std::move(t).get();
				}
				else
				{
					return std::move(tt).get();
				}
			});
		}
		else
		{
			// Don't ever move first since it would register a second callback which
			// is not allowed.
			auto p = CoreType::template createPromise<T>();
			p.tryComplete(std::move(t));
			return p.future();
		}
	});
}

template <typename T, typename CoreType>
Future<T, CoreType> Future<T, CoreType>::first(Future<T, CoreType> &other)
{
	using PromiseType = typename CoreType::template PromiseType<T>;
	auto p = std::make_shared<PromiseType>();

	this->onComplete([p](Try<T> t) { p->tryComplete(std::move(t)); });

	other.onComplete([p](Try<T> t) { p->tryComplete(std::move(t)); });

	return p->future();
}

template <typename T, typename CoreType>
Future<T, CoreType> Future<T, CoreType>::firstSucc(Future<T, CoreType> &other)
{
	using PromiseType = typename CoreType::template PromiseType<T>;
	auto p = std::make_shared<PromiseType>();

	this->onComplete([p](Try<T> t) { p->trySuccess(std::move(t).get()); });

	other.onComplete([p](Try<T> t) { p->trySuccess(std::move(t).get()); });

	return p->future();
}

template <typename PromiseType, typename Func>
typename PromiseType::FutureType async(Executor *ex, Func &&f)
{
	/*
	 * TODO Moving the promise will complete the futures associated with a
	 * BrokenPromise exception in Folly! Therefore, we have to keep the promise
	 * alive in a shared pointer. This is kind of weird since why not keep the
	 * futures uncompleted and use the new promise?!
	 */
	auto p = std::make_shared<PromiseType>();

	ex->submit([ f = std::move(f), p ]() mutable {
		try
		{
			p->trySuccess(f());
		}
		catch (...)
		{
			p->tryFailure(std::current_exception());
		}
	});

	return p->future();
}

template <typename PromiseType, typename FutureType>
Future<std::vector<std::pair<std::size_t, Try<typename FutureType::Type>>>,
       typename FutureType::CoreType>
firstN(std::vector<FutureType> &&c, std::size_t n)
{
	using CoreType = typename FutureType::CoreType;
	using T = typename CoreType::Type;
	typedef std::vector<std::pair<size_t, Try<T>>> V;

	struct FirstNContext
	{
		/*
		 * Reserve enough space for the vector, so emplace_back won't modify the whole
		 * vector and stays thread-safe.
		 */
		FirstNContext(std::size_t n)
		{
			v.reserve(n);
		}

		V v;
		std::atomic<std::size_t> vectorSize = {0};
		std::atomic<std::size_t> completed = {0};
		PromiseType p;
	};

	auto ctx = std::make_shared<FirstNContext>(n);
	const std::size_t total = c.size();

	if (total < n)
	{
		ctx->p.tryFailure(std::runtime_error("Not enough futures"));
	}
	else
	{
		std::size_t i = 0;

		for (auto it = c.begin(); it != c.end(); ++it, ++i)
		{
			it->onComplete([ctx, n, total, i](Try<T> t) {
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

template <typename PromiseType, typename FutureType>
Future<std::vector<std::pair<std::size_t, typename FutureType::Type>>,
       typename FutureType::CoreType>
firstNSucc(std::vector<FutureType> &&c, std::size_t n)
{
	using T = typename FutureType::Type;
	typedef std::vector<std::pair<size_t, T>> V;

	struct FirstNSuccContext
	{
		/*
		 * Reserve enough space for the vector, so emplace_back won't modify the whole
		 * vector and stays thread-safe.
		 */
		FirstNSuccContext(std::size_t n)
		{
			v.reserve(n);
		}

		V v;
		std::atomic<std::size_t> vectorSize = {0};
		std::atomic<std::size_t> succeeded = {0};
		std::atomic<std::size_t> failed = {0};
		PromiseType p;
	};

	auto ctx = std::make_shared<FirstNSuccContext>(n);
	const std::size_t total = c.size();

	if (total < n)
	{
		ctx->p.tryFailure(std::runtime_error("Not enough futures"));
	}
	else
	{
		std::size_t i = 0;

		for (auto it = c.begin(); it != c.end(); ++it, ++i)
		{
			it->onComplete([ctx, n, total, i](Try<T> t) {
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
}

#endif
