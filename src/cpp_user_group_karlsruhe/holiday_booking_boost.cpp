#include <boost/thread.hpp>
#include <boost/thread/concurrent_queues/queue_op_status.hpp> // TODO Apprently we have to include this before
#include <boost/thread/executors/inline_executor.hpp>

#include "holiday_booking.h"

using namespace boost;

int main()
{
	inline_executor ex;
	promise<Hotel> p;
	auto switzerland = async(ex, [&p] { p.set_value(getHotelSwitzerland()); });
	auto usa = async(ex, [&p] { p.set_value(getHotelUSA()); });
	auto hotel = p.get_future();
	hotel.then(bookHotelBoost).then(informFriendsBoost).get();

	return 0;
}