#include <future>

#include "holiday_booking.h"

using namespace std;

int main()
{
	promise<Hotel> p;
	auto switzerland = async([&p] { p.set_value(getHotelSwitzerland()); });
	auto usa = async([&p] { p.set_value(getHotelUSA()); });
	async([&p] {
		auto hotel = p.get_future().get();
		bookHotelCpp17(hotel);
		informFriendsCpp17(hotel);
	})
	    .get();

	return 0;
}