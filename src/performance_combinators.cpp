#include <boost/thread.hpp>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/futures/Future.h>

#include "extensions.h"


/**
 * \brief Simple calculation of the power at compile time.
 *
 * Source: https://stackoverflow.com/a/27270738
 */
template <int A, int B>
struct get_power
{
    static const int value = A * get_power<A, B - 1>::value;
};
template <int A>
struct get_power<A, 0>
{
    static const int value = 1;
};

/**
 * \brief This structure allows to generate recursive combinator calls at compile time.
 * TODO the formula is wrong for only 1 node per level?
 * Use the following code to call folly::collectAll() with 10^(5 + 1) - 1 nodes (futures), adding completed futures with the value 3 add the deepest level
 * of the tree:
 * \code
 * RecursiveCombinatorType<int, 5>::collectAllFolly(10, [] { return 3; });
 * \endcode
 * It will create a tree with the height of 5 and 10 nodes per level.
 *
 * \note Be careful with higher numbers of nesting which may lead to very long execution times!
 */
template<typename T, int N>
struct RecursiveCombinatorType
{
	static constexpr int level = N;

	/**
	 * The number of future nodes in the tree started at this level.
	 * Note that the root node is included.
	 * \tparam size The number of futures per level.
	 */
	template<int size>
	static constexpr int nodes = get_power<size, N + 1>::value - 1;

	using previous = RecursiveCombinatorType<T, N - 1>;

	using previous_future_type = typename previous::future_type;
	using previous_vector_type = typename previous::vector_type;
	using previous_type = typename previous::type;
	using future_type = previous_type;
	using vector_type = std::vector<folly::Try<previous_vector_type>>;
	using type = folly::Future<vector_type>;

	template<typename Func>
	static type collectAllFolly(std::size_t size, Func &&f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAllFolly(size, std::move(f)));
		}

		return folly::collectAll(v.begin(), v.end());
	}

	using previous_folly_collect_type = typename previous::folly_collect_type;

	using folly_collect_vector_type = std::vector<typename previous_folly_collect_type::value_type>;
	using folly_collect_type = folly::Future<folly_collect_vector_type>;

	template<typename Func>
	static folly_collect_type collectFolly(std::size_t size, Func &&f)
	{
		std::vector<previous_folly_collect_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectFolly(size, std::move(f)));
		}

		return folly::collect(v.begin(), v.end());
	}

	using previous_folly_collect_n_type = typename previous::folly_collect_n_type;

	using folly_collect_n_vector_value_type = std::pair<std::size_t, folly::Try<typename previous_folly_collect_n_type::value_type>>;
	using folly_collect_n_vector_type = std::vector<folly_collect_n_vector_value_type>;
	using folly_collect_n_type = folly::Future<folly_collect_n_vector_type>;

	template<typename Func>
	static folly_collect_n_type collectNFolly(std::size_t size, Func &&f, int n)
	{
		std::vector<previous_folly_collect_n_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectNFolly(size, std::move(f), n));
		}

		return folly::collectN(v.begin(), v.end(), n);
	}

	using folly_collect_any_previous_pair_type = typename previous::folly_collect_any_pair_type;
	using folly_collect_any_previous_future_pair_type = typename previous::folly_collect_any_future_pair_type;
	using folly_collect_any_pair_type = std::pair<size_t, folly::Try<folly_collect_any_previous_pair_type>>;
	using folly_collect_any_future_pair_type = folly::Future<folly_collect_any_pair_type>;

	template<typename Func>
	static folly_collect_any_future_pair_type collectAnyFolly(std::size_t size, Func &&f)
	{
		std::vector<folly_collect_any_previous_future_pair_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAnyFolly(size, std::move(f)));
		}

		return folly::collectAny(v.begin(), v.end());
	}

	using folly_collect_any_without_exception_previous_future_pair_type = typename previous::folly_collect_any_without_exception_future_pair_type;

	using folly_collect_any_without_exception_pair_type = std::pair<size_t, typename folly_collect_any_without_exception_previous_future_pair_type::value_type>;
	using folly_collect_any_without_exception_future_pair_type = folly::Future<folly_collect_any_without_exception_pair_type>;

	template<typename Func>
	static folly_collect_any_without_exception_future_pair_type collectAnyWithoutExceptionFolly(std::size_t size, Func &&f)
	{
		std::vector<folly_collect_any_without_exception_previous_future_pair_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAnyWithoutExceptionFolly(size, std::move(f)));
		}

		return folly::collectAnyWithoutException(v.begin(), v.end());
	}

	using previous_future_type_boost = typename previous::future_type_boost;
	using previous_vector_type_boost = typename previous::vector_type_boost;
	using previous_type_boost = typename previous::type_boost;
	using future_type_boost = previous_type_boost;
	using vector_type_boost = std::vector<future_type_boost>;
	using type_boost = boost::future<vector_type_boost>;

	template<typename Func>
	static type_boost whenAllBoost(std::size_t size, Func &&f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::whenAllBoost(size, std::move(f)));
		}

		return boost::when_all(v.begin(), v.end());
	}

	template<typename Func>
	static type_boost whenAnyBoost(std::size_t size, Func &&f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::whenAnyBoost(size, std::move(f)));
		}

		return boost::when_any(v.begin(), v.end());
	}
};

