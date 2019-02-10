#ifndef ADV_FOLLYEXECUTOR_H
#define ADV_FOLLYEXECUTOR_H

#include <folly/Executor.h>

#include "executor.h"

namespace adv
{

class FollyExecutor : public Executor
{
	public:
	using Function = std::function<void()>;

	explicit FollyExecutor(folly::Executor *ex) : ex(ex)
	{
	}

	void add(Function &&f) override
	{
		ex->add(std::move(f));
	}

	private:
	folly::Executor *ex;
};

} // namespace adv

#endif
