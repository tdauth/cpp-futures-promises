#ifndef CURRENCY_H
#define CURRENCY_H

#include <iostream>
#include <utility>
#include <tuple>

enum Currency
{
	USD,
	EUR,
	CHF
};

const double amount = 100;

double currentValue(Currency from, Currency to)
{
	switch (from)
	{
		case USD:
		{
			switch (to)
			{
				case EUR:
				{
					return 0.88;
				}

				case CHF:
				{
					return 0.97;
				}

				case USD:
				{
					return 1.0;
				}
			}

			break;
		}

		default:
		{
			break;
		}
	}

	throw std::runtime_error("unknown exchange rate");

	return 0;
}

std::string currencyName(Currency currency)
{
	switch (currency)
	{
		case EUR:
		{
			return "EUR";
		}

		case CHF:
		{
			return "CHF";
		}

		case USD:
		{
			return "USD";
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

typedef std::tuple<Currency, double> Transaction;

Transaction buy(Currency currency, double amount, double quote)
{
	return std::make_tuple(currency, amount * quote);
}

void printPurchase(Currency currency, double amount)
{
	std::cout << "Purchased " << amount << " " << currencyName(currency) << std::endl;
}

void printFailure(const std::exception &e)
{
	std::cerr << "Purchase failed: " << e.what() << std::endl;
}

#endif