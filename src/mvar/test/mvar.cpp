#define BOOST_TEST_MODULE MVarTest

#include <boost/test/included/unit_test.hpp>

#include "mvar/mvar.h"

BOOST_AUTO_TEST_CASE(MVarInt)
{
	adv_mvar::MVar<int> mvar;
	BOOST_REQUIRE(mvar.isEmpty());
	mvar.put(1);
	BOOST_REQUIRE(!mvar.isEmpty());
	BOOST_REQUIRE_EQUAL(1, mvar.read());
	BOOST_REQUIRE(!mvar.isEmpty());
	BOOST_REQUIRE_EQUAL(1, mvar.take());
	BOOST_REQUIRE(mvar.isEmpty());
	mvar.put(1);
	BOOST_REQUIRE(!mvar.isEmpty());
}

BOOST_AUTO_TEST_CASE(MVarVoid)
{
	adv_mvar::MVar<void> mvar;
	BOOST_REQUIRE(mvar.isEmpty());
	mvar.put();
	BOOST_REQUIRE(!mvar.isEmpty());
	mvar.read();
	BOOST_REQUIRE(!mvar.isEmpty());
	mvar.take();
	BOOST_REQUIRE(mvar.isEmpty());
	mvar.put();
	BOOST_REQUIRE(!mvar.isEmpty());
}