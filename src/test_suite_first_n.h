#ifndef ADV_TESTSUITEFIRSTN_H
#define ADV_TESTSUITEFIRSTN_H

#include <boost/test/included/unit_test.hpp>

#include "future.h"
#include "future_impl.h"
#include "promise.h"

namespace adv
{

template <typename FutureType>
class TestSuiteFirstN
{
	public:
	/*
	 * Do not use multiple executor threads to detect invalid blocking.
	 * The inline executor ensures the immediate execution of callbacks
	 * in the main thread. This simplifies testing.
	 */
	TestSuiteFirstN() : ex(new folly::InlineExecutor())
	{
	}

	~TestSuiteFirstN()
	{
		delete ex;
		ex = nullptr;
	}

	void testFirstN()
	{
		std::vector<FutureType> futures;
		futures.push_back(FutureType::successful(ex, 10));
		futures.push_back(FutureType::failed(ex, std::runtime_error("Failure!")));
		futures.push_back(FutureType::successful(ex, 12));
		futures.push_back(FutureType::successful(ex, 13));

		auto f = adv::firstN(ex, std::move(futures), 3);
		auto v = f.get();

		BOOST_CHECK_EQUAL(3u, v.size());
		auto v0 = std::move(v[0]);
		BOOST_CHECK_EQUAL(0u, v0.first);
		BOOST_CHECK_EQUAL(10, v0.second.get());
		auto v1 = std::move(v[1]);
		BOOST_CHECK_EQUAL(1u, v1.first);
		BOOST_CHECK_THROW(v1.second.get(), std::runtime_error);
		auto v2 = std::move(v[2]);
		BOOST_CHECK_EQUAL(2u, v2.first);
		BOOST_CHECK_EQUAL(12, v2.second.get());
	}

	void testFirstNSucc()
	{
		std::vector<FutureType> futures;
		futures.push_back(FutureType::successful(ex, 10));
		futures.push_back(FutureType::failed(ex, std::runtime_error("Failure!")));
		futures.push_back(FutureType::successful(ex, 12));
		futures.push_back(FutureType::successful(ex, 13));

		auto f = adv::firstNSucc(ex, std::move(futures), 3);
		auto v = f.get();

		BOOST_CHECK_EQUAL(3u, v.size());
		auto v0 = std::move(v[0]);
		BOOST_CHECK_EQUAL(0u, v0.first);
		BOOST_CHECK_EQUAL(10, v0.second);
		auto v1 = std::move(v[1]);
		BOOST_CHECK_EQUAL(2u, v1.first);
		BOOST_CHECK_EQUAL(12, v1.second);
		auto v2 = std::move(v[2]);
		BOOST_CHECK_EQUAL(3u, v2.first);
		BOOST_CHECK_EQUAL(13, v2.second);
	}

	void testAll()
	{
		testFirstN();
		testFirstNSucc();
	}

	private:
	folly::InlineExecutor *ex;
};
}

#endif