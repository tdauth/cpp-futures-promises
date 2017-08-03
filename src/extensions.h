#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <algorithm>
#include <iterator>

#include <boost/thread.hpp>

#include <folly/futures/Future.h>
#include <folly/futures/SharedPromise.h>

#include "random.h"

/**
 * Similar to folly::collectAnyWithoutException but the version of folly::collectN.
 * Collects \p n futures which completed without exceptions.
 * If too many futures failed, it fails with the last thrown exception.
 *
 */
template <class InputIterator>
folly::Future<std::vector<std::pair<size_t, typename std::iterator_traits<InputIterator>::value_type::value_type>>>
collectNWithoutException(InputIterator first, InputIterator last, size_t n)
{
	typedef typename std::iterator_traits<InputIterator>::value_type::value_type T;
	typedef std::vector<std::pair<size_t, T>> V;

	struct CollectNWithoutExceptionContext
	{
		/*
		 * Reserve enough space for the vector, so emplace_back won't modify the whole vector and stays thread-safe.
		 * Folly doesn't do this for folly::collectN which should lead to data races when the vector has a capacity smaller than n.
		 * See the following link (section Data races): http://www.cplusplus.com/reference/vector/vector/emplace_back/
		 */
		CollectNWithoutExceptionContext(size_t n)
		{
			v.reserve(n);
		}

		V v;
		std::atomic<size_t> succeeded = {0};
		std::atomic<size_t> failed = {0};
		folly::Promise<V> p;
	};

	auto ctx = std::make_shared<CollectNWithoutExceptionContext>(n);
	const size_t total = std::distance(first, last);

	if (total < n)
	{
		ctx->p.setException(std::runtime_error("Not enough futures"));
	}
	else
	{
		folly::mapSetCallback<T>(first, last, [ctx, n, total] (size_t i, folly::Try<T>&& t)
			{
				if (!ctx->p.isFulfilled())
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
							ctx->p.setException(t.exception());
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
							ctx->v.emplace_back(i, std::move(t.value()));

							if (c == n)
							{
								ctx->p.setValue(std::move(ctx->v));
							}
						}
					}
				}
			}
		);
	}

	return ctx->p.getFuture();
}

template <class Collection>
folly::Future<std::vector<std::pair<size_t, typename Collection::value_type::value_type>>>
collectNWithoutException(Collection &&c, size_t n)
{
	return collectNWithoutException(c.begin(), c.end(), n);
}

template<typename T>
folly::Future<T> orElse(folly::Future<T> &&first, folly::Future<T> &&second)
{
	return first.then([second = std::move(second)] (folly::Try<T> t) mutable
		{
			if (t.hasException())
			{
				second.wait();

				if (second.hasValue())
				{
					return second.get();
				}

				t.exception().throw_exception();
			}

			return t.value();
		}
	);
}

/**
 * Same as orElse with two futures for Folly but for Boost.Thread.
 */
template<typename T>
boost::future<T> orElse(boost::future<T> &&first, boost::future<T> &&second)
{
	return first.then([second = std::move(second)] (boost::future<T> f) mutable
		{
			if (f.has_exception())
			{
				second.wait();

				if (second.has_value())
				{
					return second.get();
				}
			}

			return f.get();
		}
	);
}

/**
 * Similar to folly::mapSetCallback() and useful for combinators on collections.
 */
template <class T, class InputIterator, class F>
void mapSetCallbackBoost(InputIterator first, InputIterator last, F func)
{
	for (size_t i = 0; first != last; ++first, ++i)
	{
		first->then([func, i] (boost::future<T> f)
			{
				func(i, std::move(f));
			}
		);
	}
}

/**
 * Similar to Folly's version of folly::collectN() with iterators.
 * Actually, the source code is almost the same as in Folly's "Future-inl.h" file
 * but with the interface of Boost.Thread.
 * It does also return the indices.
 *
 * \param first The iterator which points to the first element of the collection.
 * \param last The element which points to the last element of the collection.
 * \return Returns a boost::future<std::vector<std::pair<std::size_t, boost::future<T>>>>. The pair holds the original index of the future and the future itself.
 *
 * \note Although the iteration and registration of callbacks has a fixed order, the order of completion is not guaranteed with already completed futures since the atomic operations are not protected by one global lock.
 */
