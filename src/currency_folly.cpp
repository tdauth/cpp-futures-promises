#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <wangle/concurrent/GlobalExecutor.h>

#include "currency.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<double> exchangeRate = folly::via(wangle::getCPUExecutor().get(), [] ()
		{
			return currentValue(USD, EUR);
		}
	);

	folly::Future<double> purchase = exchangeRate
		.then([] (double v)
			{
				return buy(amount, v);
			}
		);

	folly::Future<folly::Unit> print = purchase
		.then([] (double v)
			{
				printPurchase(EUR, v);
			}
		)
		.onError(printFailure);

	print.wait();

	return 0;
}