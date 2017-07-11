#include <boost/thread.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include "currency.h"

int main()
{
	boost::basic_thread_pool ex;
	boost::future<double> exchangeRate = boost::async(ex, std::bind(currentValue, USD, EUR));

	boost::future<double> purchase = exchangeRate
		.then([] (boost::future<double> f)
		{
			double exchangeRate = f.get();

			if (!isProfitable(exchangeRate))
			{
				throw std::runtime_error("not profitable");
			}

			return buy(amount, exchangeRate);
		});


	boost::future<void> print = purchase
		.then([] (boost::future<double> f)
		{
			try
			{
				printPurchase(f.get(), EUR);
			}
			catch (const std::exception &e)
			{
				printFailure(e);
			}
		});

	print.wait();

	return 0;
}