template<typename InputIterator>
boost::future<std::vector<std::pair<std::size_t, typename InputIterator::value_type>>>
whenN(InputIterator first, InputIterator last, std::size_t n)
{
	typedef typename InputIterator::value_type future_type;
	typedef typename future_type::value_type future_value_type;
	typedef typename std::pair<std::size_t, future_type> value_type;
	typedef std::vector<value_type> result_type;
	const std::size_t size = std::distance(first, last);

	if (size < n)
	{
		return boost::make_exceptional_future<result_type>(
			std::runtime_error("Not enough futures")
		);
	}

	struct WhenNContext
	{
		std::mutex m;
		result_type v;
		size_t completed = {0};
		boost::promise<result_type> p;
	};

	auto ctx = std::make_shared<WhenNContext>();

	mapSetCallbackBoost<future_value_type>(first, last, [ctx, n] (std::size_t i, future_type f)
		{
			std::unique_lock<std::mutex> l(ctx->m);

			auto c = ++ctx->completed;

			if (c <= n)
			{
				assert(ctx->v.size() < n);
				ctx->v.emplace_back(i, std::move(f));

				if (c == n)
				{
					ctx->p.set_value(std::move(ctx->v));
				}
			}
		}
	);

	return ctx->p.get_future();
}

/**
 * Like \ref collectNWithoutException but for Boost.Thread.
 *
 * \param first The iterator which points at the first element of the collection.
 * \param last The iterator which points at the last element of the collection.
 * \param n The number of successful futures which are collected from the collection.
 * \return Returns a future of the type boost::future<std::vector<std::pair<std::size_t, T>>> which contains all result values and indices of the n successfully completed futures or is completed with the last exception of an input future if too many failed.
 */
template<typename InputIterator>
boost::future<std::vector<std::pair<std::size_t, typename InputIterator::value_type::value_type>>>
whenNSucc(InputIterator first, InputIterator last, std::size_t n)
{
	typedef typename InputIterator::value_type future_type;
	typedef typename future_type::value_type future_value_type;
	typedef typename std::pair<std::size_t, future_value_type> value_type;
	typedef std::vector<value_type> result_type;

	struct WhenNSuccContext
	{
		std::mutex m;
		result_type v;
		size_t completed = {0};
		size_t failed = {0};
		boost::promise<result_type> p;
		bool done = {false};
	};

	auto ctx = std::make_shared<WhenNSuccContext>();
	const size_t total = std::distance(first, last);

	if (total < n)
	{
		ctx->p.set_exception(std::runtime_error("Not enough futures"));
	}
	else
	{
		mapSetCallbackBoost<future_value_type>(first, last, [ctx, n, total] (std::size_t i, future_type f)
			{
				std::unique_lock<std::mutex> l(ctx->m);

				// ignore exceptions until as many futures failed that n futures cannot be completed successfully anymore
				if (f.has_exception())
				{
					if (!ctx->done)
					{
						auto c = ++ctx->failed;

						if (total - c < n)
						{
							ctx->done = true;
							ctx->p.set_exception(f.get_exception_ptr());
						}
					}
				}
				else if (!ctx->done)
				{
					auto c = ++ctx->completed;

					if (c <= n)
					{
						assert(ctx->v.size() < n);
						ctx->v.emplace_back(i, std::move(f.get()));

						if (c == n)
						{
							ctx->done = true;
							ctx->p.set_value(std::move(ctx->v));
						}
					}
				}
			}
		);
	}

	return ctx->p.get_future();
}

/**
 * Since boost::when_any() does return the whole collection in a future, it is not possible to determine which
 * future has been completed to complete the resulting future of boost::when_any().
 * This version returns a future with a pair like folly::collectAny().
 *
 * \return Returns a future with a pair holding the index of the completed future and the completed future.
 */
template<typename InputIterator>
boost::future<std::pair<std::size_t, typename InputIterator::value_type>>
whenAny(InputIterator first, InputIterator last)
{
	using future_type = boost::future<std::vector<std::pair<std::size_t, typename InputIterator::value_type>>>;

	return whenN(first, last, 1).then([] (future_type future)
		{
			return std::move(future.get()[0]);
		}
	);
}

/**
 * Extensions for promises based on the Scala library for futures and promises.
 * These combinators allow to complete a promise without a failure if it has already been completed.
 *
 * \note Make sure that the passed promises and futures are not deleted in the meantime since they are only passed via reference.
 *
 * \{
 */
