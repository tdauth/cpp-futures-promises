#include <iostream>
#include <string>

#include <boost/thread.hpp>

int main()
{
	boost::shared_future<std::string> f0 = boost::make_ready_future(std::string("test")).share();
	std::cout << "Result 0: " << f0.get() << std::endl;
	std::cout << "Result 1: " << f0.get() << std::endl;
	boost::shared_future<std::string> f1 = f0;
	std::cout << "Result 2: " << f1.get() << std::endl;
	std::cout << "Result 3: " << f1.get() << std::endl;

	return 0;
}