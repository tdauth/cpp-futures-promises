#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <algorithm>
#include <iterator>

#include <boost/thread.hpp>

#include <folly/futures/Future.h>

#include <glog/logging.h>

#include <execinfo.h>

#include "random.h"

template <class InputIterator>
folly::Future<std::vector<std::pair<size_t, typename std::iterator_traits<InputIterator>::value_type::value_type>>>
collectNWithoutException(InputIterator first, InputIterator last, size_t n)
{
	typedef typename std::iterator_traits<InputIterator>::value_type::value_type T;
	typedef std::vector<std::pair<size_t, T>> V;

	struct CollectNContext
	{
		V v;
		std::atomic<size_t> completed = {0};
		std::atomic<size_t> failed = {0};
		folly::Promise<V> p;
	};

	auto ctx = std::make_shared<CollectNContext>();
	const size_t total = std::distance(first, last);

	if (total < n)
	{
		ctx->p.setException(std::runtime_error("Not enough futures"));
	}
	else
	{
		// for each completed Future, increase count and add to vector, until we
		// have n completed futures at which point we fulfil our Promise with the
		// vector
		folly::mapSetCallback<T>(first, last, [ctx, n, total] (size_t i, folly::Try<T>&& t)
			{
				// ignore exceptions until as many futures failed that n futures cannot be completed successfully anymore
				if (t.hasException())
				{
					if (!ctx->p.isFulfilled())
					{
						auto c = ++ctx->failed;

						if (total - c < n)
						{
							ctx->p.setException(t.exception());
						}
					}
				}
				else if (!ctx->p.isFulfilled())
				{
					auto c = ++ctx->completed;

					if (c <= n)
					{
						assert(ctx->v.size() < n);
						ctx->v.emplace_back(i, std::move(t.value()));

						if (c == n)
						{
							ctx->p.setValue(std::move(ctx->v));
						}
					}
				}
			}
		);
	}

	return ctx->p.getFuture();
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

namespace
{

void logBacktrace()
{
	const int maxSize = 50;
	// http://man7.org/linux/man-pages/man3/backtrace.3.html
	void *buffer[maxSize];
	const int size = backtrace(buffer, maxSize);
	char **text = backtrace_symbols(buffer, size);

	if (text != NULL)
	{
		for (int i = 0; i < size; ++i)
		{
			LOG(ERROR) << text[i];
		}

		free(text);
	}
	else
	{
		LOG(ERROR) << "backtrace does not work!";
	}
}

}

/**
 * Registers a callback to the future \p f and returns the same future.
 * The callback should not return anything, it is simply called when the future has been completed.
 * This combinator can be used to order calls after the future's completion.
 * \param f A future for which the callback is registered.
 * \p func The callback which gets folly::Try as parameter.
 * \return Returns the a new future with the same result as the passed future.
 */
template<typename T, typename Func>
folly::Future<T> andThen(folly::Future<T> &&f, Func &&func)
{
	return f.then([func = std::move(func)] (folly::Try<T> t)
	{
		try
		{
			func(t);
		}
		catch (const std::exception &e)
		{
			LOG(ERROR) << "Exception: " << e.what();

			logBacktrace();
		}
		catch (...)
		{
			LOG(ERROR) << "Unknown exception.";

			logBacktrace();
		}

		return folly::Future<T>(t);
	});
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
	typedef typename std::pair<std::size_t, future_type> value_type;
	typedef std::vector<value_type> result_type;
	const std::size_t size = std::distance(first, last);

	if (size < n)
	{
		return boost::make_exceptional_future<result_type>(
			std::runtime_error("Not enough futures")
		);
	}

	struct CollectNContext
	{
		result_type v;
		std::atomic<size_t> completed = {0};
		boost::promise<result_type> p;
	};

	auto ctx = std::make_shared<CollectNContext>();

	mapSetCallbackBoost<typename future_type::value_type>(first, last, [ctx, n] (std::size_t i, future_type f)
		{
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
 * \{
 */
template<typename T>
bool tryComplete(folly::Promise<T> &p, folly::Try<T> &&t)
{
	try
	{
		p.setTry(std::move(t));

		return true;
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
	}

	return false;
}

template<typename T>
folly::Promise<T>& tryCompleteWith(folly::Promise<T> &p, folly::Future<T> &f)
{
	f.setCallback_([&p] (folly::Try<T> t)
		{
			tryComplete(p, std::move(t));
		}
	);

	return p;
}

template<typename T>
bool tryCompleteSuccess(folly::Promise<T> &p, T &&v)
{
	try
	{
		p.setValue(std::move(v));

		return true;
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
	}

	return false;
}

template<typename T>
bool tryCompleteSuccess(boost::promise<T> &p, T &&v)
{
	try
	{
		p.set_value(std::move(v));

		return true;
	}
	catch (const boost::promise_already_satisfied &e)
	{
	}

	return false;
}

template<typename T>
folly::Promise<T>& tryCompleteSuccessWith(folly::Promise<T> &p, folly::Future<T> &f)
{
	f.setCallback_([&p] (folly::Try<T> t)
		{
			if (t.hasValue())
			{
				tryCompleteSuccess(p, std::move(t.value()));
			}
		}
	);

	return p;
}

template<typename T, typename Exception>
bool tryCompleteFailure(folly::Promise<T> &p, Exception &&e)
{
	try
	{
		p.setException(folly::make_exception_wrapper<Exception>(std::move(e)));

		return true;
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
	}

	return false;
}

template<typename T>
folly::Promise<T>& tryCompleteFailureWith(folly::Promise<T> &p, folly::Future<T> &f)
{
	f.setCallback_([&p] (folly::Try<T> t)
		{
			try
			{
				p.setException(t.exception());
			}
			catch (const folly::PromiseAlreadySatisfied &e)
			{
			}
		}
	);

	return p;
}

template<typename T>
folly::Promise<T>& completeWith(folly::Promise<T> &p, folly::Future<T> &f)
{
	tryCompleteWith(p, f);

	return p;
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
	struct FirstContext
	{
		FirstContext(folly::Future<T> &&f1, folly::Future<T> &&f2) : f1(std::move(f1)), f2(std::move(f2))
		{
		}

		folly::Future<T> f1;
		folly::Future<T> f2;
		folly::Promise<T> p;
	};

	auto ctx = std::make_shared<FirstContext>(std::move(f1), std::move(f2));
	folly::Future<T> future = ctx->p.getFuture();

	// TODO check if the callback is executed atomically before the other callback, otherwise the priority does not always work!
	// tryCompleteWith() + release of the shared context:
	ctx->f1.setCallback_([ctx] (folly::Try<T> t)
		{
			tryComplete(ctx->p, std::move(t));
		}
	);
	ctx->f2.setCallback_([ctx] (folly::Try<T> t)
		{
			tryComplete(ctx->p, std::move(t));
		}
	);

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

	const int selection = select();

	return first(std::move(futures[selection]), std::move(futures[1- selection]));
}

/**
 * The same as folly::collect() but ignores any exceptions of the futures and only completes the result with the successful results.
 * If none of the two futures is successful it will fail with an exception of the type folly::BrokenPromise. This prevents the starvation
 * of the program.
 */
template<typename T>
folly::Future<T> firstOnlySucc(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	struct FirstOnlySuccContext
	{
		FirstOnlySuccContext(folly::Future<T> &&f1, folly::Future<T> &&f2) : f1(std::move(f1)), f2(std::move(f2))
		{
		}

		folly::Future<T> f1;
		folly::Future<T> f2;
		folly::Promise<T> p;
	};

	auto ctx = std::make_shared<FirstOnlySuccContext>(std::move(f1), std::move(f2));
	folly::Future<T> future = ctx->p.getFuture();

	// TODO check if the callback is executed atomically before the other callback, otherwise the priority does not always work!
	// tryCompleteWithSuccess() + release of the shared context (only then() allows passing the direct value):
	ctx->f1.then([ctx] (T t)
		{
			tryCompleteSuccess(ctx->p, std::move(t));
		}
	);
	ctx->f2.then([ctx] (T t)
		{
			tryCompleteSuccess(ctx->p, std::move(t));
		}
	);

	return future;
}

/**
 * The same as \ref firstOnlySucc() but for Boost.
 */
template<typename T>
boost::future<T> firstOnlySucc(boost::future<T> &&f1, boost::future<T> &&f2)
{
	struct FirstOnlySuccContext
	{
		FirstOnlySuccContext(boost::future<T> &&f1, boost::future<T> &&f2) : f1(std::move(f1)), f2(std::move(f2))
		{
		}

		boost::future<T> f1;
		boost::future<T> f2;
		boost::promise<T> p;
	};

	auto ctx = std::make_shared<FirstOnlySuccContext>(std::move(f1), std::move(f2));
	boost::future<T> future = ctx->p.get_future();

	// TODO check if the callback is executed atomically before the other callback, otherwise the priority does not always work!
	// tryCompleteWithSuccess() + release of the shared context (only then() allows passing the direct value):
	ctx->f1.then([ctx] (boost::future<T> f)
		{
			if (f.has_value())
			{
				tryCompleteSuccess(ctx->p, std::move(f.get()));
			}
		}
	);
	ctx->f2.then([ctx] (boost::future<T> f)
		{
			if (f.has_value())
			{
				tryCompleteSuccess(ctx->p, std::move(f.get()));
			}
		}
	);

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

	const int selection = select();

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
		FirstSuccContext(folly::Future<T> &&f1, folly::Future<T> &&f2) : f1(std::move(f1)), f2(std::move(f2))
		{
		}

		folly::Future<T> f1;
		folly::Future<T> f2;
	};

	auto ctx = std::make_shared<FirstSuccContext>(std::move(f1), std::move(f2));

	return first(orElseWithoutMove(ctx->f1, ctx->f2), orElseWithoutMove(ctx->f2, ctx->f1))
		.then([ctx] (folly::Try<T> t)
			{
				t.throwIfFailed();

				return t.value();
			}
		);
}

template<typename T>
folly::Future<T> firstSucc2(folly::Future<T> &&f1, folly::Future<T> &&f2)
{
	struct FirstSucc2Context
	{
		FirstSucc2Context(folly::Future<T> &&f1, folly::Future<T> &&f2) : f1(std::move(f1)), f2(std::move(f2))
		{
		}

		folly::Future<T> f1;
		folly::Future<T> f2;
		folly::Future<folly::Unit> f;
		folly::Promise<T> p;
	};

	auto ctx = std::make_shared<FirstSucc2Context>(std::move(f1), std::move(f2));
	folly::Future<T> future = ctx->p.getFuture();

	// tryCompleteWithSuccess() + release of the shared context (only then() allows passing the direct value):
	auto next1 = ctx->f1.then([ctx] (T t)
		{
			tryCompleteSuccess(ctx->p, std::move(t));
		}
	);
	auto next2 = ctx->f2.then([ctx] (T t)
		{
			tryCompleteSuccess(ctx->p, std::move(t));
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