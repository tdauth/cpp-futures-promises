#ifndef ADV_THREADPOOLEXECUTOR_H
#define ADV_THREADPOOLEXECUTOR_H

#include <folly/executors/CPUThreadPoolExecutor.h>

namespace adv
{

class ThreadPoolExecutor
{
	public:
	ThreadPoolExecutor(int n) : cpuExecutor(new folly::CPUThreadPoolExecutor(n))
	{
	}

	virtual ~ThreadPoolExecutor()
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
