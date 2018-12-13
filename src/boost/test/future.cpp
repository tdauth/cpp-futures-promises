#define BOOST_TEST_MODULE AdvancedBoostFutureTest

#include "../future.h"
#include "../../test_suite.h"
#include "../../test_suite_first_n.h"
#include "../future_impl.h"
#include "../promise.h"

using TestSuite = adv::TestSuite<adv_boost::Future<int>>;
using TestSuiteFirstN = adv::TestSuiteFirstN<adv_boost::Future<int>>;

BOOST_FIXTURE_TEST_CASE(TestAll, TestSuite)
{
	testAll();
}

BOOST_FIXTURE_TEST_CASE(TestAllFirstN, TestSuiteFirstN)
{
	testAll();
}