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
	using Callback = typename Parent::Callback;
	using Callbacks = typename Parent::Callbacks;
	using State = typename Parent::State;
	using Value = typename Parent::Value;
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

	bool tryComplete(Value &&v) override
	{
		auto s = state->take();

		if (s.index() == 0)
		{
			state->put(std::move(s));
			return false;
		}
		else
		{
			auto hs = std::get<Callbacks>(s);
			state->put(std::move(v));
			signal.put();

			for (auto h : hs)
			{
				submitCallback(std::move(h));
			}

			return true;
		}
	}

	void onComplete(Callback &&h) override
	{
		auto s = state->take();

		if (s.index() == 1)
		{
			auto hs = std::get<Callbacks>(s);
			hs.push_back(h);
			s = std::move(hs);
			state->put(std::move(s));
		}
		else
		{
			state->put(std::move(s));
			submitCallback(std::move(h));
		}
	}

	const Value &get() override
	{
		signal.read();
		return std::get<Value>(state->read());
	}

	bool isReady() const override
	{
		auto s = state->take();
		auto r = s.index() == 0;
		state->put(std::move(s));

		return r;
	}

	protected:
	explicit Core(adv::Executor *executor) : Parent(executor)
	{
		state = std::make_shared<MVar>(State(Callbacks()));
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
	void submitCallback(Callback &&h)
	{
		auto ptr = state;
		Parent::getExecutor()->add([h = std::move(h), ptr]() mutable {
			auto r = std::get<Value>(ptr->read());
			h(r);
		});
	}

	private:
	/*
	 * We have to use a shared pointer here to keep the result alive in the
	 * callbacks.
	 */
	StateSharedPtr state;
	MVarSignal signal;
};

} // namespace adv_mvar

#endif
