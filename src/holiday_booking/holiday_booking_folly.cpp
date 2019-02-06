#include <folly/executors/Async.h>
#include <folly/futures/Future.h>
#include <folly/futures/helpers.h>

#include "holiday_booking.h"

using namespace folly;
using namespace std;

int main()
{
	/*
	TODO fix it
	auto switzerland = async(&getHotelSwitzerland);
	auto usa = async(&getHotelUSA);
	auto c = vector<Future<Hotel>>{move(switzerland), move(usa)};
	auto hotel = collectAnyWithoutException(move(c))
	                 .via(getCPUExecutor().get())
	                 .thenValue(toHotel); //  Combinator!
	move(hotel).thenValue(bookHotelFolly).thenValue(informFriendsFolly);
	*/

	return 0;
}