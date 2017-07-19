#define BOOST_TEST_MODULE Choices
#include <boost/test/unit_test.hpp>

#include "choice_combinators.h"
#include "folly_fixture.h"

BOOST_AUTO_TEST_CASE(FirstSuccessBothSucceed)
{
	folly::Future<int> f0 = folly::makeFuture(0);
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccess(std::move(f0), std::move(f1));

	BOOST_CHECK_EQUAL(0, f.get());
}

BOOST_AUTO_TEST_CASE(FirstSuccessOneFails)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccess(std::move(f0), std::move(f1));

	BOOST_CHECK_EQUAL(1, f.get());
}

BOOST_AUTO_TEST_CASE(FirstSuccessBothFail)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));

	folly::Future<int> f = firstSuccess(std::move(f0), std::move(f1));

	BOOST_CHECK_EQUAL("Failure 1", f.getTry().exception().get_exception()->what());
}

BOOST_AUTO_TEST_CASE(FirstSuccessRandomBothSucceed)
{
	folly::Future<int> f0 = folly::makeFuture(0);
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccessRandom(std::move(f0), std::move(f1));

	f.wait();

	BOOST_CHECK(f.value() == 0 || f.value() == 1);
}

BOOST_AUTO_TEST_CASE(FirstSuccessRandomOneFails)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccessRandom(std::move(f0), std::move(f1));

	BOOST_CHECK_EQUAL(1, f.get());
}

BOOST_AUTO_TEST_CASE(FirstSuccessRandomBothFail)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));

	folly::Future<int> f = firstSuccessRandom(std::move(f0), std::move(f1));

	f.wait();
	const std::string what = f.getTry().exception().get_exception()->what();

	BOOST_CHECK(what == "Failure 0" || what == "Failure 1");
}

BOOST_AUTO_TEST_CASE(FirstFailureBothSucceed)
{
	folly::Future<int> f0 = folly::makeFuture(0);
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstFailure(std::move(f0), std::move(f1));

	BOOST_CHECK_THROW(f.get(), folly::BrokenPromise);
}

BOOST_AUTO_TEST_CASE(FirstFailureOneFails)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstFailure(std::move(f0), std::move(f1));

	f.wait();
	const std::string what = f.getTry().exception().get_exception()->what();

	BOOST_CHECK_EQUAL("Failure", what);
}

BOOST_AUTO_TEST_CASE(FirstFailureBothFail)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));

	folly::Future<int> f = firstFailure(std::move(f0), std::move(f1));

	f.wait();
	const std::string what = f.getTry().exception().get_exception()->what();

	BOOST_CHECK_EQUAL("Failure 0", what);
}

BOOST_AUTO_TEST_CASE(FirstFailureRandomBothSucceed)
{
	folly::Future<int> f0 = folly::makeFuture(0);
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstFailureRandom(std::move(f0), std::move(f1));

	BOOST_CHECK_THROW(f.get(), folly::BrokenPromise);
}

BOOST_AUTO_TEST_CASE(FirstFailureRandomOneFails)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstFailureRandom(std::move(f0), std::move(f1));

	f.wait();
	const std::string what = f.getTry().exception().get_exception()->what();

	BOOST_CHECK_EQUAL("Failure", what);
}

BOOST_AUTO_TEST_CASE(FirstFailureRandomBothFail)
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));

	folly::Future<int> f = firstFailureRandom(std::move(f0), std::move(f1));

	f.wait();
	const std::string what = f.getTry().exception().get_exception()->what();

	BOOST_CHECK(what == "Failure 0" || what == "Failure 1");
}