#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#
int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);
	folly::Future<std::string> f = folly::makeFuture(std::string("test"));
	std::cout << "Result: " << f.get() << std::endl;
	std::cout << "Is ready: " << f.isReady() << std::endl;
	std::cout << "Is valid: ???" << std::endl;
	std::cout << "Result: " << f.get() << std::endl;

	return 0;
}