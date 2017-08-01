#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include "future_folly.h"
#include "promise_folly.h"

void testOnComplete(xtn::Executor *ex)
{
	xtn::Future<int> f0 = xtn::async(ex, [] ()
		{
			return 10;
		}
	);
	f0.onComplete([] (xtn::Try<int> t)
		{
			std::cout << "Result: " << t.get() << std::endl;
		}
	);
}

void testGetAndIsReady(xtn::Executor *ex)
{
	xtn::Future<int> f1 = xtn::async(ex, [] ()
		{
			return 10;
		}
	);
	std::cout << "is ready: " << f1.isReady() << std::endl;
	std::cout << f1.get() << std::endl;
}

void testGuard(xtn::Executor *ex)
{
	xtn::Future<int> f2 = xtn::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	std::cout << "Result guard: " << f2.get() << std::endl;
}

void testThen(xtn::Executor *ex)
{
	xtn::Future<std::string> f3 = xtn::async(ex, [] ()
		{
			return 10;
		}
	).then([] (xtn::Try<int> t)
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

void testOrElse(xtn::Executor *ex)
{
	xtn::Future<int> f0 = xtn::async(ex, [] () { return 10; });
	xtn::Future<int> f1 = xtn::async(ex, [] () { return 11; });
	auto f2 = f0.orElse(std::move(f1));
	std::cout << "Result orElse: " << f2.get() << std::endl;
}

void testFirst(xtn::Executor *ex)
{
	xtn::Future<int> f0 = xtn::async(ex, [] () { return 10; });
	xtn::Future<int> f1 = xtn::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));
	std::cout << "Result first: " << f2.get() << std::endl;
}

void testFirstSucc(xtn::Executor *ex)
{
	xtn::Future<int> f0 = xtn::async(ex, [] () { return 10; });
	xtn::Future<int> f1 = xtn::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	std::cout << "Result firstSucc: " << f2.get() << std::endl;
}

void testFirstN(xtn::Executor *ex)
{
	std::vector<xtn::Future<int>> futures;
	futures.push_back(xtn::async(ex, [] () { return 10; }));
	futures.push_back(xtn::async(ex, [] () { return 11; }));
	futures.push_back(xtn::async(ex, [] () { return 12; }));
	futures.push_back(xtn::async(ex, [] () { return 13; }));

	xtn::Future<std::vector<std::pair<std::size_t, xtn::Try<int>>>> f = xtn::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, xtn::Try<int>>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstN: index: " << v[i].first << ", value: " << v[i].second.get() << std::endl;
	}
}

void testFirstNSucc(xtn::Executor *ex)
{
	std::vector<xtn::Future<int>> futures;
	futures.push_back(xtn::async(ex, [] () { return 10; }));
	futures.push_back(xtn::async(ex, [] () { return 11; }));
	futures.push_back(xtn::async(ex, [] () { return 12; }));
	futures.push_back(xtn::async(ex, [] () { return 13; }));

	xtn::Future<std::vector<std::pair<std::size_t, int>>> f = xtn::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstNSucc: index: " << v[i].first << ", value: " << v[i].second << std::endl;
	}
}

void testTryComplete(xtn::Executor *ex)
{
	xtn::Promise<int> p;
	xtn::Future<int> f = p.future();

	bool result = p.tryComplete(xtn::Try<int>(10));

	std::cout << "Result tryComplete: " << result << std::endl;
	std::cout << "Future result tryComplete: " << f.get() << std::endl;
}

void testTrySuccess(xtn::Executor *ex)
{
	xtn::Promise<int> p;
	xtn::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	std::cout << "Result trySuccess: " << result << std::endl;
	std::cout << "Future result trySuccess: " << f.get() << std::endl;
}

void testTryFailure(xtn::Executor *ex)
{
	xtn::Promise<int> p;
	xtn::Future<int> f = p.future();

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

void testTryCompleteWith(xtn::Executor *ex)
{
	xtn::Promise<int> p;
	xtn::Future<int> f = p.future();
	xtn::Future<int> completingFuture = xtn::async(ex, [] () { return 10; });

	p.tryCompleteWith(std::move(completingFuture));

	std::cout << "Future result tryCompleteWith: " << f.get() << std::endl;
}

void testTrySuccessWith(xtn::Executor *ex)
{
	xtn::Promise<int> p;
	xtn::Future<int> f = p.future();
	xtn::Future<int> completingFuture = xtn::async(ex, [] () { return 10; });

	p.trySuccessWith(std::move(completingFuture));

	std::cout << "Future result trySuccessWith: " << f.get() << std::endl;
}

void testTryFailureWith(xtn::Executor *ex)
{
	xtn::Promise<int> p;
	xtn::Future<int> f = p.future();
	xtn::Future<int> completingFuture = xtn::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

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

	xtn::Executor ex(wangle::getCPUExecutor().get());

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

	return 0;
}