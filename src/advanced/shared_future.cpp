#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include "advanced_futures_folly.h"

void testOnComplete(adv::Executor *ex)
{
	adv::SharedFuture<int> f0 = adv::async(ex, [] ()
		{
			return 10;
		}
	).share();

	/*
	 * Shared futures allow multiple calls:
	 */
	for (int i = 0; i < 10; ++i)
	{
		f0.onComplete([] (adv::Try<int> t)
			{
				std::cout << "Result onComplete: " << t.get() << std::endl;
			}
		);
	}
}

void testGetAndIsReady(adv::Executor *ex)
{
	adv::SharedFuture<int> f1 = adv::async(ex, [] ()
		{
			return 10;
		}
	).share();
	std::cout << "is ready: " << f1.isReady() << std::endl;
	std::cout << f1.get() << std::endl;
	std::cout << f1.get() << std::endl;
}

void testGuard(adv::Executor *ex)
{
	adv::SharedFuture<int> f2 = adv::async(ex, [] ()
		{
			return 10;
		}
	).share().guard([] (const int &v) { return v == 10; });

	std::cout << "Result guard: " << f2.get() << std::endl;
}

void testThen(adv::Executor *ex)
{
	adv::SharedFuture<int> f = adv::SharedFuture<int>(adv::async(ex, [] ()
		{
			return 10;
		}
	));

	/*
	 * A share future allows multiple registrations of callbacks.
	 */
	std::vector<adv::SharedFuture<std::string>> futures;

	for (int i = 0; i < 10; ++i)
	{
		futures.push_back(
			f.then([] (adv::Try<int> t)
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

	for (int i = 0; i < futures.size(); ++i)
	{
		std::cout << "Result then " << i << ": " << futures[i].get() << std::endl;
	}
}

void testOrElse(adv::Executor *ex)
{
	adv::SharedFuture<int> f0(folly::makeFuture(10));
	adv::SharedFuture<int> f1(folly::makeFuture(10));
	auto f2 = f0.orElse(std::move(f1));
	std::cout << "Result orElse: " << f2.get() << std::endl;
}

void testFirst(adv::Executor *ex)
{
	adv::SharedFuture<int> f0(folly::makeFuture(10));
	adv::SharedFuture<int> f1(folly::makeFuture(10));
	auto f2 = f0.first(std::move(f1));
	std::cout << "Result first: " << f2.get() << std::endl;
}

void testFirstSucc(adv::Executor *ex)
{
	adv::SharedFuture<int> f0(folly::makeFuture(10));
	adv::SharedFuture<int> f1(folly::makeFuture(10));
	auto f2 = f0.firstSucc(std::move(f1));
	std::cout << "Result firstSucc: " << f2.get() << std::endl;
}

void testFirstN(adv::Executor *ex)
{
	std::vector<adv::SharedFuture<int>> futures;

	for (int i = 0; i < 10; ++i)
	{
		futures.push_back(adv::SharedFuture<int>(adv::async(ex, [] () { return 10; })));
	}

	adv::SharedFuture<std::vector<std::pair<std::size_t, adv::Try<int>>>> f = adv::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, adv::Try<int>>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstN: index: " << v[i].first << ", value: " << v[i].second.get() << std::endl;
	}
}

void testFirstNSucc(adv::Executor *ex)
{
	std::vector<adv::SharedFuture<int>> futures;

	for (int i = 0; i < 10; ++i)
	{
		futures.push_back(adv::SharedFuture<int>(adv::async(ex, [] () { return 10; })));
	}

	adv::SharedFuture<std::vector<std::pair<std::size_t, int>>> f = adv::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstNSucc: index: " << v[i].first << ", value: " << v[i].second << std::endl;
	}
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	adv::Future<std::string> uniqueF(folly::makeFuture<std::string>("My test value"));
	/*
	 * A shared future can be created from a non-shared future.
	 */
	adv::SharedFuture<std::string> f(std::move(uniqueF));

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
	std::cerr << "Multiple read semantics: " << std::endl;
	std::cerr << "Result 0: " << f.get() << std::endl;
	std::cerr << "Result 1: " << f.get() << std::endl;

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

	return 0;
}