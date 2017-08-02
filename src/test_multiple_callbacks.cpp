/**
 * This program tests if Folly and Boost.Thread allow registering more than one callback per future
 * which is possible with adv::SharedFuture<T>.
 */
#include <boost/thread.hpp>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	try
	{
		boost::future<int> f0 = boost::make_ready_future(10);
		boost::future<int> f1 = f0.then([](boost::future<int> v) { return v.get(); });
		//TODO even leads to a crash
		//boost::future<int> f2 = f0.then([](boost::future<int> v) { return v.get(); });
		std::cout << "Results: " << f1.get() << std::endl;
	}
	catch (const std::logic_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	/*
	 * Shared futures in Boost.Thread allow registering more than one callback.
	 */
	try
	{
		boost::shared_future<int> f0 = boost::make_ready_future(10).share();
		boost::shared_future<int> f1 = f0.then([](boost::shared_future<int> v) { return v.get(); });
		boost::shared_future<int> f2 = f0.then([](boost::shared_future<int> v) { return v.get(); });
		std::cout << "Results: " << f1.get() << " and " << f2.get() << std::endl;
	}
	catch (const std::logic_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	/*
	 * Folly has no shared futures. It does only allow registering one callback per future.
	 */
	try
	{
		folly::Future<int> f0 = folly::makeFuture(10);
		folly::Future<int> f1 = f0.then([](int v) { return v; });
		folly::Future<int> f2 = f0.then([](int v) { return v; });
		std::cout << "Results: " << f1.get() << " and " << f2.get() << std::endl;
	}
	catch (const std::logic_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}