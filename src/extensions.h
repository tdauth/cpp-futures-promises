#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <algorithm>
#include <iterator>

#include <boost/thread.hpp>

#include <folly/futures/Future.h>

#include <glog/logging.h>

#include <execinfo.h>

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
 * Similar to folly::collectN():
 * Starts an asynchronous call which calls boost::wait_for_any() \p n times and moves the resulting futures into the resulting container.
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

#endif