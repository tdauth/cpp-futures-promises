#define BOOST_TEST_MODULE AdvancedBoostFutureTest

#include <atomic>
#include <boost/test/unit_test.hpp>

#include "../../future.h"
#include "../../future_impl.h"
#include "../../test_fixture.h"
#include "../future.h"
#include "../future_impl.h"
#include "../promise.h"

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

BOOST_FIXTURE_TEST_CASE(FutureFromBoostFuture, adv::TestFixture)
{
	boost::future<int> f1 = boost::make_ready_future(10);
	adv_boost::Future<int> f2(ex, std::move(f1));

	BOOST_REQUIRE(f2.isReady());
	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OnComplete, adv::TestFixture)
{
	std::string v = "5";
	adv_boost::Promise<std::string> p(ex);
	p.trySuccess("10");
	auto f = p.future();
	f.onComplete([&v](adv::Try<std::string> &&t) { v = t.get(); });

	BOOST_CHECK_EQUAL("10", v);

	// multiple callbacks are not allowed
	BOOST_CHECK_THROW(f.onComplete([](adv::Try<std::string> &&t) {}),
	                  adv::OnlyOneCallbackPerFuture);
}

BOOST_FIXTURE_TEST_CASE(OnSuccess, adv::TestFixture)
{
	std::string v = "5";
	adv_boost::Promise<std::string> p(ex);
	p.trySuccess("10");
	auto f = p.future();
	f.onSuccess([&v](std::string &&t) { v = std::move(t); });

	BOOST_REQUIRE(f.isReady());
	BOOST_CHECK_EQUAL("10", v);
}

BOOST_FIXTURE_TEST_CASE(OnFailure, adv::TestFixture)
{
	std::optional<adv::Try<std::string>> t;
	adv_boost::Promise<std::string> p(ex);
	p.tryFailure(std::runtime_error("Failure!"));
	auto f = p.future();
	f.onFailure([&t](std::exception_ptr &&e) {
		t.emplace(adv::Try<std::string>(std::move(e)));
	});

	BOOST_REQUIRE(f.isReady());
	BOOST_CHECK(t.has_value());
	auto r = std::move(t.value());
	BOOST_CHECK(r.hasException());

	try
	{
		r.get();
		BOOST_FAIL("Expected exception.");
	}
	catch (std::runtime_error &e)
	{
		BOOST_CHECK_EQUAL("Failure!", e.what());
	}
}

