#include <boost/thread.hpp>
#include <boost/thread/concurrent_queues/queue_op_status.hpp> // TODO Apprently we have to include this before
#include <boost/thread/executors/inline_executor.hpp>

#include "holiday_booking.h"

using namespace boost;

template <typename T>
std::shared_ptr<promise<T>> tryCompleteWith(promise<T> &&p, shared_future<T> f)
{
	auto ctx = std::make_shared<promise<T>>(move(p));
	f.then([ctx](shared_future<T> &&f) {
		try
		{
			ctx->set_value(f.get());
		}
		catch (...)
		{
			ctx->set_exception(current_exception());
		}
	});

	return ctx;
}

int main()
{
	inline_executor ex;
	auto switzerland = async(ex, getHotelSwitzerland).share();
	promise<Hotel> p;
	auto r = tryCompleteWith(move(p), switzerland);
	switzerland.then([](shared_future<Hotel> &&f) {
		std::cerr << "After getting Switzerland";
	});

	auto f = r->get_future();
	f.then(bookHotelBoost).then(informFriendsBoost).get();

	return 0;
}