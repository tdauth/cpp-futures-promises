#define BOOST_TEST_MODULE AdvancedFollySharedFutureTest
#include <boost/test/unit_test.hpp>

#include "../../test_suite.h"
#include "../future.h"
#include "../future_impl.h"
#include "../shared_future.h"
#include "../shared_future_impl.h"

using SharedTestSuite = adv::TestSuite<adv_folly::SharedFuture<int>>;

BOOST_FIXTURE_TEST_CASE(TestAll, SharedTestSuite)
{
	testAll();
}