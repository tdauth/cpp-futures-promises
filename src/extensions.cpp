#define BOOST_TEST_MODULE Extensions
#include <boost/test/unit_test.hpp>

#include "extensions.h"
#include "folly_fixture.h"

BOOST_AUTO_TEST_CASE(OrElseFolly)
{
	folly::Future<int> f0 = folly::makeFuture(10);
	folly::Future<int> f1 = folly::makeFuture(11);
	folly::Future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(OrElseFollyFirstFailed)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(11);
	folly::Future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_EQUAL(11, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(OrElseFollyBothFailed)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));
	folly::Future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(OrElseBoost)
{
	boost::future<int> f0 = boost::make_ready_future(10);
	boost::future<int> f1 = boost::make_ready_future(11);
	boost::future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_EQUAL(10, f.get());
}

BOOST_AUTO_TEST_CASE(OrElseBoostFirstFailed)
{
	boost::future<int> f0 = boost::make_exceptional_future<int>(std::runtime_error("Failure"));
	boost::future<int> f1 = boost::make_ready_future(11);
	boost::future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_EQUAL(11, f.get());
}

BOOST_AUTO_TEST_CASE(OrElseBoostBothFailed)
{
	boost::future<int> f0 = boost::make_exceptional_future<int>(std::runtime_error("Failure 0"));
	boost::future<int> f1 = boost::make_exceptional_future<int>(std::runtime_error("Failure 1"));
	boost::future<int> f = orElse(std::move(f0), std::move(f1));

	BOOST_REQUIRE_THROW(f.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenN)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = whenN(futures.begin(), futures.end(), 2);
	std::vector<std::pair<std::size_t, boost::future<int>>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
	/*
	 * The order is not guaranteed (look at the atomic operations in the callback):
	 */
	BOOST_CHECK_NO_THROW(v[0].second.get());
	BOOST_CHECK_NO_THROW(v[1].second.get());
}

BOOST_AUTO_TEST_CASE(WhenNFailureNotEnough)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = whenN(futures.begin(), futures.end(), 4);
	BOOST_REQUIRE_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenNWithExceptions)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure 0")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure 1")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure 2")));

	boost::future<std::vector<std::pair<std::size_t, boost::future<int>>>> results = whenN(futures.begin(), futures.end(), 2);
	std::vector<std::pair<std::size_t, boost::future<int>>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
	/*
	 * The order is not guaranteed (look at the atomic operations in the callback):
	 */
	BOOST_CHECK_THROW(v[0].second.get(), std::runtime_error);
	BOOST_CHECK_THROW(v[1].second.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenNSucc)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, int>>> results = whenNSucc(futures.begin(), futures.end(), 2);
	results.wait();

	BOOST_REQUIRE(results.has_value());

	std::vector<std::pair<std::size_t, int>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
}

BOOST_AUTO_TEST_CASE(WhenNSuccFailureNotEnough)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));

	boost::future<std::vector<std::pair<std::size_t, int>>> results = whenNSucc(futures.begin(), futures.end(), 2);
	BOOST_CHECK_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenNSuccOneFails)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, int>>> results = whenNSucc(futures.begin(), futures.end(), 2);
	results.wait();

	BOOST_REQUIRE(results.has_value());

	std::vector<std::pair<std::size_t, int>> v = results.get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
}

BOOST_AUTO_TEST_CASE(WhenNSuccTwoFail)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure 0")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure 1")));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::vector<std::pair<std::size_t, int>>> results = whenNSucc(futures.begin(), futures.end(), 2);
	BOOST_REQUIRE_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenAnyOnlyOne)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::pair<std::size_t, boost::future<int>>> results = whenAny(futures.begin(), futures.end());
	std::pair<std::size_t, boost::future<int>> r = results.get();

	/*
	 * The order is not guaranteed (look at the atomic operations in the callback):
	 */
	BOOST_CHECK_NO_THROW(r.second.get());
}

BOOST_AUTO_TEST_CASE(WhenAnyAllFail)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));

	boost::future<std::pair<std::size_t, boost::future<int>>> results = whenAny(futures.begin(), futures.end());
	std::pair<std::size_t, boost::future<int>> r = results.get();

	/*
	 * The order is not guaranteed (look at the atomic operations in the callback):
	 */
	BOOST_CHECK_THROW(r.second.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(WhenAnySuccOnlyOne)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_ready_future(2));

	boost::future<std::pair<std::size_t, int>> results = whenAnySucc(futures.begin(), futures.end());
	BOOST_CHECK_NO_THROW(results.get());
}

BOOST_AUTO_TEST_CASE(WhenAnySuccAllFail)
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));

	boost::future<std::pair<std::size_t, int>> results = whenAnySucc(futures.begin(), futures.end());

	BOOST_CHECK_THROW(results.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(CollectNWithoutException)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture(0));
	futures.push_back(folly::makeFuture(1));
	futures.push_back(folly::makeFuture(2));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);
	std::vector<std::pair<size_t, int>> v = std::move(results).get();

	BOOST_REQUIRE_EQUAL((std::size_t)2, v.size());
	BOOST_CHECK_EQUAL(0, v[0].second);
	BOOST_CHECK_EQUAL(1, v[1].second);
}

BOOST_AUTO_TEST_CASE(CollectNWithoutExceptionFailureNotEnough)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture(0));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);
	BOOST_CHECK_THROW(std::move(results).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(CollectNWithoutExceptionOneFailure)
{
	std::vector<folly::Future<int>> futures;
	futures.push_back(folly::makeFuture<int>(std::runtime_error("Failure")));
	futures.push_back(folly::makeFuture(1));
	futures.push_back(folly::makeFuture(2));

	folly::Future<std::vector<std::pair<size_t, int>>> results = collectNWithoutException(futures.begin(), futures.end(), 2);
	std::vector<std::pair<size_t, int>> v = std::move(results).get();

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
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteException)
{
	folly::Try<int> t(std::runtime_error("Failure!"));
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryComplete(p, std::move(t)));
	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TryCompleteUninitialized)
{
	folly::Try<int> t;
	folly::Promise<int> p;

	BOOST_REQUIRE(!tryComplete(p, std::move(t)));
}

