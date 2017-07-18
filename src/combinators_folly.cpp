#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>

void testCollectAll()
{
	folly::Future<int> futures[] = {
		folly::makeFuture<int>(0),
		folly::makeFuture<int>(1),
		folly::makeFuture<int>(std::runtime_error("Failure"))
	};

	folly::Future<std::vector<folly::Try<int>>> f = folly::collectAll(std::begin(futures), std::end(futures));

	f.wait();
}

void testCollect()
{
	folly::Future<int> futures[] = {
		folly::makeFuture<int>(0),
		folly::makeFuture<int>(1),
		folly::makeFuture<int>(std::runtime_error("Failure"))
	};

	folly::Future<std::vector<int>> f = folly::collect(std::begin(futures), std::end(futures));

	f.wait();

	std::cout << "Exception: " << f.getTry().exception().get_exception()->what() << std::endl;
}

void testCollectAny()
{
	folly::Future<int> futures[] = {
		folly::makeFuture<int>(0),
		folly::makeFuture<int>(1),
		folly::makeFuture<int>(std::runtime_error("Failure"))
	};

	folly::Future<std::pair<std::size_t, folly::Try<int>>> f = folly::collectAny(std::begin(futures), std::end(futures));

	f.wait();

	std::cout << "Index: " << f.value().first << std::endl;
}

void testCollectAnyWithoutException()
{
	/*
	 * When all futures fail, the returned future of folly::collectAnyWithoutException() fails with last thrown exception.
	 */
	folly::Future<int> futures[] = {
		folly::makeFuture<int>(std::runtime_error("Failure 0")),
		folly::makeFuture<int>(std::runtime_error("Failure 1")),
		folly::makeFuture<int>(std::runtime_error("Failure 2"))
	};

	folly::Future<std::pair<size_t, int>> f = folly::collectAnyWithoutException(std::begin(futures), std::end(futures));

	f.wait();

	std::cout << "Exception: " << f.getTry().exception().get_exception()->what() << std::endl;
}

void testCollectN()
{
	folly::Future<int> futures[] = {
		folly::makeFuture(0),
		folly::makeFuture(1),
		folly::makeFuture(2)
	};

	folly::Future<std::vector<std::pair<std::size_t, folly::Try<int>>>> f = folly::collectN(std::begin(futures), std::end(futures), 2);

	f.wait();
}

void testWindow()
{
	std::vector<int> values
	{
		0,
		1,
		2
	};

	std::vector<folly::Future<int>> result = folly::window(values, [](int &&v) { return folly::makeFuture(v + 1); }, 2);

	for (folly::Future<int> &f : result)
	{
		f.wait();
	}
}

void testMap()
{
	folly::Future<int> futures[] = {
		folly::makeFuture(0),
		folly::makeFuture(1),
		folly::makeFuture(2)
	};

	folly::Future<std::vector<folly::Future<int>>> f = folly::futures::map(std::begin(futures), std::end(futures), [] (int v) { return v + 1; });

	f.wait();
}

void testReduce()
{
	folly::Future<int> futures[] = {
		folly::makeFuture(0),
		folly::makeFuture(1),
		folly::makeFuture(2)
	};

	folly::Future<int> f = folly::reduce(std::begin(futures), std::end(futures), 0, [] (int r, int v) { return r + v; });

	f.wait();
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	testCollectAll();

	testCollect();

	testCollectAny();

	testCollectAnyWithoutException();

	testCollectN();

	testWindow();

	testMap();

	testReduce();

	return 0;
}