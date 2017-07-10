#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <wangle/concurrent/GlobalExecutor.h>

#include "currency.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<double> rateQuote = folly::via(wangle::getCPUExecutor().get(), std::bind(currentValue, EUR));

	folly::Future<double> purchase = rateQuote
		.filter(isProfitable)
		.then(std::bind(buy, amount, std::placeholders::_1))
		.onError([] (folly::PredicateDoesNotObtain)
		{
			throw std::runtime_error("not profitable");

			return 0.0;
		});

	purchase.then(std::bind(printPurchase, std::placeholders::_1, EUR)).onError(printFailure)
		.wait(); // synchronize in the end?

	return 0;
}