template<typename T>
bool tryComplete(folly::Promise<T> &p, folly::Try<T> &&t)
{
	try
	{
		p.setTry(std::move(t));
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
		return false;
	}

	return true;
}

template<typename T>
bool tryComplete(folly::SharedPromise<T> &p, folly::Try<T> &&t)
{
	try
	{
		p.setTry(std::move(t));
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
		return false;
	}

	return true;
}

/**
 * Tries to complete the promise \p p with the result of the future \p f as soon as the future is completed.
 * If the promise is already completed at that point in time, nothing happens.
 * To make sure that the future is not deleted in the meantime, it is passed by move reference.
 * However, the promise must be kept alive until the future is completed. Otherwise, it will access an invalid reference.
 * It is not passed by a move reference since it is returned and shouldd be accessable for further try operations.
 * \param p The promise for which it is tried to complete it.
 * \param f The future of which the result is used to try to complete the promise.
 * \param ensureFunc This function is called when the future is completed and can add additional clean ups of memory.
 * \return Returns the passed promise.
 */
template<typename T, typename Func>
folly::Promise<T>& tryCompleteWith(folly::Promise<T> &p, folly::Future<T> &&f, Func &&ensureFunc)
{

	auto ctx = std::make_shared<folly::Future<T>>(std::move(f));

	ctx->setCallback_([&p, ctx, ensureFunc = std::move(ensureFunc)] (folly::Try<T> t)
		{
			tryComplete(p, std::move(t));

			ensureFunc();
		});

	return p;
}

template<typename T>
folly::Promise<T>& tryCompleteWith(folly::Promise<T> &p, folly::Future<T> &&f)
{
	return tryCompleteWith(p, std::move(f), [] () {});
}

template<typename T>
bool tryCompleteSuccess(folly::Promise<T> &p, T &&v)
{
	try
	{
		p.setValue(std::move(v));
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
		return false;
	}

	return true;
}

template<typename T>
bool tryCompleteSuccess(boost::promise<T> &p, T &&v)
{
	try
	{
		p.set_value(std::move(v));
	}
	catch (const boost::promise_already_satisfied &e)
	{
		return false;
	}

	return true;
}

template<typename T, typename Func>
folly::Promise<T>& tryCompleteSuccessWith(folly::Promise<T> &p, folly::Future<T> &&f, Func &&ensureFunc)
{
	auto ctx = std::make_shared<folly::Future<T>>(std::move(f));

	ctx->setCallback_([&p, ctx, ensureFunc = std::move(ensureFunc)] (folly::Try<T> t)
		{
			if (t.hasValue())
			{
				tryCompleteSuccess(p, std::move(t.value()));
			}

			ensureFunc();
		});

	return p;
}

template<typename T>
folly::Promise<T>& tryCompleteSuccessWith(folly::Promise<T> &p, folly::Future<T> &&f)
{
	return tryCompleteSuccessWith(p, std::move(f), [] () { });
}

template<typename T, typename Exception>
bool tryCompleteFailure(folly::Promise<T> &p, Exception &&e)
{
	try
	{
		p.setException(folly::make_exception_wrapper<Exception>(std::move(e)));
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
		return false;
	}

	return true;
}

template<typename T, typename Func>
folly::Promise<T>& tryCompleteFailureWith(folly::Promise<T> &p, folly::Future<T> &&f, Func &&ensureFunc)
{
	auto ctx = std::make_shared<folly::Future<T>>(std::move(f));

	ctx->setCallback_([&p, ctx, ensureFunc = std::move(ensureFunc)] (folly::Try<T> t)
		{
			if (t.hasException())
			{
				// Don't use tryCompleteFailure since we cannot pass Folly's exception wrapper directly.
				try
				{
					p.setException(t.exception());
				}
				catch (const folly::PromiseAlreadySatisfied &e)
				{
				}
			}

			ensureFunc();
		});

	return p;
}

template<typename T>
folly::Promise<T>& tryCompleteFailureWith(folly::Promise<T> &p, folly::Future<T> &&f)
{
	return tryCompleteFailureWith(p, std::move(f), [] () {});
}

template<typename T>
folly::Promise<T> fromTry(folly::Try<T> &&t)
{
	folly::Promise<T> p;
	p.setTry(std::move(t));

	return p;
}

template<typename T, typename Exception>
folly::Promise<T> failed(Exception &&e)
{
	folly::Promise<T> p;
	p.setException(folly::make_exception_wrapper<Exception>(std::move(e)));

	return p;
}