template<typename T>
struct RecursiveCombinatorType<T, 1>
{
	static constexpr int level = 1;

	template<int size>
	static constexpr int nodes = get_power<size, 2>::value - 1;

	using future_type = folly::Future<T>;
	using vector_type = std::vector<folly::Try<T>>;
	using type = folly::Future<vector_type>;

	template<typename Func>
	static type collectAllFolly(std::size_t size, Func &&f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}

		return folly::collectAll(v.begin(), v.end());
	}

	using folly_collect_vector_type = std::vector<T>;
	using folly_collect_type = folly::Future<folly_collect_vector_type>;

	template<typename Func>
	static folly_collect_type collectFolly(std::size_t size, Func &&f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}

		return folly::collect(v.begin(), v.end());
	}

	using folly_collect_n_vector_value_type = std::pair<std::size_t, folly::Try<T>>;
	using folly_collect_n_vector_type = std::vector<folly_collect_n_vector_value_type>;
	using folly_collect_n_type = folly::Future<folly_collect_n_vector_type>;

	template<typename Func>
	static folly_collect_n_type collectNFolly(std::size_t size, Func &&f, int n)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}

		return folly::collectN(v.begin(), v.end(), n);
	}

	using folly_collect_any_pair_type = std::pair<size_t, folly::Try<T>>;
	using folly_collect_any_future_pair_type = folly::Future<folly_collect_any_pair_type>;

	template<typename Func>
	static folly_collect_any_future_pair_type collectAnyFolly(std::size_t size, Func &&f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}

		return folly::collectAny(v.begin(), v.end());
	}

	using folly_collect_any_without_exception_pair_type = std::pair<size_t, T>;
	using folly_collect_any_without_exception_future_pair_type = folly::Future<folly_collect_any_without_exception_pair_type>;

	template<typename Func>
	static folly_collect_any_without_exception_future_pair_type collectAnyWithoutExceptionFolly(std::size_t size, Func &&f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}

		return folly::collectAnyWithoutException(v.begin(), v.end());
	}

	using future_type_boost = boost::future<T>;
	using vector_type_boost = std::vector<future_type_boost>;
	using type_boost = boost::future<vector_type_boost>;

	template<typename Func>
	static type_boost whenAllBoost(std::size_t size, Func &&f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(boost::make_ready_future(f()));
		}

		return boost::when_all(v.begin(), v.end());
	}

	template<typename Func>
	static type_boost whenAnyBoost(std::size_t size, Func &&f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(boost::make_ready_future(f()));
		}

		return boost::when_any(v.begin(), v.end());
	}
};

template<typename T>
struct RecursiveCombinatorType<T, 0>
{
};

using TYPE = int;
const int VECTOR_SIZE = 2;
constexpr int TREE_LEVELS = 10;

BENCHMARK(FollyCollectAll)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::collectAllFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollect)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::collectFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollectN)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::collectNFolly(VECTOR_SIZE, [] () { return 3; }, VECTOR_SIZE / 2).wait();
}

BENCHMARK(FollyCollectAny)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::collectAnyFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollectAnyWithoutException)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::collectAnyWithoutExceptionFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(BoostWhenAll)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::whenAllBoost(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(BoostWhenAny)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::whenAnyBoost(VECTOR_SIZE, [] () { return 3; }).wait();
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}