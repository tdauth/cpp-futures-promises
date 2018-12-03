#define BOOST_TEST_MODULE AdvancedFollyFutureTest
#include <boost/test/unit_test.hpp>

#if not defined(BOOST_TEST_DYN_LINK) and not defined(WINDOWS)
#error Define BOOST_TEST_DYN_LINK for proper definition of main function.
#endif

#include <folly/init/Init.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

#include "folly_fixture.h"
#include "advanced_futures_folly.h"

struct Fixture
{
	// Use only one thread to detect blocking operations.
	Fixture() : cpuExecutor(new folly::CPUThreadPoolExecutor(1)), ex(new adv_folly::Executor(cpuExecutor))
	{
	}

	~Fixture()
	{
		delete ex;
		ex = nullptr;
		delete cpuExecutor;
		cpuExecutor = nullptr;
	}

	folly::CPUThreadPoolExecutor *cpuExecutor;
	adv_folly::Executor *ex;
};

BOOST_FIXTURE_TEST_CASE(TryNotInitialized, Fixture)
{
	adv_folly::Try<int> t;
	BOOST_REQUIRE(!t.hasValue());
	BOOST_REQUIRE(!t.hasException());
	BOOST_CHECK_THROW(std::move(t).get(), adv::UsingUninitializedTry);
}

BOOST_FIXTURE_TEST_CASE(TryRuntimeError, Fixture)
{
	adv_folly::Try<int> t(std::make_exception_ptr(std::runtime_error("Error")));
	BOOST_REQUIRE(t.hasException());
	BOOST_CHECK_THROW(std::move(t).get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(TryValue, Fixture)
{
	adv_folly::Try<int> t(10);
	BOOST_REQUIRE(t.hasValue());
	BOOST_CHECK_EQUAL(10, std::move(t).get());
}

BOOST_FIXTURE_TEST_CASE(OnComplete, Fixture)
{
	int v = 5;
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	);
	f0.onComplete([&v] (adv_folly::Try<int> t)
		{
			v = std::move(t).get();
		}
	);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	BOOST_CHECK_EQUAL(10, v);
}

BOOST_FIXTURE_TEST_CASE(Get, Fixture)
{
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	);

	BOOST_CHECK_EQUAL(10, std::move(f1).get());
}

BOOST_FIXTURE_TEST_CASE(IsReady, Fixture)
{
	folly::InlineExecutor inlineExecutor;
	adv_folly::Executor ex(&inlineExecutor);
	adv_folly::Future<int> f1 = adv_folly::async(&ex, [] ()
		{
			return 10;
		}
	);

	BOOST_CHECK(f1.isReady());
}

BOOST_FIXTURE_TEST_CASE(Guard, Fixture)
{
	adv_folly::Future<int> f = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_FIXTURE_TEST_CASE(GuardFails, Fixture)
{
	adv_folly::Future<int> f = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v != 10; });

	BOOST_CHECK_THROW(std::move(f).get(), adv::PredicateNotFulfilled);
}

BOOST_FIXTURE_TEST_CASE(Then, Fixture)
{
	adv_folly::Future<std::string> f3 = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).then([] (adv_folly::Try<int> t)
		{
			if (t.hasValue())
			{
				return std::to_string(std::move(t).get());
			}

			return std::string("Failure!");
		}
	);

	BOOST_CHECK_EQUAL("10", std::move(f3).get());
}

BOOST_FIXTURE_TEST_CASE(ThenWith, Fixture)
{
	adv_folly::Future<std::string> f1 = adv_folly::async(ex, [] ()
		{
			return std::string("11");
		}
	);
	adv_folly::Future<std::string> f2 = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).thenWith([f1 = std::move(f1)] (adv_folly::Try<int>&&) mutable
		{
			// TODO Why do we have to move it explicitly here and cannot simply return f1?
			return adv_folly::Future<std::string>(std::move(f1));
		}
	);

	BOOST_CHECK_EQUAL("11", std::move(f2).get());
}

// Important for orElse.
BOOST_FIXTURE_TEST_CASE(ThenWithItself, Fixture)
{
	adv_folly::Future<std::string> f1 = adv_folly::async(ex, [] ()
		{
			return std::string("11");
		}
	);
	auto ctx = std::make_shared<adv_folly::Future<std::string>>(std::move(f1));
	adv_folly::Future<std::string> f2 = ctx->thenWith([ctx] (adv_folly::Try<std::string>&&) mutable
		{
			// TODO Why do we have to move it explicitly here and cannot simply return f1?
			return adv_folly::Future<std::string>(std::move(*ctx));
		}
	);

	// TODO Throws a BrokenPromise exception. Why?
	BOOST_CHECK_EQUAL("11", std::move(f2).get());
}

