#include <future>

#include "currency.h"

int main()
{
	double exchangeRate = currentValue(USD, EUR);
	double purchase = buy(amount, exchangeRate);
	printPurchase(purchase, EUR);

	return 0;
}