#ifndef ADV_MVAR_CORE_H
#define ADV_MVAR_CORE_H

#include "../core.h"
#include "mvar.h"

namespace adv_mvar
{

template <typename T>
class Core : public adv::Core<T>
{
	public:
	using Parent = adv::Core<T>;
	using Self = Core<T>;
	using Callbacks = typename Parent::Callbacks;
	using State = typename Parent::State;
	using Try = adv::Try<T>;
	using MVar = adv_mvar::MVar<State>;
	using StateSharedPtr = std::shared_ptr<MVar>;
	using MVarSignal = adv_mvar::MVar<void>;

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
		auto s = _s->take();

		if (s.index() == 0)
		{
			_s->put(std::move(s));
			return false;
		}
		else
		{
			auto hs = std::get<typename Parent::Callbacks>(s);
			_s->put(std::move(v));
			_signal.put();

			for (std::size_t i = 0; i < hs.size(); ++i)
			{
				submitCallback(std::move(hs.at(i)));
			}

			return true;
		}
	}

	void onComplete(typename Parent::Callback &&h) override
	{
		auto s = _s->take();

		if (s.index() == 1)
		{
			auto hs = std::get<typename Parent::Callbacks>(s);
			hs.push_back(h);
			s = std::move(hs);
			_s->put(std::move(s));
		}
		else
		{
			_s->put(std::move(s));
			submitCallback(std::move(h));
		}
	}

	const adv::Try<T> &get() override
	{
		_signal.read();
		return std::get<Try>(_s->read());
	}

	bool isReady() const override
	{
		auto s = _s->take();
		auto r = s.index() == 0;
		_s->put(std::move(s));

		return r;
	}

	protected:
	explicit Core(folly::Executor *executor) : Parent(executor)
	{
		_s = std::make_shared<MVar>(State(Callbacks()));
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
			auto r = std::get<Try>(ptr->read());
			h(r);
		});
	}

	private:
	/*
	 * We have to use a shared pointer here to keep the result alive in the
	 * callbacks.
	 */
	StateSharedPtr _s;
	MVarSignal _signal;
};

} // namespace adv_mvar

#endif
