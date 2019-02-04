/**
 * This performance test compares the performance of creating unique
 * futures/promises vs creating shared futures/promises. Use the types void and
 * folly::Unit to make it possible to create futures and promises for all the
 * three libraries.
 */
#include <future>

#include <boost/thread/future.hpp>

#include <folly/Benchmark.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/futures/SharedPromise.h>
#include <folly/init/Init.h>

const unsigned long long CONTAINERS_NUMBER = 1;
const unsigned long long FUTURES_NUMBER = 1000000;

typedef void FUTURE_VALUE_TYPE;
typedef folly::Unit FUTURE_VALUE_TYPE_FOLLY;

template <typename FutureType>
inline FutureType share_cpp17(std::future<FUTURE_VALUE_TYPE> f)
{
	return f;
}

template <>
inline std::shared_future<FUTURE_VALUE_TYPE>
share_cpp17<std::shared_future<FUTURE_VALUE_TYPE>>(
    std::future<FUTURE_VALUE_TYPE> f)
{
	return f.share();
}

template <typename FutureType>
void test_cpp17_futures()
{
	for (unsigned long long j = 0; j < CONTAINERS_NUMBER; ++j)
	{
		std::vector<FutureType> futures;

		BENCHMARK_SUSPEND
		{
			futures.reserve(FUTURES_NUMBER);
		}

		for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
		{
			futures.push_back(share_cpp17<FutureType>(std::future<FUTURE_VALUE_TYPE>()));
		}

		folly::doNotOptimizeAway(futures);
	}
}

void test_cpp17_promises()
{
	for (unsigned long long j = 0; j < CONTAINERS_NUMBER; ++j)
	{
		std::vector<std::promise<void>> promises;

		BENCHMARK_SUSPEND
		{
			promises.reserve(FUTURES_NUMBER);
		}

		for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
		{
			promises.push_back(std::promise<void>());
			auto f = promises.back().get_future();

			BENCHMARK_SUSPEND
			{
				promises.back().set_value();
			}

			folly::doNotOptimizeAway(f);
		}

		folly::doNotOptimizeAway(promises);
	}
}

template <typename FutureType>
inline FutureType share_boost(boost::future<FUTURE_VALUE_TYPE> &&f)
{
	return std::move(f);
}

template <>
inline boost::shared_future<FUTURE_VALUE_TYPE>
share_boost<boost::shared_future<FUTURE_VALUE_TYPE>>(
    boost::future<FUTURE_VALUE_TYPE> &&f)
{
	return f.share();
}

template <typename FutureType>
void test_boost_futures()
{
	for (unsigned long long j = 0; j < CONTAINERS_NUMBER; ++j)
	{
		std::vector<FutureType> futures;

		BENCHMARK_SUSPEND
		{
			futures.reserve(FUTURES_NUMBER);
		}

		for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
		{
			futures.push_back(
			    share_boost<FutureType>(boost::future<FUTURE_VALUE_TYPE>()));
		}

		folly::doNotOptimizeAway(futures);
	}
}

void test_boost_promises()
{
	for (unsigned long long j = 0; j < CONTAINERS_NUMBER; ++j)
	{
		std::vector<boost::promise<void>> promises;

		BENCHMARK_SUSPEND
		{
			promises.reserve(FUTURES_NUMBER);
		}

		for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
		{
			promises.push_back(boost::promise<void>());
			auto f = promises.back().get_future();

			BENCHMARK_SUSPEND
			{
				promises.back().set_value();
			}

			folly::doNotOptimizeAway(f);
		}

		folly::doNotOptimizeAway(promises);
	}
}

template <typename FutureType>
void test_folly_futures()
{
	for (unsigned long long j = 0; j < CONTAINERS_NUMBER; ++j)
	{
		std::vector<folly::Future<FutureType>> futures;

		BENCHMARK_SUSPEND
		{
			futures.reserve(FUTURES_NUMBER);
		}

		for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
		{
			futures.push_back(folly::Future<FUTURE_VALUE_TYPE_FOLLY>());
		}

		folly::doNotOptimizeAway(futures);
	}
}

template <typename PromiseType>
void test_folly_promises()
{
	for (unsigned long long j = 0; j < CONTAINERS_NUMBER; ++j)
	{
		std::vector<PromiseType> promises;

		BENCHMARK_SUSPEND
		{
			promises.reserve(FUTURES_NUMBER);
		}

		for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
		{
			promises.push_back(PromiseType());
			/*
			 * For shared promises we have to get a future, otherwise no shared state is
			 * allocated which means that shared promiss are simply empty while unique
			 * promises are a shared state. Therefore, shared promises would be much
			 * faster.
			 */
			auto f = promises.back().getFuture();
			folly::doNotOptimizeAway(f);
		}

		folly::doNotOptimizeAway(promises);
	}
}

BENCHMARK(Cpp17UniqueFutures)
{
	test_cpp17_futures<std::future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(Cpp17SharedFutures)
{
	test_cpp17_futures<std::shared_future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(Cpp17UniquePromises)
{
	test_cpp17_promises();
}

BENCHMARK(BoostThreadUniqueFutures)
{
	test_boost_futures<boost::future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(BoostThreadSharedFutures)
{
	test_boost_futures<boost::shared_future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(BostThreadUniquePromises)
{
	test_boost_promises();
}

BENCHMARK(FollyFutures)
{
	test_folly_futures<FUTURE_VALUE_TYPE_FOLLY>();
}

BENCHMARK(FollyUniquePromises)
{
	test_folly_promises<folly::Promise<FUTURE_VALUE_TYPE_FOLLY>>();
}

BENCHMARK(FollySharedPromises)
{
	test_folly_promises<folly::SharedPromise<FUTURE_VALUE_TYPE_FOLLY>>();
}

int main(int argc, char **argv)
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}