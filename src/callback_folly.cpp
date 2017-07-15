#include <iostream>
#include <string>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<int> f0 = folly::makeFuture(10);
	folly::Future<int> f1 = f0.then([](folly::Future<int> f) { return f.get() + 1; });
	folly::Future<std::string> f2 = f1.then([](int v) { return std::to_string(v); });
	folly::Future<folly::Unit> f3 = f2.then([](folly::Try<std::string> t) { std::cout << t.value() << std::endl; });
	f3.wait();

	return 0;
}