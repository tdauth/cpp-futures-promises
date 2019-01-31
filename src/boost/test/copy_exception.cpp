#define BOOST_TEST_MODULE AdvancedBoostCopyExceptionTest
#include <boost/test/included/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/exception/all.hpp>

/**
 * This test case shows that we have to use boost::exception_ptr instead of std::exception_ptr for the Boost.Thread based implementation.
 * Otherwise, the rethrows won't work.
 */
BOOST_AUTO_TEST_CASE(CopyCurrentException)
{
	try
	{
		throw std::runtime_error("Error!");
	}
	catch (...)
	{
		boost::optional<boost::variant<int, boost::exception_ptr>> v(std::move(boost::current_exception()));
		boost::promise<int> p;
		boost::future<int> f = p.get_future();

		try
		{
			boost::rethrow_exception(std::move(boost::get<boost::exception_ptr>(std::move(v.get()))));
			BOOST_FAIL("Invalid rethrow");
		}
		catch (...)
		{
			p.set_exception(std::move(boost::current_exception()));
		}

		try
		{
			f.get();
		}
		catch (const std::exception &e)
		{
			// success
			return;
		}
		catch (...)
		{
			BOOST_FAIL("Did not keep the exception information");
		}
	}

	BOOST_FAIL("Invalid test");
}
