#ifndef ADV_INLINEEXECUTOR_H
#define ADV_INLINEEXECUTOR_H

#include <folly/executors/InlineExecutor.h>

namespace adv
{

class InlineExecutor
{
	public:
	InlineExecutor() : inlineExecutor(new folly::InlineExecutor())
	{
	}

	virtual ~InlineExecutor()
	{
		delete inlineExecutor;
		inlineExecutor = nullptr;
	}

	template <typename Func>
	void submit(Func &&f)
	{
		inlineExecutor->add(std::move(f));
	}

	private:
	folly::InlineExecutor *inlineExecutor;
};
}

#endif
