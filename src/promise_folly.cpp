#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>

void testValue()
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();
	p.setValue(10);

	std::cout << "Result: " << f.get() << std::endl;
}

void testException()
{
	folly::Promise<int> p;
	folly::Future<int> f = p.getFuture();
	p.setException(std::runtime_error("Failure"));

	try
	{
		std::cout << "Result: " << f.get() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	testValue();
	testException();

	return 0;
}