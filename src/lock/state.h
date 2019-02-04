#ifndef ADV_LOCK_STATE_H
#define ADV_LOCK_STATE_H

#include "../state.h"

namespace adv_lock
{

template <typename T>
class State : public adv::State<T>
{
	public:
	using Parent = adv::State<T>;
	using Self = State<T>;

	State(folly::Executor *executor) : Parent(executor)
	{
	}

	State(Self &&other) : Parent(std::move(other))
	{
	}

	Self &operator=(Self &&other)
	{
		Parent::operator=(std::move(other));
		return *this;
	}

	State(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	virtual bool tryComplete(adv::Try<T> &&v) override
	{
		std::lock_guard<std::mutex> l(m);

		if (_s.index() == 0)
		{
			return false;
		}
		else
		{
			auto hs = std::get<typename Parent::Callbacks>(_s);
			_s = std::move(v);

			for (std::size_t i = 0; i < hs.size(); ++i)
			{
				auto h = std::move(hs.at(i));
				Parent::getExecutor()->add([h = std::move(h), v]() { h(v); });
			}

			return true;
		}
	}

	virtual void onComplete(typename Parent::Callback &&h) override
	{
		std::lock_guard<std::mutex> l(m);

		if (_s.index() == 1)
		{
			typename Parent::Callbacks &hs = std::get<typename Parent::Callbacks>(_s);

			hs.push_back(h);
		}
		else
		{
			const adv::Try<T> &v = std::get<adv::Try<T>>(_s);
			Parent::getExecutor()->add([h = std::move(h), &v]() mutable { h(v); });
		}
	}

	virtual T get() override
	{
		std::mutex m;
		std::condition_variable c;
		adv::Try<T> r;
		onComplete([&r, &c, &m](const adv::Try<T> &t) {
			std::lock_guard<std::mutex> guard(m);
			r = t;
			c.notify_one();
		});

		std::unique_lock<std::mutex> l(m);

		if (!r.hasValue() && !r.hasException())
		{
			c.wait(l);
		}

		return r.get();
	}

	virtual bool isReady() const override
	{
		std::lock_guard<std::mutex> l(m);

		return _s.index() == 0;
	}

	private:
	mutable std::mutex m;
	typename Parent::S _s{typename Parent::Callbacks()};
};

} // namespace adv_lock

#endif
