#ifndef ADV_TESTFIXTURE_H
#define ADV_TESTFIXTURE_H

#include <folly/executors/InlineExecutor.h>

namespace adv
{

struct TestFixture
{
	/*
	 * Do not use multiple threads to detect invalid blocking.
	 * The inline executor ensures the immediate execution of callbacks in the same
	 * thread if the futures is already completed. This simplifies testing.
	 */
	TestFixture() : ex(new folly::InlineExecutor())
	{
	}

	~TestFixture()
	{
		delete ex;
		ex = nullptr;
	}

	folly::InlineExecutor *ex;
};
}

#endif