#include <folly/futures/Future.h>
#include <folly/futures/helpers.h>
#include <utility>

#include "holiday_booking.h"

using namespace folly;
using namespace std;

template <typename T>
shared_ptr<Promise<T>> tryCompleteWith(Promise<T> &&p, Future<T> &&f)
{
	auto promise = make_shared<Promise<T>>(move(p));
	auto future = make_shared<Future<T>>(move(f));
	future->setCallback_(
	    [promise, future](Try<T> &&t) { promise->setTry(move(t)); });

	return promise;
}

int main()
{
	InlineExecutor ex;
	auto switzerland = via(&ex, &getHotelSwitzerland);
	Promise<Hotel> p;
	auto r = tryCompleteWith(move(p), move(switzerland));

	r->getFuture().thenValue(bookHotelFolly).thenValue(informFriendsFolly);

	return 0;
}