BOOST_AUTO_TEST_CASE(TryCompleteFails)
{
	folly::Try<int> t(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();
	p.setValue(10);

	BOOST_REQUIRE(!tryComplete(p, std::move(t)));
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteShared)
{
	folly::Try<int> t(10);
	folly::SharedPromise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryComplete(p, std::move(t)));
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteExceptionShared)
{
	folly::Try<int> t(std::runtime_error("Failure!"));
	folly::SharedPromise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryComplete(p, std::move(t)));
	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TryCompleteUninitializedShared)
{
	folly::Try<int> t;
	folly::SharedPromise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(!tryComplete(p, std::move(t)));
}

BOOST_AUTO_TEST_CASE(TryCompleteFailsShared)
{
	folly::Try<int> t;
	folly::SharedPromise<int> p;
	folly::Future<int> f = p.getFuture();
	p.setValue(10);

	BOOST_REQUIRE(!tryComplete(p, std::move(t)));
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteWith)
{
	folly::Future<int> future = folly::makeFuture(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	tryCompleteWith(p, std::move(future));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteSuccess)
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryCompleteSuccess(p, 10));
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteSuccessFails)
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();
	p.setValue(10);

	BOOST_REQUIRE(!tryCompleteSuccess(p, 10));
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteSuccessWith)
{
	folly::Future<int> future = folly::makeFuture(10);
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	tryCompleteSuccessWith(p, std::move(future));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteFailure)
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	BOOST_REQUIRE(tryCompleteFailure(p, std::runtime_error("Failure")));
	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TryCompleteFailureFails)
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();
	p.setValue(10);

	BOOST_REQUIRE(!tryCompleteFailure(p, std::runtime_error("Failure")));
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(TryCompleteFailureWith)
{
	folly::Future<int> future = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();

	tryCompleteFailureWith<int>(p, std::move(future));

	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FromTry)
{
	folly::Promise<int> p = fromTry(folly::Try<int>(10));
	BOOST_CHECK(p.isFulfilled());
	folly::Future<int> f = p.getFuture();
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(Failed)
{
	folly::Promise<int> p = failed<int>(std::runtime_error("Failure"));
	BOOST_CHECK(p.isFulfilled());
	folly::Future<int> f = p.getFuture();
	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Successful)
{
	folly::Promise<int> p = successful(10);
	BOOST_CHECK(p.isFulfilled());
	folly::Future<int> f = p.getFuture();
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(First)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = first(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(FirstFirstFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = first(std::move(f1), std::move(f2));

	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FirstBothFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure0 "));
	folly::Future<int> f2 = folly::makeFuture<int>(std::runtime_error("Failure 1"));
	folly::Future<int> f = first(std::move(f1), std::move(f2));

	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FirstRandom)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstRandom(std::move(f1), std::move(f2));

	const int result = std::move(f).get();

	BOOST_CHECK(result == 10 || result == 11);
}

BOOST_AUTO_TEST_CASE(FirstOnlySucc)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstOnlySucc(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(FirstOnlySuccFirstFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstOnlySucc(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(11, std::move(f).get());
}

/**
 * Since both fail, the program will starve if no timeout is used.
 */
BOOST_AUTO_TEST_CASE(FirstOnlySuccBothFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f2 = folly::makeFuture<int>(std::runtime_error("Failure 1"));
	folly::Future<int> f = firstOnlySucc(std::move(f1), std::move(f2)).within(std::chrono::seconds(1));

	BOOST_CHECK_THROW(std::move(f).get(), folly::FutureTimeout);

	/*
	 * Memory leak because the resulting future of firstOnlySucc is never completed and ensure is never called which holds the final copy of the shared pointer.
	 */
	// TODO Call the ensure of f manually from outside if it is possible.
}

BOOST_AUTO_TEST_CASE(FirstOnlySuccRandom)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstOnlySuccRandom(std::move(f1), std::move(f2));

	const int result = std::move(f).get();

	BOOST_CHECK(result == 10 || result == 11);
}

BOOST_AUTO_TEST_CASE(FirstSucc)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstSucc(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(FirstSuccFirstFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstSucc(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(11, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(FirstSuccBothFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f2 = folly::makeFuture<int>(std::runtime_error("Failure 1"));
	folly::Future<int> f = firstSucc(std::move(f1), std::move(f2));

	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FirstSuccRandom)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstSuccRandom(std::move(f1), std::move(f2));

	const int result = std::move(f).get();

	BOOST_CHECK(result == 10 || result == 11);
}

BOOST_AUTO_TEST_CASE(FirstSucc2)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstSucc2(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(FirstSucc2FirstFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstSucc2(std::move(f1), std::move(f2));

	BOOST_CHECK_EQUAL(11, std::move(f).get());
}

BOOST_AUTO_TEST_CASE(FirstSucc2BothFailed)
{
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f2 = folly::makeFuture<int>(std::runtime_error("Failure 1"));
	folly::Future<int> f = firstSucc2(std::move(f1), std::move(f2));

	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FirstSucc2Random)
{
	folly::Future<int> f1 = folly::makeFuture(10);
	folly::Future<int> f2 = folly::makeFuture(11);
	folly::Future<int> f = firstSucc2Random(std::move(f1), std::move(f2));

	const int result = std::move(f).get();

	BOOST_CHECK(result == 10 || result == 11);
}
