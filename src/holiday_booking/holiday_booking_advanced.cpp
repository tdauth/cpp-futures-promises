#include "holiday_booking.h"

using namespace adv;

int main()
{
	folly::InlineExecutor ex;
	Future<Hotel> switzerland = async(&ex, getHotelSwitzerland);
	Future<Hotel> usa = async(&ex, getHotelUSA);
	Future<Hotel> hotel = switzerland.orElse(usa);
	hotel.onComplete(bookHotelAdv);
	hotel.onComplete(informFriendsAdv);

	return 0;
}
