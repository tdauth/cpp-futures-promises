#include <folly/executors/Async.h>
#include <folly/futures/Future.h>
#include <folly/futures/helpers.h>

#include "holiday_booking.h"

using namespace folly;
using namespace std;

int main()
{
	InlineExecutor ex;
	auto switzerland = via(&ex, &getHotelSwitzerland);
	auto usa = via(&ex, &getHotelUSA);
	vector<Future<Hotel>> c;
	c.push_back(move(switzerland));
	c.push_back(move(usa));
	auto hotel = collectAnyWithoutException(move(c)).via(&ex).thenValue(toHotel);
	move(hotel).thenValue(bookHotelFolly).thenValue(informFriendsFolly);

	return 0;
}