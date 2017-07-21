#include <iostream>

#include <boost/thread.hpp>

void testValue()
{
	boost::promise<int> p;
	boost::future<int> f = p.get_future();
	p.set_value(10);

	std::cout << "Result: " << f.get() << std::endl;
}

void testException()
{
	boost::promise<int> p;
	boost::future<int> f = p.get_future();
	p.set_exception(std::runtime_error("Failure"));

	try
	{
		std::cout << "Result: " << f.get() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

void lazySet(boost::promise<int> &p)
{
	p.set_value(10);
}

void testWaitCallback()
{
	boost::promise<int> p;
	p.set_wait_callback(lazySet);
	boost::future<int> f = p.get_future();

	std::cout << "Result: " << f.get() << std::endl;
}

int main()
{
	testValue();
	testException();
	testWaitCallback();

	return 0;
}