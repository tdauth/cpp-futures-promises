#ifndef ADV_MVAR_MVAR_H
#define ADV_MVAR_MVAR_H

#include <condition_variable>
#include <mutex>
#include <optional>

namespace adv_mvar
{

/**
 * http://hackage.haskell.org/package/base-4.12.0.0/docs/Control-Concurrent-MVar.html
 *
 * Some references:
 * https://github.com/sanketr/mvar
 * https://stackoverflow.com/a/8941979/1221159
 * https://haskellhideout.wordpress.com/2010/11/14/mvars-in-c/
 */
template <typename T>
class MVar
{
	public:
	using Self = MVar<T>;

	MVar() = default;
	explicit MVar(T &&v) : v(v)
	{
	}

	MVar(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	void put(T &&v)
	{
		{
			std::unique_lock<std::mutex> l(m);
			putCondition.wait(l, [this] { return !this->v; });
			this->v = v;
		}

		takeCondition.notify_all();
	}

	T take()
	{
		T r;

		{
			std::unique_lock<std::mutex> l(m);
			takeCondition.wait(l, [this] { return this->v; });
			r = std::move(*v);
			v.reset();
		}

		putCondition.notify_all();

		return r;
	}

	const T &read()
	{
		std::unique_lock<std::mutex> l(m);
		takeCondition.wait(l, [this] { return this->v; });

		return *v;
	}

	bool isEmpty()
	{
		std::unique_lock<std::mutex> l(m);
		return !v;
	}

	private:
	std::optional<T> v;
	std::mutex m;
	std::condition_variable takeCondition;
	std::condition_variable putCondition;
};

template <>
class MVar<void>
{
	public:
	using Self = MVar<void>;

	MVar() = default;
	MVar(const Self &other) = delete;
	Self &operator=(const Self &other) = delete;

	void put()
	{
		{
			std::unique_lock<std::mutex> l(m);
			putCondition.wait(l, [this] { return !this->v; });
			this->v = true;
		}

		takeCondition.notify_all();
	}

	void take()
	{
		{
			std::unique_lock<std::mutex> l(m);
			takeCondition.wait(l, [this] { return this->v; });
			this->v = false;
		}

		putCondition.notify_all();
	}

	void read()
	{
		std::unique_lock<std::mutex> l(m);
		takeCondition.wait(l, [this] { return this->v; });
	}

	bool isEmpty()
	{
		std::unique_lock<std::mutex> l(m);
		return !v;
	}

	private:
	bool v{false};
	std::mutex m;
	std::condition_variable takeCondition;
	std::condition_variable putCondition;
};

} // namespace adv_mvar

#endif
