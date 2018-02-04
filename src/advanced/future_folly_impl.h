#ifndef ADV_FUTUREFOLLYIMPL_H
#define ADV_FUTUREFOLLYIMPL_H

namespace adv
{

template<typename T>
Future<T> Future<T>::first(Future<T> &&other)
{
	struct SharedContext
	{
		SharedContext(Future<T> &&f0, Future<T> &&f1) : f0(std::move(f0)), f1(std::move(f1))
		{ }

		Promise<T> p;
		Future<T> f0;
		Future<T> f1;
	};

	auto ctx = std::make_shared<SharedContext>(std::move(*this), std::move(other));
	auto future = ctx->p.future();

	ctx->f0.onComplete([ctx] (Try<T> t) {
		ctx->p.tryComplete(std::move(t));
	});

	ctx->f1.onComplete([ctx] (Try<T> t) {
		ctx->p.tryComplete(std::move(t));
	});

	return future;
}

template<typename T>
Future<T> Future<T>::firstSucc(Future<T> &&other)
{
	struct SharedContext
	{
		SharedContext(Future<T> &&f0, Future<T> &&f1) : f0(std::move(f0)), f1(std::move(f1))
		{ }

		Promise<T> p;
		Future<T> f0;
		Future<T> f1;
	};

	auto ctx = std::make_shared<SharedContext>(std::move(*this), std::move(other));
	auto future = ctx->p.future();

	ctx->f0.onComplete([ctx] (Try<T> t) {
		ctx->p.trySuccess(t.get());
	});

	ctx->f1.onComplete([ctx] (Try<T> t) {
		ctx->p.trySuccess(t.get());
	});

	return future;
}

template<typename T>
SharedFuture<T> Future<T>::share()
{
	return SharedFuture<T>(std::move(*this));
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
		std::atomic<std::size_t> completed = {0};
		std::atomic<bool> done = {false};
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
				if (!ctx->done)
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

						if (c == n)
						{
							ctx->p.trySuccess(std::move(ctx->v));
							ctx->done = true;
						}
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
		std::atomic<std::size_t> succeeded = {0};
		std::atomic<std::size_t> failed = {0};
		std::atomic<bool> done = {false};
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
				if (!ctx->done)
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
								t.get();
							}
							catch (...)
							{
								ctx->p.tryFailure(std::current_exception());
							}

							ctx->done = true;
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
							ctx->v.emplace_back(i, std::move(t.get()));

							if (c == n)
							{
								ctx->p.trySuccess(std::move(ctx->v));
								ctx->done = true;
							}
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