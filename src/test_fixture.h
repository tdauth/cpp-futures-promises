#ifndef ADV_TESTFIXTURE_H
#define ADV_TESTFIXTURE_H

#include "executor.h"
#include <folly/init/Init.h>

namespace adv
{

struct TestFixture
{
	TestFixture() : ex(new adv::Executor(1))
	{
	}

	~TestFixture()
	{
		delete ex;
		ex = nullptr;
	}

	Executor *ex;
};
}

#endif