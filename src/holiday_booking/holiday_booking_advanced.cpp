#include <iostream>

#include "../advanced_futures_promises.h"

enum Hotel
{
	Switzerland,
	USA
};

void bookHotel(adv::Try<Hotel> hotel)
{
	std::cerr << "Booking hotel." << std::endl;
}

void informFriends(adv::Try<Hotel> hotel)
{
	std::cerr << "Informing friends." << std::endl;
}

int main()
{
	folly::InlineExecutor ex;
	adv::Future<Hotel> switzerland = adv::async(&ex, []() { return Switzerland; });
	adv::Future<Hotel> usa = adv::async(&ex, []() { return USA; });
	adv::Future<Hotel> hotel = switzerland.orElse(usa);
	hotel.onComplete([](const adv::Try<Hotel> &t) { bookHotel(t); });
	hotel.onComplete([](const adv::Try<Hotel> &t) { informFriends(t); });

	hotel.get();

	return 0;
}
