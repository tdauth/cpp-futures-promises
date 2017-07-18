#include <boost/thread.hpp>

void testWhenAll()
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));

	boost::future<std::vector<boost::future<int>>> f = boost::when_all(std::begin(futures), std::end(futures));

	f.wait();
}

void testWhenAny()
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));

	boost::future<std::vector<boost::future<int>>> f = boost::when_any(std::begin(futures), std::end(futures));

	f.wait();
}

void testWaitForAll()
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));

	boost::wait_for_all(futures.begin(), futures.end());
}

void testWaitForAny()
{
	std::vector<boost::future<int>> futures;
	futures.push_back(boost::make_ready_future(0));
	futures.push_back(boost::make_ready_future(1));
	futures.push_back(boost::make_exceptional_future<int>(std::runtime_error("Failure")));

	const std::vector<boost::future<int>>::iterator iterator = boost::wait_for_any(futures.begin(), futures.end());

	std::cout << "Index: " << iterator->get() << std::endl;
}

int main()
{
	testWhenAll();

	testWhenAny();

	testWaitForAll();

	testWaitForAny();

	return 0;
}