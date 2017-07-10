#ifndef CURRENCY_H
#define CURRENCY_H

#include <iostream>
#include <functional>
#include <utility>
#include <tuple>

enum Currency
{
	CHF,
	EUR
};

double amount = 100;

double currentValue(Currency currency)
{
	switch (currency)
	{
		case CHF:
		{
			return 0.97;
		}

		case EUR:
		{
			return 0.88;
		}
	}

	return 0;
}

std::string currencyName(Currency currency)
{
	switch (currency)
	{
		case CHF:
		{
			return "CHF";
		}

		case EUR:
		{
			return "EUR";
		}
	}

	return "";
}

bool isProfitable(double quote)
{
	return quote >= 0.8;
}

double buy(double amount, double quote)
{
	return amount * quote;
}

std::tuple<Currency, double> buy2(Currency currency, double amount, double quote)
{
	return std::make_tuple(currency, amount * quote);
}

void printPurchase(double amount, Currency currency)
{
	std::cout << "Purchased " << amount << " " << currencyName(currency) << std::endl;
}

void printPurchase2(std::tuple<Currency, double> v)
{
	std::cout << "Purchased " << std::get<1>(v) << " " << currencyName(std::get<0>(v)) << std::endl;
}

void printFailure(const std::exception &e)
{
	std::cerr << "Purchase failed: " << e.what() << std::endl;
}

#endif