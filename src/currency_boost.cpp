#include <boost/thread.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include "currency.h"

int main()
{
	boost:: basic_thread_pool ex;
	boost::future<double> rateQuote = boost::async(ex, std::bind(currentValue, EUR));

	boost::future<double> purchase = rateQuote
	.then([] (boost::future<double> f)
	{
		return buy(amount, f.get());
	});

	purchase.then([] (boost::future<double> f)
	{
		printPurchase(f.get(), EUR);
	}).wait(); // synchronize in the end?

	return 0;
}