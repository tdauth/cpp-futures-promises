#ifndef ADV_TESTFIXTURE_H
#define ADV_TESTFIXTURE_H

#include "inline_executor.h"
#include <folly/init/Init.h>

namespace adv
{

struct TestFixture
{
	// Do not use multiple threads to detect invalid blocking.
	TestFixture() : ex(new InlineExecutor())
	{
	}

	~TestFixture()
	{
		delete ex;
		ex = nullptr;
	}

	InlineExecutor *ex;
};
}

#endif