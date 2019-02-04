#ifndef ADV_STATE_H
#define ADV_STATE_H

#include "try.h"

namespace adv
{

template <typename T>
class State
{
	public:
	using Type = T;
	using ValueType = Try<T>;
	using Callback = std::function<void(const ValueType &)>;
	using Callbacks = std::vector<Callback>;
	using S = std::variant<ValueType, Callbacks>;
	using Self = State<T>;

	State(folly::Executor *executor) : executor(executor)
	{
	}

	State(Self &&other) : executor(other.executor)
	{
	}

	Self &operator=(Self &&other)
	{
		this->executor = other.executor;
		return *this;
	}

	State(const Self &other) = delete;

	Self &operator=(const Self &other) = delete;

	template <typename S>
	static State<S> *create(folly::Executor *executor);

	template <typename S>
	static std::shared_ptr<State<S>> createShared(folly::Executor *executor);

	virtual bool tryComplete(adv::Try<T> &&v) = 0;

	virtual void onComplete(Callback &&h) = 0;

	virtual T get() = 0;

	virtual bool isReady() const = 0;

	folly::Executor *getExecutor() const
	{
		return executor;
	}

	std::shared_ptr<Self> incrementPromiseCounts(std::shared_ptr<Self> s)
	{
		promiseCounter++;

		return s;
	}

	int getPromiseCountsAndDecrement()
	{
		auto r = promiseCounter.load();

		--promiseCounter;

		return r;
	}

	private:
	folly::Executor *executor;
	std::atomic<int> promiseCounter{0};
};

} // namespace adv

#endif
