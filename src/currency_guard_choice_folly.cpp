#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <wangle/concurrent/GlobalExecutor.h>

#include "currency.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<double> rateQuoteEUR = folly::via(wangle::getCPUExecutor().get(),
		[] ()
		{
			return currentValue(USD, EUR);
		}
	);

	folly::Future<Transaction> purchaseEUR = rateQuoteEUR
		.filter(isProfitable)
		.then([] (double v)
			{
				return buy(EUR, amount, v);
			}
		);

	folly::Future<double> rateQuoteCHF = folly::via(wangle::getCPUExecutor().get(),
		[] ()
		{
			return currentValue(USD, CHF);
		}
	);

	folly::Future<Transaction> purchaseCHF = rateQuoteCHF
		.filter(isProfitable)
		.then([] (double v)
			{
				return buy(CHF, amount, v);
			}
		);

	folly::Future<Transaction> futures[] =
	{
		std::move(purchaseEUR),
		std::move(purchaseCHF)
	};

	folly::Future<folly::Unit> print = folly::collectAnyWithoutException(
			std::begin(futures), std::end(futures)
		)
		.then([](std::pair<std::size_t, Transaction> pair)
			{
				printPurchase(std::get<0>(pair.second), std::get<1>(pair.second));
			}
		)
		.onError(printFailure);

	print.wait();

	return 0;
}