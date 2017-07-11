#include <future>

#include "currency.h"

int main()
{
	std::future<double> exchangeRate = std::async(currentValue, USD, EUR);

	std::future<double> purchase = std::async([exchangeRate = std::move(exchangeRate)] () mutable
	{
		return buy(amount, exchangeRate.get());
	});

	std::future<void> print = std::async([purchase = std::move(purchase)] () mutable
	{
		printPurchase(purchase.get(), EUR);
	});

	print.wait();

	return 0;
}