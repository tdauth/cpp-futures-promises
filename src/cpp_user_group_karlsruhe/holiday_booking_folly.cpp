#include <folly/executors/Async.h>
#include <folly/futures/Future.h>
#include <folly/futures/helpers.h>
#include <folly/init/Init.h>

#include "holiday_booking.h"

using namespace folly;
using namespace std;

template <typename T>
vector<Future<T>> toVector(Future<T> &&f0, Future<T> &&f1)
{
	vector<Future<Hotel>> c;
	c.push_back(move(f0));
	c.push_back(move(f1));

	return c;
}

int main(int argc, char *argv[])
{
	init(&argc, &argv);
	auto switzerland = async(&getHotelSwitzerland);
	auto usa = async(&getHotelUSA);
	auto hotel = collectAnyWithoutException(toVector(move(switzerland), move(usa)))
	                 .via(getCPUExecutor().get())
	                 .thenValue(toHotel);
	move(hotel).thenValue(bookHotelFolly).thenValue(informFriendsFolly);

	return 0;
}