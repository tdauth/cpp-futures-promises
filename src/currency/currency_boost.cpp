#include <boost/thread.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include "currency.h"

int main()
{
	boost::basic_thread_pool ex;
	boost::future<double> exchangeRate = boost::async(ex, [] ()
		{
			return currentValue(USD, EUR);
		}
	);

	boost::future<double> purchase = exchangeRate
		.then([] (boost::future<double> f)
		{
			return buy(amount, f.get());
		}
	);

	boost::future<void> print = purchase
		.then([] (boost::future<double> f)
		{
			printPurchase(EUR, f.get());
		}
	);

	print.wait();

	return 0;
}