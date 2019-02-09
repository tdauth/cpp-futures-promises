#define BOOST_TEST_MODULE MVarTest

#include <thread>

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

BOOST_AUTO_TEST_CASE(MVarIntMultipleThreads)
{
	adv_mvar::MVar<int> mvar;
	BOOST_REQUIRE(mvar.isEmpty());

	std::vector<int> results;
	std::mutex m;

	std::thread t0([&mvar, &results, &m] {
		auto r = mvar.take();
		std::lock_guard<std::mutex> l(m);
		results.push_back(r);
	});

	std::thread t1([&mvar] { mvar.put(1); });

	std::thread t2([&mvar] { mvar.put(2); });

	std::thread t3([&mvar, &results, &m] {
		auto r = mvar.take();
		std::lock_guard<std::mutex> l(m);
		results.push_back(r);
	});

	t0.join();
	t1.join();
	t2.join();
	t3.join();

	BOOST_REQUIRE(mvar.isEmpty());
	BOOST_REQUIRE_EQUAL(2, results.size());
	BOOST_REQUIRE(std::find(results.begin(), results.end(), 1) != results.end());
	BOOST_REQUIRE(std::find(results.begin(), results.end(), 2) != results.end());
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