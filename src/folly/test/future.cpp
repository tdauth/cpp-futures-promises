#define BOOST_TEST_MODULE AdvancedFollyFutureTest
#include <boost/test/unit_test.hpp>

#if not defined(BOOST_TEST_DYN_LINK) and not defined(WINDOWS)
#error Define BOOST_TEST_DYN_LINK for proper definition of main function.
#endif

#include <atomic>

#include <folly/init/Init.h>

//#include "../../fixture.h"
#include "../../future.h"
#include "../../future_impl.h"
#include "../../test_fixture.h"
#include "../future.h"
#include "../future_impl.h"
#include "../promise.h"
/*
BOOST_FIXTURE_TEST_CASE(TryNotInitialized, adv::TestFixture)
{
 adv::Try<int> t;
 BOOST_REQUIRE(!t.hasValue());
 BOOST_REQUIRE(!t.hasException());
 BOOST_CHECK_THROW(t.get(), adv::UsingUninitializedTry);
}

BOOST_FIXTURE_TEST_CASE(TryRuntimeError, adv::TestFixture)
{
 adv::Try<int> t(std::make_exception_ptr(std::runtime_error("Error")));
 BOOST_REQUIRE(t.hasException());
 BOOST_CHECK_THROW(t.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(TryValue, adv::TestFixture)
{
 adv::Try<int> t(10);
 BOOST_REQUIRE(t.hasValue());
 BOOST_CHECK_EQUAL(10, t.get());
}

BOOST_FIXTURE_TEST_CASE(FutureFromFollyFuture, adv::TestFixture)
{
 folly::Future<int> f1 = folly::makeFuture(10);
 adv_folly::Future<int> f2(std::move(f1));

 BOOST_REQUIRE(f2.isReady());
 BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OnComplete, adv::TestFixture)
{
 std::string v = "5";

 adv_folly::Promise<std::string> p = folly::Promise<std::string>();
 p.trySuccess("10");
 adv_folly::Future<std::string> f0(p.future());
 f0.onComplete([&v](adv::Try<std::string> &&t) { v = t.get(); });

 BOOST_CHECK_EQUAL("10", v);

 // multiple callbacks are not allowed
 BOOST_CHECK_THROW(f0.onComplete([](adv::Try<std::string> &&t) {}),
                   adv::OnlyOneCallbackPerFuture);
}
*/

BOOST_FIXTURE_TEST_CASE(Get, adv::TestFixture)
{
	adv_folly::Future<int> f =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; });

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(IsReady, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	p.trySuccess(10);
	adv_folly::Future<int> f1 = p.future();

	BOOST_CHECK(f1.isReady());
}

BOOST_FIXTURE_TEST_CASE(Then, adv::TestFixture)
{
	adv_folly::Future<std::string> f3 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; })
	        .then([](adv::Try<int> t) {
		        if (t.hasValue())
		        {
			        return std::to_string(t.get());
		        }

		        return std::string("Failure!");
	        });

	BOOST_CHECK_EQUAL("10", f3.get());
}

BOOST_FIXTURE_TEST_CASE(ThenWith, adv::TestFixture)
{
	adv_folly::Future<std::string> f1 =
	    adv::async<adv_folly::Promise<std::string>>(
	        ex, []() { return std::string("11"); });
	adv_folly::Future<std::string> f2 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; })
	        .thenWith([f1 = std::move(f1)](adv::Try<int> &&) mutable {
		        // TODO Why do we have to move it explicitly here and cannot simply
		        // return f1?
		        return adv_folly::Future<std::string>(std::move(f1));
	        });

	BOOST_CHECK_EQUAL("11", f2.get());
}

BOOST_FIXTURE_TEST_CASE(Guard, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	p.trySuccess(10);
	adv_folly::Future<int> f =
	    p.future().guard([](const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(GuardFails, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	p.trySuccess(10);
	adv_folly::Future<int> f =
	    p.future().guard([](const int &v) { return v != 10; });

	BOOST_CHECK_THROW(f.get(), adv::PredicateNotFulfilled);
}

BOOST_FIXTURE_TEST_CASE(OrElseFirstSuccessful, adv::TestFixture)
{
	adv_folly::Future<int> f0 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; });
	adv_folly::Future<int> f1 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 11; });
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OrElseSecondSuccessful, adv::TestFixture)
{
	adv_folly::Future<int> f0 = adv::async<adv_folly::Promise<int>>(ex, []() {
		throw std::runtime_error("Failure!");
		return 10;
	});
	adv_folly::Future<int> f1 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 11; });
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(11, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OrElseBothFail, adv::TestFixture)
{
	adv_folly::Future<int> f0 = adv::async<adv_folly::Promise<int>>(ex, []() {
		throw std::runtime_error("Failure 0!");
		return 10;
	});
	adv_folly::Future<int> f1 = adv::async<adv_folly::Promise<int>>(ex, []() {
		throw std::runtime_error("Failure 1!");
		return 11;
	});
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_THROW(f2.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(First, adv::TestFixture)
{
	adv_folly::Future<int> f0 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; });
	adv_folly::Future<int> f1 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 11; });
	auto f2 = f0.first(f1);
	auto r = f2.get();

	BOOST_CHECK(r == 10 || r == 11);
}

