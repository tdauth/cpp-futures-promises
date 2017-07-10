#include <boost/thread.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include "currency.h"

int main()
{
	boost:: basic_thread_pool ex;
	boost::future<double> rateQuote = boost::async(ex, std::bind(currentValue, USD));

	boost::future<double> purchase = rateQuote
		.then([] (boost::future<double> f)
		{
			auto quote = f.get();

			if (isProfitable(quote))
			{
				return buy(amount, quote);
			}

			throw std::runtime_error("not profitable");
		});


	purchase.then([] (boost::future<double> f)
		{
			printPurchase(f.get());
		})
		.wait(); // synchronize in the end?

	return 0;
}