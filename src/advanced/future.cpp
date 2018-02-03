#define BOOST_TEST_MODULE AdvancedFutureTest
#include <boost/test/unit_test.hpp>

#if not defined(BOOST_TEST_DYN_LINK) and not defined(WINDOWS)
#error Define BOOST_TEST_DYN_LINK for proper definition of main function.
#endif

#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include <boost/thread/executors/basic_thread_pool.hpp>

#include "folly_fixture.h"
#include "advanced_futures_folly.h"
#include "advanced_futures_boost.h"

struct Fixture
{
	Fixture() : ex(new adv::Executor(wangle::getCPUExecutor().get()))
	{
	}

	~Fixture()
	{
		delete ex;
		ex = nullptr;
	}

	adv::Executor *ex;
};

struct BoostFixture
{
	BoostFixture() : ex(new adv_boost::Executor(&ex_boost))
	{
	}

	~BoostFixture()
	{
		delete ex;
		ex = nullptr;
	}

	typedef boost::basic_thread_pool Ex;
	Ex ex_boost;
	adv_boost::Executor<Ex> *ex;
};

BOOST_FIXTURE_TEST_CASE(OnComplete, Fixture)
{
	adv::Future<int> f0 = adv::async(ex, [] ()
		{
			return 10;
		}
	);
	f0.onComplete([] (adv::Try<int> t)
		{
			std::cout << "Result: " << t.get() << std::endl;
		}
	);
}

BOOST_FIXTURE_TEST_CASE(OnCompleteBoost, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	);
	f0.onComplete([] (adv_boost::Try<int> t)
		{
			std::cout << "Result: " << t.get() << std::endl;
		}
	);
}

BOOST_FIXTURE_TEST_CASE(GetAndIsReady, Fixture)
{
	adv::Future<int> f1 = adv::async(ex, [] ()
		{
			return 10;
		}
	);

	BOOST_CHECK_EQUAL(10, f1.get());
	BOOST_CHECK(f1.isReady());
}

BOOST_FIXTURE_TEST_CASE(GetAndIsReadyBoost, BoostFixture)
{
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	);

	BOOST_CHECK_EQUAL(10, f1.get());
	BOOST_CHECK(f1.isReady());
}

BOOST_FIXTURE_TEST_CASE(Guard, Fixture)
{
	adv::Future<int> f2 = adv::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(GuardBoost, BoostFixture)
{
	adv_boost::Future<int> f2 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(Then, Fixture)
{
	adv::Future<std::string> f3 = adv::async(ex, [] ()
		{
			return 10;
		}
	).then([] (adv::Try<int> t)
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

BOOST_FIXTURE_TEST_CASE(ThenBoost, BoostFixture)
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

BOOST_FIXTURE_TEST_CASE(OrElse, Fixture)
{
	adv::Future<int> f0 = adv::async(ex, [] () { return 10; });
	adv::Future<int> f1 = adv::async(ex, [] () { return 11; });
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(OrElseBoost, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.orElse(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(First, Fixture)
{
	adv::Future<int> f0 = adv::async(ex, [] () { return 10; });
	adv::Future<int> f1 = adv::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(FirstBoost, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(FirstSucc, Fixture)
{
	adv::Future<int> f0 = adv::async(ex, [] () { return 10; });
	adv::Future<int> f1 = adv::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(FirstSuccBoost, BoostFixture)
{
	adv_boost::Future<int> f0 = adv_boost::async(ex, [] () { return 10; });
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));

	BOOST_CHECK_EQUAL(10, f2.get());
}

BOOST_FIXTURE_TEST_CASE(FirstN, Fixture)
{
	std::vector<adv::Future<int>> futures;
	futures.push_back(adv::async(ex, [] () { return 10; }));
	futures.push_back(adv::async(ex, [] () { return 11; }));
	futures.push_back(adv::async(ex, [] () { return 12; }));
	futures.push_back(adv::async(ex, [] () { return 13; }));

	adv::Future<std::vector<std::pair<std::size_t, adv::Try<int>>>> f = adv::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv::Try<int>>> v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstN: index: " << v[i].first << ", value: " << v[i].second.get() << std::endl;
	}
}

BOOST_FIXTURE_TEST_CASE(FirstNBoost, BoostFixture)
{
	std::vector<adv_boost::Future<int>> futures;
	futures.push_back(adv_boost::async(ex, [] () { return 10; }));
	futures.push_back(adv_boost::async(ex, [] () { return 11; }));
	futures.push_back(adv_boost::async(ex, [] () { return 12; }));
	futures.push_back(adv_boost::async(ex, [] () { return 13; }));

	adv_boost::Future<std::vector<std::pair<std::size_t, adv_boost::Try<int>>>> f = adv_boost::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv_boost::Try<int>>> v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstN: index: " << v[i].first << ", value: " << v[i].second.get() << std::endl;
	}
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, Fixture)
{
	std::vector<adv::Future<int>> futures;
	futures.push_back(adv::async(ex, [] () { return 1; }));
	futures.push_back(adv::async(ex, [] () { throw std::runtime_error("Failure!"); return 2; }));
	futures.push_back(adv::async(ex, [] () { return 3; }));
	futures.push_back(adv::async(ex, [] () { return 4; }));

	adv::Future<std::vector<std::pair<std::size_t, int>>> f = adv::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	BOOST_CHECK_EQUAL(3u, v.size());
	// TODO check for elements 1, 3 and 4

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstNSucc: index: " << v[i].first << ", value: " << v[i].second << std::endl;
	}
}

BOOST_FIXTURE_TEST_CASE(FirstNSuccBoost, BoostFixture)
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

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstNSucc: index: " << v[i].first << ", value: " << v[i].second << std::endl;
	}
}

BOOST_FIXTURE_TEST_CASE(TryComplete, Fixture)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();

	bool result = p.tryComplete(adv::Try<int>(10));

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteBoost, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();

	bool result = p.tryComplete(adv_boost::Try<int>(10));

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccess, Fixture)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccessBoost, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	BOOST_CHECK(result);
	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailure, Fixture)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();

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

BOOST_FIXTURE_TEST_CASE(TryFailureBoost, BoostFixture)
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

BOOST_FIXTURE_TEST_CASE(TryCompleteWith, Fixture)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();
	adv::Future<int> completingFuture = adv::async(ex, [] () { return 10; });

	p.tryCompleteWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithBoost, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();
	adv_boost::Future<int> completingFuture = adv_boost::async(ex, [] () { return 10; });

	p.tryCompleteWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWith, Fixture)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();
	adv::Future<int> completingFuture = adv::async(ex, [] () { return 10; });

	p.trySuccessWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWithBoost, BoostFixture)
{
	adv_boost::Promise<int> p;
	adv_boost::Future<int> f = p.future();
	adv_boost::Future<int> completingFuture = adv_boost::async(ex, [] () { return 10; });

	p.trySuccessWith(std::move(completingFuture));

	BOOST_CHECK_EQUAL(10, f.get());
}

BOOST_FIXTURE_TEST_CASE(TryFailureWith, Fixture)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();
	adv::Future<int> completingFuture = adv::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

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

BOOST_FIXTURE_TEST_CASE(TryFailureWithBoost, BoostFixture)
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
