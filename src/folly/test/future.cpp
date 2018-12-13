#define BOOST_TEST_MODULE AdvancedFollyFutureTest

#include "../future.h"
#include "../../test_suite.h"
#include "../future_impl.h"
#include "../promise.h"

using TestSuite = adv::TestSuite<adv_folly::Future<int>>;

BOOST_FIXTURE_TEST_CASE(TryNotInitialized, TestSuite)
{
	testTryNotInitialized();
}

BOOST_FIXTURE_TEST_CASE(TryRuntimeError, TestSuite)
{
	testTryRuntimeError();
}

BOOST_FIXTURE_TEST_CASE(TryValue, TestSuite)
{
	testTryValue();
}

BOOST_FIXTURE_TEST_CASE(OnComplete, TestSuite)
{
	testOnComplete();
}

BOOST_FIXTURE_TEST_CASE(OnSuccess, TestSuite)
{
	testOnSuccess();
}

BOOST_FIXTURE_TEST_CASE(OnFailure, TestSuite)
{
	testOnFailure();
}

BOOST_FIXTURE_TEST_CASE(Get, TestSuite)
{
	testGet();
}

BOOST_FIXTURE_TEST_CASE(IsReady, TestSuite)
{
	testIsReady();
}

BOOST_FIXTURE_TEST_CASE(Then, TestSuite)
{
	testThen();
}

BOOST_FIXTURE_TEST_CASE(ThenWith, TestSuite)
{
	testThenWith();
}

BOOST_FIXTURE_TEST_CASE(Guard, TestSuite)
{
	testGuard();
}

BOOST_FIXTURE_TEST_CASE(GuardFails, TestSuite)
{
	testGuardFails();
}

BOOST_FIXTURE_TEST_CASE(OrElseFirstSuccessful, TestSuite)
{
	testOrElseFirstSuccessful();
}

BOOST_FIXTURE_TEST_CASE(OrElseSecondSuccessful, TestSuite)
{
	testOrElseSecondSuccessful();
}

BOOST_FIXTURE_TEST_CASE(OrElseBothFail, TestSuite)
{
	testOrElseBothFail();
}

BOOST_FIXTURE_TEST_CASE(First, TestSuite)
{
	testFirst();
}

BOOST_FIXTURE_TEST_CASE(FirstWithException, TestSuite)
{
	testFirstWithException();
}

BOOST_FIXTURE_TEST_CASE(FirstSucc, TestSuite)
{
	testFirstSucc();
}

BOOST_FIXTURE_TEST_CASE(FirstSuccWithException, TestSuite)
{
	testFirstSuccWithException();
}

BOOST_FIXTURE_TEST_CASE(FirstSuccBothFail, TestSuite)
{
	testFirstSuccBothFail();
}

BOOST_FIXTURE_TEST_CASE(Successful, TestSuite)
{
	testSuccessful();
}

BOOST_FIXTURE_TEST_CASE(Failed, TestSuite)
{
	testFailed();
}

BOOST_FIXTURE_TEST_CASE(Async, TestSuite)
{
	testAsync();
}

BOOST_FIXTURE_TEST_CASE(FirstN, TestSuite)
{
	testFirstN();
}

BOOST_FIXTURE_TEST_CASE(FirstNSucc, TestSuite)
{
	testFirstNSucc();
}

BOOST_FIXTURE_TEST_CASE(BrokenPromise, TestSuite)
{
	testBrokenPromise();
}

BOOST_FIXTURE_TEST_CASE(TryComplete, TestSuite)
{
	testTryComplete();
}

BOOST_FIXTURE_TEST_CASE(TrySuccess, TestSuite)
{
	testTrySuccess();
}

BOOST_FIXTURE_TEST_CASE(TryFailure, TestSuite)
{
	testTryFailure();
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWith, TestSuite)
{
	testTryCompleteWith();
}

BOOST_FIXTURE_TEST_CASE(TryCompleteWithFailure, TestSuite)
{
	testTryCompleteWithFailure();
}

BOOST_FIXTURE_TEST_CASE(TrySuccessWith, TestSuite)
{
	testTrySuccessWith();
}

BOOST_FIXTURE_TEST_CASE(TryFailureWith, TestSuite)
{
	testTryFailureWith();
}