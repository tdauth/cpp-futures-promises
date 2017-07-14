#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/futures/SharedPromise.h>

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::SharedPromise<std::string> p;
	folly::Future<std::string> f0 = p.getFuture();
	folly::Future<std::string> f1 = p.getFuture();
	p.setValue("test");
	std::cout << "Result 0: " << f0.get() << std::endl;
	std::cout << "Result 1: " << f1.get() << std::endl;

	return 0;
}