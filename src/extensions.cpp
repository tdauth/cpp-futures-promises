#define BOOST_TEST_MODULE Extensions
#include <boost/test/unit_test.hpp>

#include "extensions.h"
#include "folly_fixture.h"

BOOST_AUTO_TEST_CASE(OrElse)
{
	folly::Future<int> f0 = folly::makeFuture(10);
	folly::Future<int> f1 = folly::makeFuture(11);
	folly::Future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(OrElseFirstFailed)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(11);
	folly::Future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_EQUAL(11, f.get());
}

BOOST_AUTO_TEST_CASE(OrElseBothFailed)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));
	folly::Future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_THROW(f.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(AndThen)
{
	int v = 0;
	folly::Future<int> f0 = folly::makeFuture(10);
	folly::Future<int> f1 = andThen(std::move(f0), [&v] (folly::Try<int> t) { v = t.value(); });

	BOOST_REQUIRE_EQUAL(10, f1.get());
	BOOST_REQUIRE_EQUAL(10, v);
}

BOOST_AUTO_TEST_CASE(WhenN)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = when_n(futures.begin(), futures.end(), 2);
	std::vector<std::pair<std::size_t, boost::future<int>>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
	/*
	 * The order is not guaranteed (look at the atomic operations in the callback):
	 */
	BOOST_CHECK_NO_THROW(v[0].second.get());
	BOOST_CHECK_NO_THROW(v[1].second.get());
}

BOOST_AUTO_TEST_CASE(WhenNFailure)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = when_n(futures.begin(), futures.end(), 4);
	BOOST_REQUIRE_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenAnyOnlyOne)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::pair<std::size_t, boost::future<int>>> results = when_any_only_one(futures.begin(), futures.end());
	std::pair<std::size_t, boost::future<int>> r = results.get();

	/*
	 * The order is not guaranteed (look at the atomic operations in the callback):
	 */
	BOOST_CHECK_NO_THROW(r.second.get());
}

BOOST_AUTO_TEST_CASE(CollectNWithoutException)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture(0));
	futures.push_back(folly::makeFuture(1));
	futures.push_back(folly::makeFuture(2));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);
	std::vector<std::pair<size_t, int>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
	BOOST_CHECK_EQUAL(0, v[0].second);
	BOOST_CHECK_EQUAL(1, v[1].second);
}

BOOST_AUTO_TEST_CASE(CollectNWithoutExceptionOneFailure)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture<int>(std::runtime_error("Failure")));
	futures.push_back(folly::makeFuture(1));
	futures.push_back(folly::makeFuture(2));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);
	std::vector<std::pair<size_t, int>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
	BOOST_CHECK_EQUAL(1, v[0].second);
	BOOST_CHECK_EQUAL(2, v[1].second);
}

BOOST_AUTO_TEST_CASE(CollectNWithoutExceptionTwoFailures)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture<int>(std::runtime_error("Failure 0")));
	futures.push_back(folly::makeFuture<int>(std::runtime_error("Failure 1")));
	futures.push_back(folly::makeFuture(2));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);

	results.wait();

	BOOST_CHECK_EQUAL("Failure 1", results.getTry().exception().get_exception()->what());
}

BOOST_AUTO_TEST_CASE(TryComplete)
{
	folly::Try<int> t(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryComplete(p, std::move(t)));
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(TryCompleteWith)
{
	folly::Future<int> future = folly::makeFuture(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	tryCompleteWith(p, future);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(TryCompleteSuccess)
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryCompleteSuccess(p, 10));
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(TryCompleteSuccessWith)
{
	folly::Future<int> future = folly::makeFuture(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	tryCompleteSuccessWith(p, future);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(TryCompleteFailure)
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryCompleteFailure(p, std::runtime_error("Failure")));
	BOOST_CHECK_THROW(f.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TryCompleteFailureWith)
{
	folly::Future<int> future = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	tryCompleteFailureWith<int>(p, future);

	BOOST_CHECK_THROW(f.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(CompleteWith)
{
	folly::Future<int> future = folly::makeFuture(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	completeWith(p, future);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(First)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = first(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(FirstRandom)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstRandom(std::move(f1), std::move(f2));

	const int result = f.get();

	BOOST_CHECK(result == 10 || result == 11);
}