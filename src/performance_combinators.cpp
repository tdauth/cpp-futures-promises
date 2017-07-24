#include <boost/thread.hpp>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/futures/Future.h>

#include "extensions.h"

/**
 * \brief This structure allows to generate recursive combinator calls at compile time.
 *
 * Use the following code to call folly::collectAll() 5 * 10 times, adding completed futures with the value 3 add the deepest level
 * of the tree:
 * \code
 * RecursiveCombinatorType<int, 5>::collectAllFolly(10, [] { return 3; });
 * \endcode
 *
 * \note Be careful with higher numbers of nesting which may lead to very long execution times!
 */
template<typename T, int n>
struct RecursiveCombinatorType
{
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
};

template<typename T>
struct RecursiveCombinatorType<T, 0>
{
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
};

using TYPE = int;
const int VECTOR_SIZE = 10;

BENCHMARK(FollyCollectAll0)
{
	RecursiveCombinatorType<TYPE, 0>::collectAllFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollectAll5)
{
	RecursiveCombinatorType<TYPE, 5>::collectAllFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}