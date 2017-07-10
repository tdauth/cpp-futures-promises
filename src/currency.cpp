#include <future>

#include "currency.h"

int main()
{
	std::future<double> rateQuote = std::async(currentValue, EUR);

	std::future<double> purchase = std::async([rateQuote = std::move(rateQuote)] () mutable {
		auto quote = rateQuote.get();

		if (isProfitable(quote))
		{
			return buy(amount, quote);
		}

		throw std::runtime_error("not profitable");
	});

	std::async([purchase = std::move(purchase)] () mutable {
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