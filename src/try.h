#ifndef ADV_TRY_H
#define ADV_TRY_H

#include <optional>
#include <type_traits>
#include <variant>

#include <folly/futures/Future.h>

namespace adv
{

/**
 * Stores either a result value or exception.
 * Other than Folly's type this can never be empty.
 * It is more similar to Scala's type.
 * @tparam T The type of the result value.
 */
template <typename T>
class Try
{
	public:
	Try() = delete;

	explicit Try(T &&v) : _v(std::move(v))
	{
	}

	explicit Try(std::exception_ptr &&e) : _v(std::move(e))
	{
	}

	Try(const Try<T> &other) : _v(other._v)
	{
	}

	Try<T> &operator=(const Try<T> &other)
	{
		_v = other._v;
		return *this;
	}

	const T &get() const
	{
		if (_v.index() != 0)
		{
			std::rethrow_exception(std::get<std::exception_ptr>(_v));
		}

		return std::get<T>(_v);
	}

	bool hasValue() const
	{
		return _v.index() == 0;
	}

	bool hasException() const
	{
		return _v.index() == 1;
	}

	private:
	std::variant<T, std::exception_ptr> _v;
};

template <typename T>
bool operator==(const Try<T> &t0, const Try<T> &t1)
{
	if (t0.hasValue() && t1.hasValue())
	{
		return t0.get() == t1.get();
	}
	// TODO compare exceptions

	return false;
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const Try<T> t)
{
	if (t.hasValue())
	{
		out << "Try=" << t.get();
	}
	else
	{
		try
		{
			t.get();
		}
		catch (const std::exception &e)
		{
			out << "Try=" << e.what();
		}
		catch (...)
		{
			out << "Try=UnknownException";
		}
	}

	return out;
}

} // namespace adv

#endif