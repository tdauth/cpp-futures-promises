#include <future>

#include "holiday_booking.h"

using namespace std;

int main()
{
	promise<Hotel> p;
	future<Hotel> f = p.get_future();
	async([p = move(p)] {});

	try
	{
		f.get();
	}
	catch (future_error &e)
	{
		cerr << e.what() << endl;
	}

	return 0;
}