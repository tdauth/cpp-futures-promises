#include <future>

#include "currency.h"

int main()
{
	std::future<double> rateQuote = std::async(std::launch::async, currentValue, EUR);

	std::future<double> purchase = std::async(std::launch::async, [rateQuote = std::move(rateQuote)] () mutable
	{
		return buy(amount, rateQuote.get());
	});

	std::async(std::launch::async, [purchase = std::move(purchase)] () mutable
	{
		printPurchase(purchase.get(), EUR);
	}).wait();

	return 0;
}