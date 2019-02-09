#ifndef HOLIDAYBOOKING_H
#define HOLIDAYBOOKING_H

#include <iostream>

#include <boost/thread.hpp>

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

	return "Unknown hotel";
}

Hotel getHotelSwitzerland()
{
	return Switzerland;
}

Hotel getHotelUSA()
{
	return USA;
}

Hotel bookHotel(Hotel hotel)
{
	std::cerr << "Booking hotel " << hotelToString(hotel) << std::endl;
	return hotel;
}

Hotel informFriends(Hotel hotel)
{
	std::cerr << "Informing friends about hotel " << hotelToString(hotel)
	          << std::endl;
	return hotel;
}

Hotel toHotel(std::pair<std::size_t, Hotel> &&pair)
{
	return pair.second;
}

Hotel bookHotelBoost(boost::future<Hotel> &&hotel)
{
	return bookHotel(hotel.get());
}

Hotel informFriendsBoost(boost::future<Hotel> &&hotel)
{
	return informFriends(hotel.get());
}

Hotel informFriendsCpp17(Hotel hotel)
{
	return bookHotel(hotel);
}

Hotel bookHotelCpp17(Hotel hotel)
{
	return informFriends(hotel);
}

Hotel bookHotelFolly(Hotel &&hotel)
{
	return bookHotel(hotel);
}

Hotel informFriendsFolly(Hotel &&hotel)
{
	return informFriends(hotel);
}

void bookHotelAdv(const adv::Try<Hotel> &hotel)
{
	bookHotel(hotel.get());
}

void informFriendsAdv(const adv::Try<Hotel> &hotel)
{
	informFriends(hotel.get());
}

#endif
