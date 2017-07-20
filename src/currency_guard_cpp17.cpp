#include <future>

#include "currency.h"

int main()
{
	std::future<double> exchangeRate = std::async([] { return currentValue(USD, EUR); });

	std::future<double> purchase = std::async([exchangeRate = std::move(exchangeRate)] () mutable
		{
			double quote = exchangeRate.get();

			if (!isProfitable(quote))
			{
				throw std::runtime_error("not profitable");
			}

			return buy(amount, quote);
		}
	);

	std::future<void> print = std::async([purchase = std::move(purchase)] () mutable
		{
			try
			{
				printPurchase(EUR, purchase.get());
			}
			catch (const std::exception &e)
			{
				printFailure(e);
			}
		}
	);

	print.wait();

	return 0;
}