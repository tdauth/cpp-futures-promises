#ifndef CURRENCY_H
#define CURRENCY_H

#include <iostream>
#include <functional>
#include <utility>

enum Currency
{
	USD
};

double amount = 100;

double currentValue(Currency currency)
{
	switch (currency)
	{
		case USD:
		{
			return 1.14;
		}
	}

	return 0;
}

bool isProfitable(double quote)
{
	return quote > 1.0;
}

double buy(double amount, double quote)
{
	return amount * quote;
}

void printPurchase(double amount)
{
	std::cout << "Purchased " << amount << " USD" << std::endl;
}

#endif