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
template<typename T, int n>
struct RecursiveCombinatorType
{
	static constexpr int level = n;

	/**
	 * The number of future nodes in the tree started at this level.
	 * Note that the root node is included.
	 * \tparam size The number of futures per level.
	 */
	template<int size>
	static constexpr int nodes = get_power<size, n + 1>::value - 1;

	using previous = RecursiveCombinatorType<T, n - 1>;

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

	using previous_pair_type = typename previous::pair_type;
	using previous_future_pair_type = typename previous::future_pair_type;
	using pair_type = std::pair<size_t, folly::Try<previous_pair_type>>;
	using future_pair_type = folly::Future<pair_type>;

	template<typename Func>
	static future_pair_type collectAnyFolly(std::size_t size, Func &&f)
	{
		std::vector<previous_future_pair_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAnyFolly(size, std::move(f)));
		}

		return folly::collectAny(v.begin(), v.end());
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

	using pair_type = std::pair<size_t, folly::Try<T>>;
	using future_pair_type = folly::Future<pair_type>;

	template<typename Func>
	static future_pair_type collectAnyFolly(std::size_t size, Func &&f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}

		return folly::collectAny(v.begin(), v.end());
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

BENCHMARK(FollyCollectAny)
{
	RecursiveCombinatorType<TYPE, TREE_LEVELS>::collectAnyFolly(VECTOR_SIZE, [] () { return 3; }).wait();
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