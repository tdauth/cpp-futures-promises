#define BOOST_TEST_MODULE AdvancedLockFutureTest

#include "../../test_suite.h"
#include "lock/core.h"

using TestSuite =
    adv::TestSuite<adv_lock::Core<int>, adv_lock::Core<std::string>>;

BOOST_FIXTURE_TEST_CASE(TestAll, TestSuite)
{
	testAll();
}