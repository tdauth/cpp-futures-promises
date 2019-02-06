#ifndef ADV_LOCK_CORE_H
#define ADV_LOCK_CORE_H

#include "../core.h"

namespace adv_lock
{

template <typename T>
class Core : public adv::Core<T>
{
	public:
	using Parent = adv::Core<T>;
	using Self = Core<T>;
	using StateSharedPtr = std::shared_ptr<typename Parent::State>;

	Core() = delete;

	// TODO make protected and only allow access by the shared pointer!
	virtual ~Core() = default;

	Core(Self &&other) noexcept : Parent(std::move(other))
	{
	}

	Self &operator=(Self &&other) noexcept
	{
		Parent::operator=(std::move(other));
		return *this;
	}

	Core(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	bool tryComplete(adv::Try<T> &&v) override
	{
		std::lock_guard<std::mutex> l(m);

		if (_s->index() == 0)
		{
			return false;
		}
		else
		{
			auto hs = std::get<typename Parent::Callbacks>(*_s);
			*_s = std::move(v);

			for (std::size_t i = 0; i < hs.size(); ++i)
			{
				submitCallback(std::move(hs.at(i)));
			}

			return true;
		}
	}

	void onComplete(typename Parent::Callback &&h) override
	{
		std::lock_guard<std::mutex> l(m);

		if (_s->index() == 1)
		{
			typename Parent::Callbacks &hs = std::get<typename Parent::Callbacks>(*_s);
			hs.push_back(h);
		}
		else
		{
			submitCallback(std::move(h));
		}
	}

	const adv::Try<T> &get() override
	{
		std::mutex m;
		std::condition_variable c;
		onComplete([&c, &m](const adv::Try<T> &t) {
			std::lock_guard<std::mutex> guard(m);
			c.notify_one();
		});

		if (!isReady())
		{
			std::unique_lock<std::mutex> l(m);
			c.wait(l);
		}

		std::lock_guard<std::mutex> l(m);
		return std::get<adv::Try<T>>(*_s);
	}

	bool isReady() const override
	{
		std::lock_guard<std::mutex> l(m);

		return _s->index() == 0;
	}

	protected:
	explicit Core(folly::Executor *executor) : Parent(executor)
	{
	}

	/**
	 * Allow access to create a new Core instance.
	 */
	template <typename U>
	friend class adv::Core;

	/**
	 * We have to pass a copy of the shared pointer to ensure the lifetime when
	 * reading the result.
	 */
	void submitCallback(typename Parent::Callback &&h)
	{
		auto ptr = _s;
		Parent::getExecutor()->add([h = std::move(h), ptr]() mutable {
			auto r = std::get<adv::Try<T>>(*ptr);
			h(r);
		});
	}

	private:
	mutable std::mutex m;
	/*
	 * We have to use a shared pointer here to keep the result alive in the
	 * callbacks.
	 */
	StateSharedPtr _s{new typename Parent::State(typename Parent::Callbacks())};
};

} // namespace adv_lock

#endif
