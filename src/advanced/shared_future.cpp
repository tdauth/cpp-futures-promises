#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include "advanced_futures_folly.h"

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
	adv::SharedFuture<std::string> f(std::move(uniqueF));

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

	std::cerr << "Multiple read semantics: " << std::endl;
	std::cerr << "Result 0: " << f.get() << std::endl;
	std::cerr << "Result 1: " << f.get() << std::endl;

	adv::Executor ex(wangle::getCPUExecutor().get());

	testOrElse(&ex);
	testFirst(&ex);
	testFirstN(&ex);
	testFirstNSucc(&ex);

	return 0;
}