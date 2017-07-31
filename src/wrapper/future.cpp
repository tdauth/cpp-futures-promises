#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include "future_folly.h"
#include "promise_folly.h"

void testGuard(wish::Executor *ex)
{
	wish::Future<int> f2 = wish::async(ex, [] ()
		{
			return 10;
		}
	).guard([] (const int &v) { return v == 10; });

	std::cout << "Result guard: " << f2.get() << std::endl;
}

void testThen(wish::Executor *ex)
{
	wish::Future<std::string> f3 = wish::async(ex, [] ()
		{
			return 10;
		}
	).then([] (wish::Try<int> t)
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

void testOrElse(wish::Executor *ex)
{
	wish::Future<int> f0 = wish::async(ex, [] () { return 10; });
	wish::Future<int> f1 = wish::async(ex, [] () { return 11; });
	auto f2 = f0.orElse(std::move(f1));
	std::cout << "Result orElse: " << f2.get() << std::endl;
}

void testFirst(wish::Executor *ex)
{
	wish::Future<int> f0 = wish::async(ex, [] () { return 10; });
	wish::Future<int> f1 = wish::async(ex, [] () { return 11; });
	auto f2 = f0.first(std::move(f1));
	std::cout << "Result first: " << f2.get() << std::endl;
}

void testFirstSucc(wish::Executor *ex)
{
	wish::Future<int> f0 = wish::async(ex, [] () { return 10; });
	wish::Future<int> f1 = wish::async(ex, [] () { return 11; });
	auto f2 = f0.firstSucc(std::move(f1));
	std::cout << "Result firstSucc: " << f2.get() << std::endl;
}

void testFirstN(wish::Executor *ex)
{
	std::vector<wish::Future<int>> futures;
	futures.push_back(wish::async(ex, [] () { return 10; }));
	futures.push_back(wish::async(ex, [] () { return 11; }));
	futures.push_back(wish::async(ex, [] () { return 12; }));
	futures.push_back(wish::async(ex, [] () { return 13; }));

	wish::Future<std::vector<std::pair<std::size_t, wish::Future<int>>>> f = wish::firstN(std::move(futures), 3);
	std::vector<std::pair<std::size_t, wish::Future<int>>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstN: index: " << v[i].first << ", value: " << v[i].second.get() << std::endl;
	}
}

void testFirstNSucc(wish::Executor *ex)
{
	std::vector<wish::Future<int>> futures;
	futures.push_back(wish::async(ex, [] () { return 10; }));
	futures.push_back(wish::async(ex, [] () { return 11; }));
	futures.push_back(wish::async(ex, [] () { return 12; }));
	futures.push_back(wish::async(ex, [] () { return 13; }));

	wish::Future<std::vector<std::pair<std::size_t, int>>> f = wish::firstNSucc(std::move(futures), 3);
	std::vector<std::pair<std::size_t, int>> v = f.get();

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		std::cout << "Result firstNSucc: index: " << v[i].first << ", value: " << v[i].second << std::endl;
	}
}

void testTryComplete(wish::Executor *ex)
{
	wish::Promise<int> p;
	wish::Future<int> f = p.future();

	bool result = p.tryComplete(wish::Try<int>(10));

	std::cout << "Result tryComplete: " << result << std::endl;
	std::cout << "Future result tryComplete: " << f.get() << std::endl;
}

void testTrySuccess(wish::Executor *ex)
{
	wish::Promise<int> p;
	wish::Future<int> f = p.future();

	bool result = p.trySuccess(10);

	std::cout << "Result trySuccess: " << result << std::endl;
	std::cout << "Future result trySuccess: " << f.get() << std::endl;
}

void testTryFailure(wish::Executor *ex)
{
	wish::Promise<int> p;
	wish::Future<int> f = p.future();

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

void testTryCompleteWith(wish::Executor *ex)
{
	wish::Promise<int> p;
	wish::Future<int> f = p.future();
	wish::Future<int> completingFuture = wish::async(ex, [] () { return 10; });

	p.tryCompleteWith(std::move(completingFuture));

	std::cout << "Future result tryCompleteWith: " << f.get() << std::endl;
}

void testTrySuccessWith(wish::Executor *ex)
{
	wish::Promise<int> p;
	wish::Future<int> f = p.future();
	wish::Future<int> completingFuture = wish::async(ex, [] () { return 10; });

	p.trySuccessWith(std::move(completingFuture));

	std::cout << "Future result trySuccessWith: " << f.get() << std::endl;
}

void testTryFailureWith(wish::Executor *ex)
{
	wish::Promise<int> p;
	wish::Future<int> f = p.future();
	wish::Future<int> completingFuture = wish::async(ex, [] () { throw std::runtime_error("Failure!"); return 10; });

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

	wish::Executor ex(wangle::getCPUExecutor().get());
	wish::Future<int> f0 = wish::async(&ex, [] ()
		{
			return 10;
		}
	);
	f0.onComplete([] (wish::Try<int> t)
		{
			std::cout << "Result: " << t.get() << std::endl;
		}
	);

	wish::Future<int> f1 = wish::async(&ex, [] ()
		{
			return 10;
		}
	);
	std::cout << "is ready: " << f1.isReady() << std::endl;
	std::cout << f1.get() << std::endl;

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