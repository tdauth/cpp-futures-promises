#ifndef ADV_TRY_H
#define ADV_TRY_H

#include <optional>
#include <type_traits>
#include <variant>

#include <folly/futures/Future.h>

namespace adv
{

class UsingUninitializedTry : public std::exception
{
};

template <typename T>
class Try
{
	public:
	Try() = default;

	explicit Try(T &&v) : _v(std::move(v))
	{
	}

	explicit Try(std::exception_ptr &&e) : _v(std::move(e))
	{
	}

	Try(const Try<T> &other) : _v(other._v)
	{
	}

	Try(Try<T> &&other) noexcept : _v(std::move(other._v))
	{
	}

	Try<T> &operator=(const Try<T> &other)
	{
		_v = other._v;
		return *this;
	}

	T get()
	{
		if (!_v.has_value())
		{
			throw adv::UsingUninitializedTry();
		}

		if (_v.value().index() != 0)
		{
			std::rethrow_exception(
			    std::move(std::get<std::exception_ptr>(std::move(_v).value())));
		}

		return std::move(std::get<T>(std::move(_v.value())));
	}

	const T &get() const
	{
		if (!_v.has_value())
		{
			throw adv::UsingUninitializedTry();
		}

		if (_v.value().index() != 0)
		{
			std::rethrow_exception(std::get<std::exception_ptr>(_v.value()));
		}

		return std::get<T>(_v.value());
	}

	bool hasValue() const
	{
		if (_v.has_value())
		{
			return _v.value().index() == 0;
		}

		return false;
	}

	bool hasException() const
	{
		if (_v.has_value())
		{
			return _v.value().index() == 1;
		}

		return false;
	}

	private:
	std::optional<std::variant<T, std::exception_ptr>> _v;
};
} // namespace adv

#endif