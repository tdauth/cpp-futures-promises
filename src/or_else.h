#ifndef ORELSE_H
#define ORELSE_H

#include <folly/futures/Future.h>

namespace folly
{

template<typename T>
Future<T> orElse(Future<T> &&first, Future<T> &&second)
{
	return first.then([second = std::move(second)] (Try<T> t) mutable
		{
			if (!t.hasValue())
			{
				second.wait();

				if (second.hasValue())
				{
					return second.getTry().value();
				}

				t.exception().throw_exception();
			}

			return t.value();
		}
	);
}

}

#endif