BOOST_FIXTURE_TEST_CASE(Get, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	p.trySuccess(10);
	auto f = p.future();

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(IsReady, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	p.trySuccess(10);
	auto f = p.future();

	BOOST_CHECK(f.isReady());
}

BOOST_FIXTURE_TEST_CASE(Then, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	p.trySuccess(10);
	auto f = p.future().then([](adv::Try<int> &&t) {
		if (t.hasValue())
		{
			return std::to_string(t.get());
		}

		return std::string("Failure!");
	});

	BOOST_CHECK_EQUAL("10", f.get());
}

BOOST_FIXTURE_TEST_CASE(ThenWith, adv::TestFixture)
{
	adv_boost::Promise<std::string> p0(ex);
	p0.trySuccess("11");
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.trySuccess(10);
	auto f1 = p1.future();
	auto f = f1.thenWith([f0 = std::move(f0)](adv::Try<int> &&) mutable {
		return std::move(f0);
	});

	BOOST_CHECK_EQUAL("11", f.get());
}

BOOST_FIXTURE_TEST_CASE(Guard, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	p.trySuccess(10);
	auto f = p.future().guard([](const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(GuardFails, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	p.trySuccess(10);
	auto f = p.future().guard([](const int &v) { return v != 10; });

	BOOST_CHECK_THROW(f.get(), adv::PredicateNotFulfilled);
}

BOOST_FIXTURE_TEST_CASE(OrElseFirstSuccessful, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.trySuccess(10);
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.trySuccess(11);
	auto f1 = p1.future();
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OrElseSecondSuccessful, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.tryFailure(std::runtime_error("Failure!"));
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.trySuccess(11);
	auto f1 = p1.future();
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(11, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OrElseBothFail, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.tryFailure(std::runtime_error("Failure 0!"));
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.tryFailure(std::runtime_error("Failure 1!"));
	auto f1 = p1.future();
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_THROW(f2.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(First, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.trySuccess(10);
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.trySuccess(11);
	auto f1 = p1.future();
	auto f2 = f0.first(f1);
	auto r = f2.get();

	BOOST_CHECK_EQUAL(10, r);
}

BOOST_FIXTURE_TEST_CASE(FirstWithException, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.tryFailure(std::runtime_error("Failure 0!"));
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.tryFailure(std::runtime_error("Failure 1!"));
	auto f1 = p1.future();
	auto f2 = f0.first(f1);

	try
	{
		f2.get();
		BOOST_FAIL("Expecting exception.");
	}
	catch (const std::runtime_error &e)
	{
		BOOST_CHECK_EQUAL("Failure 0!", e.what());
	}
}

BOOST_FIXTURE_TEST_CASE(FirstSucc, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.trySuccess(10);
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.trySuccess(11);
	auto f1 = p1.future();
	auto f2 = f0.firstSucc(f1);
	auto r = f2.get();

	BOOST_CHECK_EQUAL(10, r);
}

BOOST_FIXTURE_TEST_CASE(FirstSuccWithException, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.tryFailure(std::runtime_error("Failure 0!"));
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.trySuccess(11);
	auto f1 = p1.future();
	auto f2 = f0.firstSucc(f1);
	auto r = f2.get();

	BOOST_CHECK_EQUAL(11, r);
}

BOOST_FIXTURE_TEST_CASE(FirstSuccBothFail, adv::TestFixture)
{
	adv_boost::Promise<int> p0(ex);
	p0.tryFailure(std::runtime_error("Failure 0!"));
	auto f0 = p0.future();
	adv_boost::Promise<int> p1(ex);
	p1.tryFailure(std::runtime_error("Failure 1!"));
	auto f1 = p1.future();
	auto f2 = f0.firstSucc(f1);

	try
	{
		f2.get();
		BOOST_FAIL("Expecting exception.");
	}
	catch (const adv::BrokenPromise &e)
	{
	}
}

BOOST_FIXTURE_TEST_CASE(Successful, adv::TestFixture)
{
	auto f = adv_boost::Future<int>::successful(ex, 10);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(Failed, adv::TestFixture)
{
	auto f = adv_boost::Future<int>::failed(ex, std::runtime_error("Failure!"));
	BOOST_CHECK_THROW(f.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(Async, adv::TestFixture)
{
	auto f = adv::async<adv_boost::Promise<int>>(ex, []() { return 10; });
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(FirstN, adv::TestFixture)
{
	std::vector<adv_boost::Future<int>> futures;
	futures.push_back(adv_boost::Future<int>::successful(ex, 10));
	futures.push_back(
	    adv_boost::Future<int>::failed(ex, std::runtime_error("Failure!")));
	futures.push_back(adv_boost::Future<int>::successful(ex, 12));
	futures.push_back(adv_boost::Future<int>::successful(ex, 13));

	auto f = adv::firstN(ex, std::move(futures), 3);
	auto v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	auto v0 = std::move(v[0]);
	BOOST_CHECK_EQUAL(0u, v0.first);
	BOOST_CHECK_EQUAL(10, v0.second.get());
	auto v1 = std::move(v[1]);
	BOOST_CHECK_EQUAL(1u, v1.first);
	BOOST_CHECK_THROW(v1.second.get(), std::runtime_error);
	auto v2 = std::move(v[2]);
	BOOST_CHECK_EQUAL(2u, v2.first);
	BOOST_CHECK_EQUAL(12, v2.second.get());
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, adv::TestFixture)
{
	std::vector<adv_boost::Future<int>> futures;
	futures.push_back(adv_boost::Future<int>::successful(ex, 10));
	futures.push_back(
	    adv_boost::Future<int>::failed(ex, std::runtime_error("Failure!")));
	futures.push_back(adv_boost::Future<int>::successful(ex, 12));
	futures.push_back(adv_boost::Future<int>::successful(ex, 13));

	auto f = adv::firstNSucc(ex, std::move(futures), 3);
	auto v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	auto v0 = std::move(v[0]);
	BOOST_CHECK_EQUAL(0u, v0.first);
	BOOST_CHECK_EQUAL(10, v0.second);
	auto v1 = std::move(v[1]);
	BOOST_CHECK_EQUAL(2u, v1.first);
	BOOST_CHECK_EQUAL(12, v1.second);
	auto v2 = std::move(v[2]);
	BOOST_CHECK_EQUAL(3u, v2.first);
	BOOST_CHECK_EQUAL(13, v2.second);
}

BOOST_FIXTURE_TEST_CASE(BrokenPromise, adv::TestFixture)
{
	adv_boost::Promise<int> *p = new adv_boost::Promise<int>(ex);
	adv_boost::Future<int> f = p->future();
	delete p;
	p = nullptr;

	BOOST_CHECK_THROW(f.get(), adv::BrokenPromise);
}

BOOST_FIXTURE_TEST_CASE(TryComplete, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	adv_boost::Future<int> f = p.future();

	bool result = p.tryComplete(adv::Try<int>(10));

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccess, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	adv_boost::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailure, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	auto f = p.future();
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
	adv_boost::Promise<int> p(ex);
	auto f = p.future();
	adv_boost::Promise<int> completingPromise(ex);
	completingPromise.trySuccess(10);
	auto completingFuture = completingPromise.future();
	std::move(p).tryCompleteWith(completingFuture);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithFailure, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	auto f = p.future();
	adv_boost::Promise<int> completingPromise(ex);
	completingPromise.tryFailure(std::runtime_error("Failure!"));
	auto completingFuture = completingPromise.future();

	std::move(p).tryCompleteWith(completingFuture);

	BOOST_CHECK_THROW(f.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWith, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	auto f = p.future();
	adv_boost::Promise<int> completingPromise(ex);
	completingPromise.trySuccess(10);
	auto completingFuture = completingPromise.future();

	std::move(p).trySuccessWith(completingFuture);

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailureWith, adv::TestFixture)
{
	adv_boost::Promise<int> p(ex);
	auto f = p.future();
	adv_boost::Promise<int> completingPromise(ex);
	completingPromise.tryFailure(std::runtime_error("Failure!"));
	auto completingFuture = completingPromise.future();

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