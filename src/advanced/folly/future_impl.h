#ifndef ADV_FOLLY_FUTURE_IMPL_H
#define ADV_FOLLY_FUTURE_IMPL_H

#include <type_traits>

namespace adv_folly
{

template<typename T>
template<typename Func>
Future<typename std::result_of<Func(Try<T>)>::type> Future<T>::then(Func &&f)
{
    using S = typename std::result_of<Func(Try<T>)>::type;

    auto p = std::make_shared<Promise<S>>();

    this->onComplete([f = std::move(f), p] (Try<T> t) mutable
        {
        try {
            S result = S(f(std::move(t)));
            p->trySuccess(std::move(result));
        } catch (...) {
            p->tryFailure(std::current_exception());
        }
        }
    );

    return p->future();
}

template<typename T>
struct is_adv_future : std::false_type
{
};

template<typename T>
struct is_adv_future<Future<T>> : std::true_type
{
};

template<typename T>
template<typename Func>
typename std::result_of<Func(Try<T>)>::type Future<T>::thenWith(Func &&f)
{
	using FutureS = typename std::result_of<Func(Try<T>)>::type;
	static_assert(is_adv_future<FutureS>::value, "Return type must be of Future<S>");
	using S = typename FutureS::type;

	auto p = Promise<S>();
	auto r = p.future();

	this->onComplete([f = std::move(f), p = std::move(p)] (Try<T> &&t) mutable
        {
	        // The future will stay alive until it is completed.
            std::move(p).tryCompleteWithSafe(f(std::move(t)));
		}
	);

	return r;
}

template<typename T>
SharedFuture<T> Future<T>::share()
{
	return SharedFuture<T>(std::move(*this));
}

template<typename T>
Future<T> Future<T>::orElse(Future<T> &&other)
{
    return this->thenWith([other = std::move(other)] (Try<T> &&t) mutable {
        if (t.hasException()) {
            return other.then([t = std::move(t)] (Try<T> &&tt) mutable {
                                    if (tt.hasException()) {
                                        return std::move(t).get();
                                    }
                                    else {
                                        return std::move(tt).get();
                                    }
                                }
            );
        }
        else {
            // Don't ever move first since it would register a second callback which is not allowed.
            Promise<T> p;
            p.tryComplete(std::move(t));
            return p.future();
        }
    });
}

template<typename T>
Future<T> Future<T>::first(Future<T> &other)
{
    auto p = std::make_shared<Promise<T>>();

	this->onComplete([p] (Try<T> t) {
        p->tryComplete(std::move(t));
	});

   other.onComplete([p] (Try<T> t) {
       p->tryComplete(std::move(t));
	});

	return p->future();
}

template<typename T>
Future<T> Future<T>::firstSucc(Future<T> &other)
{
    auto p = std::make_shared<Promise<T>>();

	this->onComplete([p] (Try<T> t) {
		p->trySuccess(std::move(t).get());
	});

	other.onComplete([p] (Try<T> t) {
        p->trySuccess(std::move(t).get());
	});

    return p->future();
}

template<typename T>
Future<std::vector<std::pair<std::size_t, Try<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n)
{
	typedef std::vector<std::pair<size_t, Try<T>>> V;

	struct FirstNContext
	{
		/*
		 * Reserve enough space for the vector, so emplace_back won't modify the whole vector and stays thread-safe.
		 * Folly doesn't do this for folly::collectN which should lead to data races when the vector has a capacity smaller than n.
		 * See the following link (section Data races): http://www.cplusplus.com/reference/vector/vector/emplace_back/
		 */
		FirstNContext(std::size_t n)
		{
			v.reserve(n);
		}

		V v;
		std::atomic<std::size_t> vectorSize = {0};
		std::atomic<std::size_t> completed = {0};
		Promise<V> p;
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
			it->onComplete([ctx, n, total, i] (Try<T> t)
			{
				auto c = ++ctx->completed;

				if (c <= n)
				{
					/*
					 * This is only thread-safe if it does not reallocate the whole vector.
					 * Since we allocated enough space, it should never happen and therefore we don't need a mutex
					 * to protect it from data races.
					 */
					ctx->v.emplace_back(i, std::move(t));
					/**
					 * Compare to the actual size of the vector, after adding the element, to prevent possible data races which would occur if we had used c instead.
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

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n)
{
	typedef std::vector<std::pair<size_t, T>> V;

	struct FirstNSuccContext
	{
		/*
		 * Reserve enough space for the vector, so emplace_back won't modify the whole vector and stays thread-safe.
		 * Folly doesn't do this for folly::collectN which should lead to data races when the vector has a capacity smaller than n.
		 * See the following link (section Data races): http://www.cplusplus.com/reference/vector/vector/emplace_back/
		 */
		FirstNSuccContext(std::size_t n)
		{
			v.reserve(n);
		}

		V v;
		std::atomic<std::size_t> vectorSize = {0};
		std::atomic<std::size_t> succeeded = {0};
		std::atomic<std::size_t> failed = {0};
		Promise<V> p;
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
			it->onComplete([ctx, n, total, i] (Try<T> t)
			{
				// ignore exceptions until as many futures failed that n futures cannot be completed successfully anymore
				if (t.hasException())
				{
					auto c = ++ctx->failed;

					/*
					 * Since the local variable can never have the counter incremented by more than one,
					 * we can check for the exact final value and do only one setException call.
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
						 * Since we allocated enough space, it should never happen and therefore we don't need a mutex
						 * to protect it from data races.
						 */
						ctx->v.emplace_back(i, std::move(t).get());
						/**
						 * Compare to the actual size of the vector, after adding the element, to prevent possible data races which would occur if we had used c instead.
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
