#define BOOST_TEST_MODULE AdvancedBoostFutureTest
#include <boost/test/unit_test.hpp>

#if not defined(BOOST_TEST_DYN_LINK) and not defined(WINDOWS)
#error Define BOOST_TEST_DYN_LINK for proper definition of main function.
#endif

#include <iostream>

#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/thread/executors/inline_executor.hpp>

#include "advanced_futures_boost.h"

struct BoostFixture
{
	typedef boost::basic_thread_pool Ex;

	BoostFixture() : ex(new adv_boost::Executor<Ex>(&ex_boost))
	{
	}

	~BoostFixture()
	{
		delete ex;
		ex = nullptr;
	}

	Ex ex_boost;
	adv_boost::Executor<Ex> *ex;
};

BOOST_FIXTURE_TEST_CASE(TryNotInitialized, BoostFixture)
{
	adv_boost::Try<int> t;
	BOOST_REQUIRE(!t.hasValue());
	BOOST_REQUIRE(!t.hasException());
	BOOST_CHECK_THROW(t.get(), adv::UsingUninitializedTry);
}

BOOST_FIXTURE_TEST_CASE(TryRuntimeError, BoostFixture)
{
	adv_boost::Try<int> t(boost::copy_exception(std::runtime_error("Error")));
	BOOST_REQUIRE(t.hasException());
	BOOST_CHECK_THROW(t.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(TryValue, BoostFixture)
{
	adv_boost::Try<int> t(10);
	BOOST_REQUIRE(t.hasValue());
	BOOST_CHECK_EQUAL(10, t.get());
}

BOOST_FIXTURE_TEST_CASE(OnComplete, BoostFixture)
{
	volatile int v = 5;
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	);
	f0.onComplete([&v] (adv_boost::Try<int> t)
		{
			v = t.get();
		}
	);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	BOOST_CHECK_EQUAL(10, v);
}

BOOST_FIXTURE_TEST_CASE(Get, BoostFixture)
{
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	);

	BOOST_CHECK_EQUAL(10, f1.get());
}

BOOST_FIXTURE_TEST_CASE(IsReady, BoostFixture)
{
	boost::inline_executor inlineExecutor;
	adv_boost::Executor<boost::inline_executor> ex(&inlineExecutor);
	adv_boost::Future<int> f1 = adv_boost::async(&ex, [] ()
		{
			return 10;
		}
	);

	BOOST_CHECK(f1.isReady());
}

BOOST_FIXTURE_TEST_CASE(Guard, BoostFixture)
{
	adv_boost::Future<int> f = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(GuardFails, BoostFixture)
{
	adv_boost::Future<int> f = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v != 10; });

	BOOST_CHECK_THROW(f.get(), adv::PredicateNotFulfilled);
}

BOOST_FIXTURE_TEST_CASE(Then, BoostFixture)
{
	adv_boost::Future<std::string> f3 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	).then([] (adv_boost::Try<int> t)
		{
			if (t.hasValue())
			{
				return std::to_string(t.get());
			}

			return std::string("Failure!");
		}
	);

	BOOST_CHECK_EQUAL("10", f3.get());
}

BOOST_FIXTURE_TEST_CASE(OrElse, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(First, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));
	auto r = f2.get();

	BOOST_CHECK(r == 10 || r == 11);
}

BOOST_FIXTURE_TEST_CASE(FirstWithException, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { throw std::runtime_error("Failure!"); return 11; });
	auto f2 = f0.first(std::move(f1));

	BOOST_CHECK_THROW(f2.get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(FirstSucc, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	auto r = f2.get();

	BOOST_CHECK(r == 10 || r == 11);
}

BOOST_FIXTURE_TEST_CASE(FirstSuccWithException, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	auto r = f2.get();

	BOOST_CHECK_EQUAL(11, r);
}

BOOST_FIXTURE_TEST_CASE(FirstN, BoostFixture)
{
	std::vector<adv_boost::Future<int>> futures;
	futures.push_back(adv_boost::async(ex, [] () { return 10; }));
	futures.push_back(adv_boost::async(ex, [] () { throw std::runtime_error("Failure!"); return 11; }));
	futures.push_back(adv_boost::async(ex, [] () { return 12; }));
	futures.push_back(adv_boost::async(ex, [] () { return 13; }));

	adv_boost::Future<std::vector<std::pair<std::size_t, adv_boost::Try<int>>>> f = adv_boost::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv_boost::Try<int>>> v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, BoostFixture)
{
	std::vector<adv_boost::Future<int>> futures;
	futures.push_back(adv_boost::async(ex, [] () { return 1; }));
	futures.push_back(adv_boost::async(ex, [] () { throw std::runtime_error("Failure!"); return 2; }));
	futures.push_back(adv_boost::async(ex, [] () { return 3; }));
	futures.push_back(adv_boost::async(ex, [] () { return 4; }));

	adv_boost::Future<std::vector<std::pair<std::size_t, int>>> f = adv_boost::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements 1, 3 and 4
	// TODO sometimes this test fails with a number of only 2 elements. Due to the exception?
}

BOOST_FIXTURE_TEST_CASE(BrokenPromise, BoostFixture)
{
	adv_boost::Promise<int> *p = new adv_boost::Promise<int>();
	adv_boost::Future<int> f = p->future();
	delete p;
	p = nullptr;

	BOOST_CHECK_THROW(f.get(), adv::BrokenPromise);
}

BOOST_FIXTURE_TEST_CASE(TryComplete, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();

	bool result = p.tryComplete(adv_boost::Try<int>(10));

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccess, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailure, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();

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

BOOST_FIXTURE_TEST_CASE(TryCompleteWith, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();
	adv_boost::Future<int> completingFuture = adv_boost::async(ex, [] () { return 10; });

	p.tryCompleteWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWith, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();
	adv_boost::Future<int> completingFuture = adv_boost::async(ex, [] () { return 10; });

	p.trySuccessWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailureWith, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();
	adv_boost::Future<int> completingFuture = adv_boost::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

	p.tryFailureWith(std::move(completingFuture));

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
