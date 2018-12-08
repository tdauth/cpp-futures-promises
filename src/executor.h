#ifndef ADV_EXECUTOR_H
#define ADV_EXECUTOR_H

#include <folly/executors/CPUThreadPoolExecutor.h>

namespace adv
{

class Executor
{
	public:
	Executor(int n) : cpuExecutor(new folly::CPUThreadPoolExecutor(n))
	{
	}

	virtual ~Executor()
	{
		delete cpuExecutor;
		cpuExecutor = nullptr;
	}

	template <typename Func>
	void submit(Func &&f)
	{
		cpuExecutor->add(std::move(f));
	}

	private:
	folly::CPUThreadPoolExecutor *cpuExecutor;
};
}

#endif
