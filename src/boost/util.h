#ifndef ADV_BOOST_UTIL_H
#define ADV_BOOST_UTIL_H

#include "../try.h"
#include <folly/Try.h>

namespace adv_boost
{

template <typename T>
inline adv::Try<T> convertFutureIntoTry(boost::future<T> f)
{
	try
	{
		return adv::Try<T>(f.get());
	}
	catch (...)
	{
		return adv::Try<T>(std::current_exception());
	}
}
}

#endif