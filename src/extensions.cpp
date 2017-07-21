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

	boost::future<std::vector<boost::future<int>>> results = when_n(futures.begin(), futures.end(), 2);
	std::vector<boost::future<int>> v = results.get();

	BOOST_REQUIRE_EQUAL(2, v.size());
	BOOST_CHECK_EQUAL(0, v[0].get());
	BOOST_CHECK_EQUAL(1, v[1].get());
}

BOOST_AUTO_TEST_CASE(WhenNFailure)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<boost::future<int>>> results = when_n(futures.begin(), futures.end(), 4);
	BOOST_REQUIRE_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenN2)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = when_n2(futures.begin(), futures.end(), 2);
	std::vector<std::pair<std::size_t, boost::future<int>>> v = results.get();

	BOOST_REQUIRE_EQUAL(2, v.size());
	BOOST_CHECK_EQUAL(0, v[0].second.get());
	BOOST_CHECK_EQUAL(1, v[1].second.get());
}

BOOST_AUTO_TEST_CASE(WhenN2Failure)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = when_n2(futures.begin(), futures.end(), 4);
	BOOST_REQUIRE_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(CollectNWithoutException)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture(0));
	futures.push_back(folly::makeFuture(1));
	futures.push_back(folly::makeFuture(2));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);
	std::vector<std::pair<size_t, int>> v = results.get();

	BOOST_REQUIRE_EQUAL(2, v.size());
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

	BOOST_REQUIRE_EQUAL(2, v.size());
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