BOOST_FIXTURE_TEST_CASE(FirstWithException, adv::TestFixture)
{
	adv_folly::Future<int> f0 = adv::async<adv_folly::Promise<int>>(ex, []() {
		throw std::runtime_error("Failure!");
		return 10;
	});
	adv_folly::Future<int> f1 = adv::async<adv_folly::Promise<int>>(ex, []() {
		throw std::runtime_error("Failure!");
		return 11;
	});
	auto f2 = f0.first(f1);

	BOOST_CHECK_THROW(f2.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(FirstSucc, adv::TestFixture)
{
	adv_folly::Future<int> f0 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; });
	adv_folly::Future<int> f1 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 11; });
	auto f2 = f0.firstSucc(f1);
	auto r = f2.get();

	BOOST_CHECK(r == 10 || r == 11);
}

BOOST_FIXTURE_TEST_CASE(FirstSuccWithException, adv::TestFixture)
{
	adv_folly::Future<int> f0 = adv::async<adv_folly::Promise<int>>(ex, []() {
		throw std::runtime_error("Failure!");
		return 10;
	});
	adv_folly::Future<int> f1 =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 11; });
	auto f2 = f0.firstSucc(f1);
	auto r = f2.get();

	BOOST_CHECK_EQUAL(11, r);
}

/*
BOOST_FIXTURE_TEST_CASE(FirstN, adv::TestFixture)
{
 std::vector<adv_folly::Future<int>> futures;
 futures.push_back(
     adv::async<adv_folly::Promise<int>>(ex, []() { return 10; }));
 futures.push_back(adv::async<adv_folly::Promise<int>>(ex, []() {
  throw std::runtime_error("Failure!");
  return 11;
 }));
 futures.push_back(
     adv::async<adv_folly::Promise<int>>(ex, []() { return 12; }));
 futures.push_back(
     adv::async<adv_folly::Promise<int>>(ex, []() { return 13; }));

 adv_folly::Future<std::vector<std::pair<std::size_t, adv::Try<int>>>> f =
     adv::firstN<adv_folly::Promise<std::vector<std::pair<std::size_t,
adv::Try<int>>>>>(std::move(futures), 3); std::vector<std::pair<std::size_t,
adv::Try<int>>> v = f.get();

 BOOST_CHECK_EQUAL(3u, v.size());
 // TODO check for elements
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, adv::TestFixture)
{
 std::vector<adv_folly::Future<int>> futures;
 futures.push_back(adv::async<adv_folly::Promise<int>>(ex, []() { return 1; }));
 futures.push_back(adv::async<adv_folly::Promise<int>>(ex, []() {
  throw std::runtime_error("Failure!");
  return 2;
 }));
 futures.push_back(adv::async<adv_folly::Promise<int>>(ex, []() { return 3; }));
 futures.push_back(adv::async<adv_folly::Promise<int>>(ex, []() { return 4; }));

 adv_folly::Future<std::vector<std::pair<std::size_t, int>>> f =
     adv::firstNSucc<adv_folly::Promise<std::vector<std::pair<std::size_t,
int>>>>(std::move(futures), 3); std::vector<std::pair<std::size_t, int>> v =
f.get();

 BOOST_CHECK_EQUAL(3u, v.size());
 // TODO check for elements 1, 3 and 4
}
 */

BOOST_FIXTURE_TEST_CASE(BrokenPromise, adv::TestFixture)
{
	adv_folly::Promise<int> *p = new adv_folly::Promise<int>();
	adv_folly::Future<int> f = p->future();
	delete p;
	p = nullptr;

	BOOST_CHECK_THROW(f.get(), adv::BrokenPromise);
}

BOOST_FIXTURE_TEST_CASE(TryComplete, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();

	bool result = p.tryComplete(adv::Try<int>(10));

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccess, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailure, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();

	bool result = p.tryFailure(std::runtime_error("Failure!"));

	BOOST_CHECK(result);

	try
	{
		f.get();
		BOOST_FAIL("Expected exception");
	}
	catch (const std::exception &e)
	{
		BOOST_CHECK_EQUAL("Failure!", e.what());
	}
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWith, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; });

	std::move(p).tryCompleteWith(completingFuture);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithDelayed, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture =
	    adv_folly::Future<folly::Unit>(
	        folly::futures::sleep(folly::Duration(1000)))
	        .then([](adv::Try<folly::Unit> &&t) { return 10; });

	std::move(p).tryCompleteWith(completingFuture);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithFailure, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture =
	    adv::async<adv_folly::Promise<int>>(ex, []() {
		    throw std::runtime_error("Failure!");
		    return 10;
	    });

	std::move(p).tryCompleteWith(completingFuture);

	BOOST_CHECK_THROW(f.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWith, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture =
	    adv::async<adv_folly::Promise<int>>(ex, []() { return 10; });

	std::move(p).trySuccessWith(completingFuture);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailureWith, adv::TestFixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture =
	    adv::async<adv_folly::Promise<int>>(ex, []() {
		    throw std::runtime_error("Failure!");
		    return 10;
	    });

	std::move(p).tryFailureWith(completingFuture);

	try
	{
		f.get();
		BOOST_FAIL("Expected exception");
	}
	catch (const std::exception &e)
	{
		BOOST_CHECK_EQUAL("Failure!", e.what());
	}
}