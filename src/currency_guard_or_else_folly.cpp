#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <wangle/concurrent/GlobalExecutor.h>

#include "currency.h"
#include "or_else.h"

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::Future<double> rateQuoteEUR = folly::via(wangle::getCPUExecutor().get(), std::bind(currentValue, USD, EUR));

	folly::Future<Transaction> purchaseEUR = rateQuoteEUR
		.filter(isProfitable)
		.then(std::bind(buy2, EUR, amount, std::placeholders::_1))
		.onError([] (folly::PredicateDoesNotObtain)
		{
			throw std::runtime_error("not profitable");

			return std::make_tuple(EUR, 0.0);
		});

	folly::Future<double> rateQuoteCHF = folly::via(wangle::getCPUExecutor().get(), std::bind(currentValue, USD, CHF));

	folly::Future<Transaction> purchaseCHF = rateQuoteCHF
		.filter(isProfitable)
		.then(std::bind(buy2, CHF, amount, std::placeholders::_1))
		.onError([] (folly::PredicateDoesNotObtain)
		{
			throw std::runtime_error("not profitable");

			return std::make_tuple(CHF, 0.0);
		});

	folly::Future<folly::Unit> print = orElse(std::move(purchaseEUR), std::move(purchaseCHF))
		.then(printPurchase2)
		.onError(printFailure);


	print.wait();

	return 0;
}