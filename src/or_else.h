#ifndef ORELSE_H
#define ORELSE_H

#include <folly/futures/Future.h>

template<typename T>
folly::Future<T> orElse(folly::Future<T> &&first, folly::Future<T> &&second)
{
	return first.then([second = std::move(second)] (folly::Try<T> t) mutable
		{
			if (!t.hasValue())
			{
				second.wait();

				if (second.hasValue())
				{
					return second.value();
				}

				t.exception().throw_exception();
			}

			return t.value();
		}
	);
}

#endif