#define BOOST_TEST_MODULE AdvancedFollySharedFutureTest
#include <boost/test/unit_test.hpp>

#if not defined(BOOST_TEST_DYN_LINK) and not defined(WINDOWS)
#error Define BOOST_TEST_DYN_LINK for proper definition of main function.
#endif

#include <iostream>

#include <folly/init/Init.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

#include "folly_fixture.h"
#include "advanced_futures_folly.h"

struct Fixture
{
	Fixture() : cpuExecutor(new folly::CPUThreadPoolExecutor(std::thread::hardware_concurrency())), ex(new adv_folly::Executor(cpuExecutor))
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

BOOST_FIXTURE_TEST_CASE(MultipleRead, Fixture)
{
	adv_folly::Future<std::string> uniqueF(folly::makeFuture<std::string>("My test value"));
	/*
	 * A shared future can be created from a non-shared future.
	 */
	adv_folly::SharedFuture<std::string> f = uniqueF.share();

	/*
	 * It can be copied around, for example to multiple threads.
	 */
	std::thread t0([f] () mutable {
		std::cerr << "Result 0: " << f.get() << std::endl;
	});

	std::thread t1([f] () mutable {
		std::cerr << "Result 1: " << f.get() << std::endl;
	});

	std::thread t2([f] () mutable {
		std::cerr << "Result 2: " << f.get() << std::endl;
	});

	std::thread t3([f] () mutable {
		std::cerr << "Result 3: " << f.get() << std::endl;
	});

	t0.join();
	t1.join();
	t2.join();
	t3.join();

	/*
	 * It allows reading its value multiple times.
	 */
	BOOST_CHECK_EQUAL("My test value", f.get());
	BOOST_CHECK_EQUAL("My test value", f.get());
}

BOOST_FIXTURE_TEST_CASE(OnComplete, Fixture)
{
	adv_folly::SharedFuture<int> f = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).share();

	volatile int v = 0;

	/*
	 * Shared futures allow multiple calls:
	 */
	for (int i = 0; i < 10; ++i)
	{
		f.onComplete([&v] (adv_folly::Try<int> t)
			{
				v += 1;
			}
		);
	}

	f.get();

	BOOST_CHECK_EQUAL(10, v);
}

BOOST_FIXTURE_TEST_CASE(GetAndIsReady, Fixture)
{
	adv_folly::SharedFuture<int> f1 = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).share();

	BOOST_CHECK_EQUAL(10, f1.get());
	BOOST_CHECK_EQUAL(10, f1.get());
	BOOST_CHECK(f1.isReady());
}

BOOST_FIXTURE_TEST_CASE(Guard, Fixture)
{
	adv_folly::SharedFuture<int> f2 = adv_folly::async(ex, [] ()
		{
			return 10;
		}
	).share().guard([] (const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(Then, Fixture)
{
	adv_folly::SharedFuture<int> f = adv_folly::SharedFuture<int>(adv_folly::async(ex, [] ()
		{
			return 10;
		}
	));

	/*
	 * A share future allows multiple registrations of callbacks.
	 */
	std::vector<adv_folly::SharedFuture<std::string>> futures;

	for (int i = 0; i < 10; ++i)
	{
		futures.push_back(
			f.then([] (adv_folly::Try<int> t)
				{
					if (t.hasValue())
					{
						return std::to_string(t.get());
					}

					return std::string("Failure!");
				}
			)
		);
	}

	for (std::size_t i = 0; i < futures.size(); ++i)
	{
		BOOST_CHECK_EQUAL("10", futures[i].get());
	}
}

BOOST_FIXTURE_TEST_CASE(OrElse, Fixture)
{
	adv_folly::SharedFuture<int> f0 = adv_folly::async(ex, [] { throw std::runtime_error("Error"); return 10; }).share();
	adv_folly::SharedFuture<int> f1 = adv_folly::async(ex, [] { return 11; }).share();
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(11, f2.get());
}

BOOST_FIXTURE_TEST_CASE(First, Fixture)
{
	adv_folly::SharedFuture<int> f0 = adv_folly::async(ex, [] () { std::this_thread::sleep_for(std::chrono::seconds(5)); return 10; }).share();
	adv_folly::SharedFuture<int> f1 = adv_folly::async(ex, [] () { return 11; }).share();
	auto f2 = f0.first(std::move(f1));

	BOOST_CHECK_EQUAL(11, f2.get());
}


BOOST_FIXTURE_TEST_CASE(FirstSucc, Fixture)
{
	adv_folly::SharedFuture<int> f0 = adv_folly::async(ex, [] { throw std::runtime_error("Error"); return 10; }).share();
	adv_folly::SharedFuture<int> f1 = adv_folly::async(ex, [] { return 11; }).share();
	auto f2 = f0.firstSucc(std::move(f1));

	BOOST_CHECK_EQUAL(11, f2.get());
}

BOOST_FIXTURE_TEST_CASE(FirstN, Fixture)
{
	std::vector<adv_folly::SharedFuture<int>> futures;

	for (int i = 0; i < 10; ++i)
	{
		futures.push_back(adv_folly::SharedFuture<int>(adv_folly::async(ex, [] () { return 10; })));
	}

	adv_folly::SharedFuture<std::vector<std::pair<std::size_t, adv_folly::Try<int>>>> f = adv_folly::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv_folly::Try<int>>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		BOOST_CHECK(v[i].first >= 0 && v[i].first <= 9);
		BOOST_CHECK_EQUAL(10, v[i].second.get());
	}
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, Fixture)
{
	std::vector<adv_folly::SharedFuture<int>> futures;

	for (int i = 0; i < 10; ++i)
	{
		futures.push_back(adv_folly::SharedFuture<int>(adv_folly::async(ex, [] () { return 10; })));
	}

	adv_folly::SharedFuture<std::vector<std::pair<std::size_t, int>>> f = adv_folly::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		BOOST_CHECK(v[i].first >= 0 && v[i].first <= 9);
		BOOST_CHECK_EQUAL(10, v[i].second);
	}
}
