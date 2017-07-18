#include <iostream>

#include "choice_combinators.h"

void testFirstSuccess0()
{
	folly::Future<int> f0 = folly::makeFuture(0);
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccess(std::move(f0), std::move(f1));

	std::cout << "Result: " << f.get() << std::endl;
}

void testFirstSuccess1()
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure"));
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccess(std::move(f0), std::move(f1));

	std::cout << "Result: " << f.get() << std::endl;
}

void testFirstSuccess2()
{
	folly::Future<int> f0 = folly::makeFuture<int>(std::runtime_error("Failure 0"));
	folly::Future<int> f1 = folly::makeFuture<int>(std::runtime_error("Failure 1"));

	folly::Future<int> f = firstSuccess(std::move(f0), std::move(f1));

	std::cout << "Result: " << f.getTry().exception().get_exception()->what() << std::endl;
}

void testFirstSuccessRandom0()
{
	folly::Future<int> f0 = folly::makeFuture(0);
	folly::Future<int> f1 = folly::makeFuture(1);

	folly::Future<int> f = firstSuccessRandom(std::move(f0), std::move(f1));

	std::cout << "Result: " << f.get() << std::endl;
}

int main()
{
	testFirstSuccess0();
	testFirstSuccess1();
	testFirstSuccess2();

	testFirstSuccessRandom0();

	return 0;
}