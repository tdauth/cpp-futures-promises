#define BOOST_TEST_MODULE AdvancedLockFutureTest

#include "../../test_suite.h"
#include "../state.h"

using TestSuite =
    adv::TestSuite<adv_lock::State<int>, adv_lock::State<std::string>>;

BOOST_FIXTURE_TEST_CASE(TestAll, TestSuite)
{
	testAll();
}