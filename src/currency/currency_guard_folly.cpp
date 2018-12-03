#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>

#include "currency.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<double> exchangeRate = folly::via(folly::getCPUExecutor().get(), [] ()
		{
			return currentValue(USD, EUR);
		}
	);

	folly::Future<double> purchase = std::move(exchangeRate)
		.filter(isProfitable)
		.thenValue([] (double v)
			{
				return buy(amount, v);
			}
		);

	folly::Future<folly::Unit> print = std::move(purchase)
		.thenValue([] (double v)
			{
				printPurchase(EUR, v);
			}
		)
		.onError(printFailure);

	print.wait();

	return 0;
}
