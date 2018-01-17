#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include <boost/thread/executors/basic_thread_pool.hpp>

#include "advanced_futures_folly.h"
#include "advanced_futures_boost.h"

void testOnComplete(adv::Executor *ex)
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

template<typename Ex>
void testOnCompleteBoost(adv_boost::Executor<Ex> *ex)
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


void testGetAndIsReady(adv::Executor *ex)
{
	adv::Future<int> f1 = adv::async(ex, [] ()
		{
			return 10;
		}
	);
	std::cout << "is ready: " << f1.isReady() << std::endl;
	std::cout << f1.get() << std::endl;
}

template<typename Ex>
void testGetAndIsReadyBoost(adv_boost::Executor<Ex> *ex)
{
	adv_boost::Future<int> f1 = adv_boost::async(ex, [] ()
		{
			return 10;
		}
	);
	std::cout << "is ready: " << f1.isReady() << std::endl;
	std::cout << f1.get() << std::endl;
}

void testGuard(adv::Executor *ex)
{
	adv::Future<int> f2 = adv::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	std::cout << "Result guard: " << f2.get() << std::endl;
}

void testThen(adv::Executor *ex)
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

	std::cout << "Result then: " << f3.get() << std::endl;
}

void testOrElse(adv::Executor *ex)
{
	adv::Future<int> f0 = adv::async(ex, [] () { return 10; });
	adv::Future<int> f1 = adv::async(ex, [] () { return 11; });
	auto f2 = f0.orElse(std::move(f1));
	std::cout << "Result orElse: " << f2.get() << std::endl;
}

void testFirst(adv::Executor *ex)
{
	adv::Future<int> f0 = adv::async(ex, [] () { return 10; });
	adv::Future<int> f1 = adv::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));
	std::cout << "Result first: " << f2.get() << std::endl;
}

void testFirstSucc(adv::Executor *ex)
{
	adv::Future<int> f0 = adv::async(ex, [] () { return 10; });
	adv::Future<int> f1 = adv::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	std::cout << "Result firstSucc: " << f2.get() << std::endl;
}

void testFirstN(adv::Executor *ex)
{
	std::vector<adv::Future<int>> futures;
	futures.push_back(adv::async(ex, [] () { return 10; }));
	futures.push_back(adv::async(ex, [] () { return 11; }));
	futures.push_back(adv::async(ex, [] () { return 12; }));
	futures.push_back(adv::async(ex, [] () { return 13; }));

	adv::Future<std::vector<std::pair<std::size_t, adv::Try<int>>>> f = adv::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv::Try<int>>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstN: index: " << v[i].first << ", value: " << v[i].second.get() << std::endl;
	}
}

void testFirstNSucc(adv::Executor *ex)
{
	std::vector<adv::Future<int>> futures;
	futures.push_back(adv::async(ex, [] () { return 1; }));
	futures.push_back(adv::async(ex, [] () { throw std::runtime_error("Failure!"); return 2; }));
	futures.push_back(adv::async(ex, [] () { return 3; }));
	futures.push_back(adv::async(ex, [] () { return 4; }));

	adv::Future<std::vector<std::pair<std::size_t, int>>> f = adv::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstNSucc: index: " << v[i].first << ", value: " << v[i].second << std::endl;
	}
}

void testTryComplete(adv::Executor *ex)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();

	bool result = p.tryComplete(adv::Try<int>(10));

	std::cout << "Result tryComplete: " << result << std::endl;
	std::cout << "Future result tryComplete: " << f.get() << std::endl;
}

void testTrySuccess(adv::Executor *ex)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	std::cout << "Result trySuccess: " << result << std::endl;
	std::cout << "Future result trySuccess: " << f.get() << std::endl;
}

void testTryFailure(adv::Executor *ex)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();

	bool result = p.tryFailure(std::runtime_error("Failure!"));

	std::cout << "Result tryFailure: " << result << std::endl;

	try
	{
		f.get();
	}
	catch (const std::exception &e)
	{
		std::cout << "Future result tryFailure: " << e.what() << std::endl;
	}
}

void testTryCompleteWith(adv::Executor *ex)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();
	adv::Future<int> completingFuture = adv::async(ex, [] () { return 10; });

	p.tryCompleteWith(std::move(completingFuture));

	std::cout << "Future result tryCompleteWith: " << f.get() << std::endl;
}

void testTrySuccessWith(adv::Executor *ex)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();
	adv::Future<int> completingFuture = adv::async(ex, [] () { return 10; });

	p.trySuccessWith(std::move(completingFuture));

	std::cout << "Future result trySuccessWith: " << f.get() << std::endl;
}

void testTryFailureWith(adv::Executor *ex)
{
	adv::Promise<int> p;
	adv::Future<int> f = p.future();
	adv::Future<int> completingFuture = adv::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

	p.tryFailureWith(std::move(completingFuture));

	try
	{
		f.get();
	}
	catch (const std::exception &e)
	{
		std::cout << "Future result tryFailureWith: " << e.what() << std::endl;
	}
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	adv::Executor ex(wangle::getCPUExecutor().get());

	testOnComplete(&ex);
	testGetAndIsReady(&ex);
	testGuard(&ex);
	testThen(&ex);
	testOrElse(&ex);
	testFirst(&ex);
	testFirstSucc(&ex);
	testFirstN(&ex);
	testFirstNSucc(&ex);
	testTryComplete(&ex);
	testTrySuccess(&ex);
	testTryFailure(&ex);
	testTryCompleteWith(&ex);
	testTrySuccessWith(&ex);
	testTryFailureWith(&ex);

	boost::basic_thread_pool thread_pool;
	adv_boost::Executor<boost::basic_thread_pool> ex_boost(&thread_pool);
	testOnCompleteBoost(&ex_boost);
	testGetAndIsReadyBoost(&ex_boost);

	return 0;
}
