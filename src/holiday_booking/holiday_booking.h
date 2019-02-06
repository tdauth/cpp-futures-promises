#ifndef HOLIDAYBOOKING_H
#define HOLIDAYBOOKING_H

#include <iostream>

#include <folly/Try.h>

#include "../advanced_futures_promises.h"

enum Hotel
{
	Switzerland,
	USA
};

std::string hotelToString(Hotel h)
{
	switch (h)
	{
	case Switzerland:
		return "Switzerland";
	case USA:
		return "USA";
	}

	return "Unknow hotel";
}

Hotel getHotelSwitzerland();
Hotel getHotelUSA();

Hotel toHotel(std::pair<std::size_t, Hotel> &&pair)
{
	return pair.second;
}

void bookHotel(const adv::Try<Hotel> &hotel)
{
	std::cerr << "Booking hotel " << hotelToString(hotel.get()) << std::endl;
}

void informFriends(const adv::Try<Hotel> &hotel)
{
	std::cerr << "Informing friends about hotel " << hotelToString(hotel.get())
	          << std::endl;
}

Hotel bookHotelFolly(Hotel &&hotel)
{
	std::cerr << "Booking hotel " << hotelToString(hotel) << std::endl;

	return hotel;
}

Hotel informFriendsFolly(Hotel &&hotel)
{
	std::cerr << "Informing friends about hotel " << hotelToString(hotel)
	          << std::endl;

	return hotel;
}

#endif
