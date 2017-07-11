#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <wangle/concurrent/GlobalExecutor.h>

#include "currency.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<double> exchangeRate = folly::via(wangle::getCPUExecutor().get(), std::bind(currentValue, USD, EUR));

	folly::Future<double> purchase = exchangeRate
		.filter(isProfitable)
		.then(std::bind(buy, amount, std::placeholders::_1))
		.onError([] (folly::PredicateDoesNotObtain)
		{
			throw std::runtime_error("not profitable");

			return 0.0;
		});

	folly::Future<folly::Unit> print = purchase
		.then(std::bind(printPurchase, std::placeholders::_1, EUR))
		.onError(printFailure);

	print.wait();

	return 0;
}