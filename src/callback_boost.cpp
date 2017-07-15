#include <iostream>
#include <string>

#include <boost/thread.hpp>

int main()
{
	boost::future<int> f0 = boost::make_ready_future(10);
	boost::future<int> f1 =  f0.then([](boost::future<int> f) { return f.get() + 1; });
	boost::future<std::string> f2 = f1.then([](boost::future<int> f) { return std::to_string(f.get()); });
	boost::future<void> f3 = f2.then([](boost::future<std::string> f) { std::cout << f.get() << std::endl; });
	f3.wait();

	return 0;
}