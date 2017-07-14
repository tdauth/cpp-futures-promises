#include <iostream>
#include <string>

#include <boost/thread.hpp>

int main()
{
	boost::future<std::string> f = boost::make_ready_future(std::string("test"));
	std::cout << "Result 0: " << f.get() << std::endl;

	try
	{
		std::cout << "Is ready: " << f.is_ready() << std::endl;
		std::cout << "Is valid: " << f.valid() << std::endl;
		std::cout << "Result 1: " << f.get() << std::endl;
	}
	catch (const boost::future_uninitialized &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}