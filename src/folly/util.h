#ifndef ADV_FOLLY_UTIL_H
#define ADV_FOLLY_UTIL_H

#include "../try.h"
#include <folly/Try.h>

namespace adv_folly
{

template <typename T>
inline adv::Try<T> tryFromFollyTry(folly::Try<T> &&t)
{
	try
	{
		return adv::Try<T>(std::move(t).value());
	}
	catch (...)
	{
		return adv::Try<T>(std::current_exception());
	}
}

template <typename T>
inline folly::Try<T> follyTryFromTry(adv::Try<T> &&t)
{
	try
	{
		return folly::Try<T>(std::move(t).get());
	}
	catch (std::exception &e)
	{
		return folly::Try<T>(folly::exception_wrapper(std::current_exception(), e));
	}
	catch (...)
	{
		return folly::Try<T>(folly::exception_wrapper(std::current_exception()));
	}
}
}

#endif