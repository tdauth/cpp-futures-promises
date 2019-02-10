#include "holiday_booking.h"

using namespace adv;

int main()
{
	folly::InlineExecutor follyEx;
	FollyExecutor ex(&follyEx);
	auto switzerland = async(&ex, getHotelSwitzerland);
	auto usa = async(&ex, getHotelUSA);
	auto hotel = switzerland.fallbackTo(usa);
	hotel.onComplete(bookHotelAdv);
	hotel.onComplete(informFriendsAdv);

	return 0;
}