template<typename T>
folly::Promise<T> successful(T &&t)
{
	folly::Promise<T> p;
	p.setValue(t);

	return p;
}
/**
 * \}
 */

/**
 * Paper extensions:
 * \{
 */
/**
 * The same as boost::when_any() and folly::collectAny() but by using tryCompleteWith().
 */
template<typename T>
folly::Future<T> first(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	auto p = std::make_shared<folly::Promise<T>>();
	auto future = p->getFuture();

	tryCompleteWith(*p, std::move(f1), [p] () { });
	tryCompleteWith(*p, std::move(f2), [p] () { });

	return future;
}

/**
 * The same as \ref first() but shuffles both futures in the beginning to avoid explicit prioritising of the first future.
 */
template<typename T>
folly::Future<T> firstRandom(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	// Make a randomized choice between the two futures:
	folly::Future<T> futures[] = {
		std::move(f1),
		std::move(f2)
	};

	const int selection = randomNumber(0, 1);

	return first(std::move(futures[selection]), std::move(futures[1- selection]));
}

/**
 * The same as folly::collect() but ignores any exceptions of the futures and only completes the result with the successful results.
 * If none of the two futures is successful, the program will starve.
 * It will also lead to a memory leak since the passed futures and the generated promise will never be deleted by the ensure() call.
 */
template<typename T>
folly::Future<T> firstOnlySucc(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	auto p = std::make_shared<folly::Promise<T>>();
	auto future = p->getFuture().ensure([p] () { });

	tryCompleteSuccessWith(*p, std::move(f1));
	tryCompleteSuccessWith(*p, std::move(f2));

	return future;
}

/**
 * The same as \ref firstOnlySucc() but shuffles both futures in the beginning to avoid explicit prioritising of the first future.
 */
template<typename T>
folly::Future<T> firstOnlySuccRandom(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	// Make a randomized choice between the two futures:
	folly::Future<T> futures[] = {
		std::move(f1),
		std::move(f2)
	};

	const int selection = randomNumber(0, 1);

	return firstOnlySucc(std::move(futures[selection]), std::move(futures[1- selection]));
}

template<typename T>
folly::Future<T> orElseWithoutMove(folly::Future<T> &first, folly::Future<T> &second)
{
	return first.then([&second] (folly::Try<T> t) mutable
		{
			if (t.hasException())
			{
				second.wait();

				if (second.hasValue())
				{
					return second.get();
				}

				t.exception().throw_exception();
			}

			return t.value();
		}
	);
}

/**
 * Similar to folly::collectAnyWithoutException() but implemented with \ref orElse().
 */
template<typename T>
folly::Future<T> firstSucc(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	struct FirstSuccContext
	{
		FirstSuccContext(folly::Future<T> &&f1, folly::Future<T> &&f2)
			: f1(std::move(f1)), f2(std::move(f2))
		{
		}

		folly::Future<T> f1;
		folly::Future<T> f2;
	};

	auto ctx = std::make_shared<FirstSuccContext>(std::move(f1), std::move(f2));

	return first(
		orElseWithoutMove(ctx->f1, ctx->f2),
		orElseWithoutMove(ctx->f2, ctx->f1))
		.ensure([ctx] () { }
	);
}

template<typename T>
folly::Future<T> firstSucc2(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	struct FirstSucc2Context
	{
		FirstSucc2Context(folly::Future<T> &&f1, folly::Future<T> &&f2)
			: f1(std::move(f1)), f2(std::move(f2))
		{
		}

		folly::Future<T> f1;
		folly::Future<T> f2;
		folly::Future<folly::Unit> f;
		folly::Promise<T> p;
	};

	auto ctx = std::make_shared<FirstSucc2Context>(std::move(f1), std::move(f2));
	auto future = ctx->p.getFuture();

	auto next1 = ctx->f1.then([ctx] (T v)
		{
			tryCompleteSuccess(ctx->p, std::move(v));
		}
	);
	auto next2 = ctx->f2.then([ctx] (T v)
		{
			tryCompleteSuccess(ctx->p, std::move(v));
		}
	);

	ctx->f = orElse(std::move(next1), std::move(next2))
		.onError([ctx] (folly::exception_wrapper wrapper)
			{
				ctx->p.setException(std::move(wrapper));
			}
		).ensure([ctx] () { });

	return future;
}
/**
 * \}
 */

#endif