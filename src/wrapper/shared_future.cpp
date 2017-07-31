#include <iostream>

#include <folly/init/Init.h>

#include <wangle/concurrent/GlobalExecutor.h>

#include "shared_future_folly.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	xtn::SharedFuture<std::string> f(folly::makeFuture("My test value"));

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

	return 0;
}