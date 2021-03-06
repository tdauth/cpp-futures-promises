#ifndef ADV_CORE_H
#define ADV_CORE_H

#include "executor.h"
#include "try.h"

namespace adv
{

/**
 * This exception is thrown when the result of a future is retrieved but the
 * corresponding promise has already been deleted before completing the future
 * meaning the future would never be completed by the promise.
 */
class BrokenPromise : public std::exception
{
};

template <typename T>
class Core
{
	public:
	using Type = T;
	using Value = Try<T>;
	using Callback = std::function<void(const Value &)>;
	using Callbacks = std::vector<Callback>;
	using State = std::variant<Value, Callbacks>;
	using Self = Core<T>;
	using SharedPtr = std::shared_ptr<Self>;

	enum Implementation
	{
		MVar
	};

	Core() = delete;
	Core(const Self &) = delete;
	Self &operator=(const Self &) = delete;

	// TODO Allow access only by std::shared_ptr!
	virtual ~Core() = default;

	template <typename S>
	static typename Core<S>::SharedPtr
	createShared(Executor *executor, Implementation implementation = MVar);

	virtual bool tryComplete(Value &&v) = 0;

	virtual void onComplete(Callback &&h) = 0;

	virtual const Value &get() = 0;

	virtual bool isReady() const = 0;

	Executor *getExecutor() const
	{
		return executor;
	}

	void incrementPromiseCounter()
	{
		++promiseCounter;
	}

	void decrementPromiseCounter()
	{
		auto r = --promiseCounter;

		if (r == 0)
		{
			tryComplete(Try<T>(std::make_exception_ptr(BrokenPromise())));
		}
	}

	protected:
	explicit Core(Executor *executor) : executor(executor)
	{
	}

	private:
	Executor *executor;
	// We do always start with one promise.
	std::atomic<int> promiseCounter{1};
};

} // namespace adv

#endif