BOOST_FIXTURE_TEST_CASE(OrElseFirstSuccessful, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { return 10; });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { return 11; });
	auto f2 = std::move(f0).orElse(std::move(f1));

	BOOST_CHECK_EQUAL(10, std::move(f2).get());
}

BOOST_FIXTURE_TEST_CASE(OrElseSecondSuccessful, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { return 10; throw std::runtime_error("Failure!"); });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { return 11; });
	auto f2 = std::move(f0).orElse(std::move(f1));

	BOOST_CHECK_EQUAL(11, std::move(f2).get());
}

BOOST_FIXTURE_TEST_CASE(OrElseBothFail, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { return 10; throw std::runtime_error("Failure 0!"); });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { return 11; throw std::runtime_error("Failure 1!"); });
	auto f2 = std::move(f0).orElse(std::move(f1));

	BOOST_CHECK_THROW(std::move(f2).get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(First, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { return 10; });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));
	auto r = std::move(f2).get();

	BOOST_CHECK(r == 10 || r == 11);
}

BOOST_FIXTURE_TEST_CASE(FirstWithException, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 11; });
	auto f2 = f0.first(std::move(f1));

	BOOST_CHECK_THROW(std::move(f2).get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(FirstSucc, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { return 10; });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	auto r = std::move(f2).get();

	BOOST_CHECK(r == 10 || r == 11);
}

BOOST_FIXTURE_TEST_CASE(FirstSuccWithException, Fixture)
{
	adv_folly::Future<int> f0 = adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });
	adv_folly::Future<int> f1 = adv_folly::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	auto r = std::move(f2).get();

	BOOST_CHECK_EQUAL(11, r);
}

BOOST_FIXTURE_TEST_CASE(FirstN, Fixture)
{
	std::vector<adv_folly::Future<int>> futures;
	futures.push_back(adv_folly::async(ex, [] () { return 10; }));
	futures.push_back(adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 11; }));
	futures.push_back(adv_folly::async(ex, [] () { return 12; }));
	futures.push_back(adv_folly::async(ex, [] () { return 13; }));

	adv_folly::Future<std::vector<std::pair<std::size_t, adv_folly::Try<int>>>> f = adv_folly::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv_folly::Try<int>>> v = std::move(f).get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, Fixture)
{
	std::vector<adv_folly::Future<int>> futures;
	futures.push_back(adv_folly::async(ex, [] () { return 1; }));
	futures.push_back(adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 2; }));
	futures.push_back(adv_folly::async(ex, [] () { return 3; }));
	futures.push_back(adv_folly::async(ex, [] () { return 4; }));

	adv_folly::Future<std::vector<std::pair<std::size_t, int>>> f = adv_folly::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = std::move(f).get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements 1, 3 and 4
}

BOOST_FIXTURE_TEST_CASE(BrokenPromise, Fixture)
{
	adv_folly::Promise<int> *p = new adv_folly::Promise<int>();
	adv_folly::Future<int> f = p->future();
	delete p;
	p = nullptr;

	BOOST_CHECK_THROW(std::move(f).get(), adv::BrokenPromise);
}

BOOST_FIXTURE_TEST_CASE(TryComplete, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();

	bool result = p.tryComplete(adv_folly::Try<int>(10));

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccess, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_FIXTURE_TEST_CASE(TryFailure, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();

	bool result = p.tryFailure(std::runtime_error("Failure!"));

	BOOST_CHECK(result);

	try
	{
		std::move(f).get();
		BOOST_FAIL("Expected exception");
	}
	catch (const std::exception &e)
	{
		BOOST_CHECK_EQUAL("Failure!", e.what());
	}
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWith, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture = adv_folly::async(ex, [] () { return 10; });

	std::move(p).tryCompleteWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithDelayed, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture = adv_folly::Future<folly::Unit>(folly::futures::sleep(folly::Duration(1000))).then([] (adv_folly::Try<folly::Unit> &&t) { return 10; });

	std::move(p).tryCompleteWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithFailure, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture = adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

	std::move(p).tryCompleteWith(std::move(completingFuture));

	BOOST_CHECK_THROW(std::move(f).get(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWith, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture = adv_folly::async(ex, [] () { return 10; });

	std::move(p).trySuccessWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, std::move(f).get());
}

BOOST_FIXTURE_TEST_CASE(TryFailureWith, Fixture)
{
	adv_folly::Promise<int> p;
	adv_folly::Future<int> f = p.future();
	adv_folly::Future<int> completingFuture = adv_folly::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

	std::move(p).tryFailureWith(std::move(completingFuture));

	try
	{
		std::move(f).get();
		BOOST_FAIL("Expected exception");
	}
	catch (const std::exception &e)
	{
		BOOST_CHECK_EQUAL("Failure!", e.what());
	}
}
