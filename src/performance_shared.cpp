/**
 * This performance test compares the performance of creating unique futures/promises vs creating shared futures/promises.
 */
#include <future>

#include <boost/thread/future.hpp>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/futures/SharedPromise.h>
#include <folly/futures/Promise.h>
#include <folly/futures/Future.h>

const unsigned long long FUTURES_NUMBER = 1000000;

typedef int FUTURE_VALUE_TYPE;
typedef int FUTURE_VALUE_TYPE_FOLLY;

template<typename FutureType>
inline FutureType share_cpp14(std::future<FUTURE_VALUE_TYPE> f)
{
	return f;
}

template<>
inline std::shared_future<FUTURE_VALUE_TYPE> share_cpp14<std::shared_future<FUTURE_VALUE_TYPE>>(std::future<FUTURE_VALUE_TYPE> f)
{
	return f.share();
}

template<typename FutureType>
void test_cpp14()
{
	std::vector<FutureType> futures;
	futures.reserve(FUTURES_NUMBER);

	for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
	{
		futures.push_back(share_cpp14<FutureType>(std::async([]() { return 10; })));
	}
}

template<typename FutureType>
inline FutureType share_boost(boost::future<FUTURE_VALUE_TYPE> &&f)
{
	return std::move(f);
}

template<>
inline boost::shared_future<FUTURE_VALUE_TYPE> share_boost<boost::shared_future<FUTURE_VALUE_TYPE>>(boost::future<FUTURE_VALUE_TYPE> &&f)
{
	return f.share();
}

template<typename FutureType>
void test_boost()
{
	std::vector<FutureType> futures;
	futures.reserve(FUTURES_NUMBER);

	for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
	{
		futures.push_back(share_boost<FutureType>(boost::make_future(10)));
	}
}

template<typename FutureType>
void test_folly_futures()
{
	std::vector<folly::Future<FutureType>> futures;
	futures.reserve(FUTURES_NUMBER);

	for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
	{
		futures.push_back(folly::makeFuture(10));
	}
}

template<typename PromiseType, typename FutureType>
void test_folly()
{
	std::vector<folly::Future<FutureType>> futures;
	futures.reserve(FUTURES_NUMBER);

	for (unsigned long long i = 0; i < FUTURES_NUMBER; ++i)
	{
		PromiseType p = PromiseType();
		p.setValue(10);
		futures.push_back(p.getFuture());
	}
}

BENCHMARK(Cpp17UniqueFutures)
{
	test_cpp14<std::future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(Cpp17SharedFutures)
{
	test_cpp14<std::shared_future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(BoostThreadUniqueFutures)
{
	test_boost<boost::future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(BoostThreadSharedFutures)
{
	test_boost<boost::shared_future<FUTURE_VALUE_TYPE>>();
}

BENCHMARK(FollyFutures)
{
	test_folly_futures<FUTURE_VALUE_TYPE_FOLLY>();
}

BENCHMARK(FollyUniquePromises)
{
	test_folly<folly::Promise<FUTURE_VALUE_TYPE_FOLLY>, FUTURE_VALUE_TYPE_FOLLY>();
}

BENCHMARK(FollySharedPromises)
{
	test_folly<folly::SharedPromise<FUTURE_VALUE_TYPE_FOLLY>, FUTURE_VALUE_TYPE_FOLLY>();
}

int main(int argc, char **argv)
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}