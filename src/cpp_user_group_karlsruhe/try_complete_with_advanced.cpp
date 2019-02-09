#include "holiday_booking.h"

using namespace adv;

template <typename T>
void tryCompleteWith(Promise<T> p, Future<T> f)
{
	f.onComplete([p](const Try<T> &t) mutable { p.tryComplete(t); });
}

int main()
{
	folly::InlineExecutor ex;
	auto switzerland = async(&ex, getHotelSwitzerland);
	Promise<Hotel> p(&ex);
	tryCompleteWith(p, switzerland);

	auto f = p.future();
	f.onComplete(bookHotelAdv);
	f.onComplete(informFriendsAdv);

	return 0;
}
