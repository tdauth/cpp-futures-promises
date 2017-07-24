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
			if (!t.hasValue())
			{
				second.wait();

				if (second.hasValue())
				{
					return second.value();
				}

				t.exception().throw_exception();
			}

			return t.value();
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
 * Similar to folly::collectN() but without returning the indices:
 * Starts an asynchronous call which calls boost::wait_for_any() \p n times and moves the resulting futures into the resulting container.
 *
 * \param first The iterator which points to the first element of the collection.
 * \param last The element which points to the last element of the collection.
 * \return Returns a boost::future<std::vector<boost::future<T>>>. The indices cannot be returned since boos::wait_for_any() does only return an iterator.
 */
template<typename InputIterator>
boost::future<std::vector<typename InputIterator::value_type>>
when_n(InputIterator first, InputIterator last, std::size_t n)
{
	typedef typename InputIterator::value_type value_type;
	typedef std::vector<value_type> result_type;
	const std::size_t size = std::distance(first, last);

	if (size < n)
	{
		return boost::make_exceptional_future<result_type>(
			std::runtime_error("Not enough futures")
		);
	}

	return boost::async([first, last, n, size] ()
		{
			result_type tmp;
			tmp.reserve(size);
			std::move(first, last, std::back_inserter(tmp));

			result_type result;
			result.reserve(n);

			for (std::size_t i = 0; i < n; ++i)
			{
				typename result_type::iterator any =
					boost::wait_for_any(tmp.begin(), tmp.end());
				result.push_back(std::move(*any));
				tmp.erase(any);
			}

			return result;
		}
	);
}

/**
 * This version uses an implementation which is more similar to Folly's version.
 * Actually, the source code is almost the same as in Folly's "Future-inl.h" file
 * but with the interface of Boost.Thread.
 * It does also return the indices.
 *
 * \param first The iterator which points to the first element of the collection.
 * \param last The element which points to the last element of the collection.
 * \return Returns a boost::future<std::vector<std::pair<std::size_t, boost::future<T>>>>. The pair holds the original index of the future and the future itself.
 */
template<typename InputIterator>
boost::future<std::vector<std::pair<std::size_t, typename InputIterator::value_type>>>
when_n2(InputIterator first, InputIterator last, std::size_t n)
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
	std::size_t i = 0;

	std::for_each(first, last, [ctx, n, &i] (future_type &v)
		{
			v.then([ctx, n, i] (future_type f)
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

			++i;
		}
	);

	return ctx->p.get_future();
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

template<typename T>
folly::Future<T> firstRandom(folly::Future<T> &&f1, folly::Future<T> &&f2)
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

	// Make a randomized choice between the two futures:
	folly::Future<T> futures[] = {
		std::move(f1),
		std::move(f2)
	};

	const int selection = select();

	auto ctx = std::make_shared<FirstContext>(std::move(futures[selection]), std::move(futures[1- selection]));
	folly::Future<T> future = ctx->p.getFuture();

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
 * \}
 */

#endif