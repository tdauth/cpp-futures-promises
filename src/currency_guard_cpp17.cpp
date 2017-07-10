#include <future>

#include "currency.h"

int main()
{
	std::future<double> rateQuote = std::async(std::launch::async, currentValue, EUR);

	std::future<double> purchase = std::async(std::launch::async, [rateQuote = std::move(rateQuote)] () mutable {
		auto quote = rateQuote.get();

		if (!isProfitable(quote))
		{
			throw std::runtime_error("not profitable");
		}

		return buy(amount, quote);
	});

	std::async(std::launch::async, [purchase = std::move(purchase)] () mutable {
		try
		{
			printPurchase(purchase.get(), EUR);
		}
		catch (const std::exception &e)
		{
			printFailure(e);
		}
	}).wait(); // synchronize in the end?

	return 0;
}