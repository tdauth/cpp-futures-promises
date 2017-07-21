#define BOOST_TEST_MODULE Extensions
#include <boost/test/unit_test.hpp>

#include "extensions.h"
#include "folly_fixture.h"

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