#ifndef ADV_EXECUTOR_H
#define ADV_EXECUTOR_H

#include <functional>

namespace adv
{

class Executor
{
	public:
	using Function = std::function<void()>;

	virtual ~Executor()
	{
	}

	virtual void add(Function &&f) = 0;
};

} // namespace adv

#endif
