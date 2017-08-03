#include <boost/thread.hpp>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/futures/Future.h>

#include "extensions.h"

#include "advanced_futures_folly.h"

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
	 * TODO the formula is wrong for only 1 node per level?
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
	static type collectAllFolly(std::size_t size, Func f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAllFolly(size, f));
		}

		return folly::collectAll(v.begin(), v.end());
	}

	using previous_folly_collect_type = typename previous::folly_collect_type;

	using folly_collect_vector_type = std::vector<typename previous_folly_collect_type::value_type>;
	using folly_collect_type = folly::Future<folly_collect_vector_type>;

	template<typename Func>
	static folly_collect_type collectFolly(std::size_t size, Func f)
	{
		std::vector<previous_folly_collect_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectFolly(size, f));
		}

		return folly::collect(v.begin(), v.end());
	}

	using previous_folly_collect_n_type = typename previous::folly_collect_n_type;

	using folly_collect_n_vector_value_type = std::pair<std::size_t, folly::Try<typename previous_folly_collect_n_type::value_type>>;
	using folly_collect_n_vector_type = std::vector<folly_collect_n_vector_value_type>;
	using folly_collect_n_type = folly::Future<folly_collect_n_vector_type>;

	template<typename Func>
	static folly_collect_n_type collectNFolly(std::size_t size, Func f, int n)
	{
		std::vector<previous_folly_collect_n_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectNFolly(size, f, n));
		}

		return folly::collectN(v.begin(), v.end(), n);
	}

	using folly_collect_any_previous_pair_type = typename previous::folly_collect_any_pair_type;
	using folly_collect_any_previous_future_pair_type = typename previous::folly_collect_any_future_pair_type;
	using folly_collect_any_pair_type = std::pair<size_t, folly::Try<folly_collect_any_previous_pair_type>>;
	using folly_collect_any_future_pair_type = folly::Future<folly_collect_any_pair_type>;

	template<typename Func>
	static folly_collect_any_future_pair_type collectAnyFolly(std::size_t size, Func f)
	{
		std::vector<folly_collect_any_previous_future_pair_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAnyFolly(size, f));
		}

		return folly::collectAny(v.begin(), v.end());
	}

	using folly_collect_any_without_exception_previous_future_pair_type = typename previous::folly_collect_any_without_exception_future_pair_type;

	using folly_collect_any_without_exception_pair_type = std::pair<size_t, typename folly_collect_any_without_exception_previous_future_pair_type::value_type>;
	using folly_collect_any_without_exception_future_pair_type = folly::Future<folly_collect_any_without_exception_pair_type>;

	template<typename Func>
	static folly_collect_any_without_exception_future_pair_type collectAnyWithoutExceptionFolly(std::size_t size, Func f)
	{
		std::vector<folly_collect_any_without_exception_previous_future_pair_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectAnyWithoutExceptionFolly(size, f));
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
	static type_boost whenAllBoost(std::size_t size, Func f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::whenAllBoost(size, f));
		}

		return boost::when_all(v.begin(), v.end());
	}

	template<typename Func>
	static type_boost whenAnyBoost(std::size_t size, Func f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::whenAnyBoost(size, f));
		}

		return boost::when_any(v.begin(), v.end());
	}

	template<typename Func>
	static adv::Future<T> orElseCustom(std::size_t size, Func f)
	{
		return previous::orElseCustom(size, f).orElse(previous::orElseCustom(size, f));
	}

	using previous_custom_collect_n_without_exception_type = typename previous::custom_collect_n_without_exception_type;

	using custom_collect_n_without_exception_vector_value_type = std::pair<std::size_t, typename previous_custom_collect_n_without_exception_type::value_type>;
	using custom_collect_n_without_exception_vector_type = std::vector<custom_collect_n_without_exception_vector_value_type>;
	using custom_collect_n_without_exception_type = folly::Future<custom_collect_n_without_exception_vector_type>;

	template<typename Func>
	static custom_collect_n_without_exception_type collectNWithoutExceptionCustom(std::size_t size, Func f, int n)
	{
		std::vector<previous_custom_collect_n_without_exception_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(previous::collectNWithoutExceptionCustom(size, f, n));
		}

		return collectNWithoutException(v.begin(), v.end(), n);
	}

	template<typename Func>
	static adv::Future<T> firstCustom(std::size_t size, Func f)
	{
		return previous::firstCustom(size, f).first(previous::firstCustom(size, f));
	}

	template<typename Func>
	static adv::Future<T> firstSuccCustom(std::size_t size, Func f)
	{
		return previous::firstSuccCustom(size, f).firstSucc(previous::firstSuccCustom(size, f));
	}

	using previous_first_n_result_type_type = typename previous::first_n_result_type_type;
	using previous_first_n_result_type = typename previous::first_n_result_type;

	using first_n_result_type_type = std::vector<std::pair<std::size_t, adv::Try<previous_first_n_result_type_type>>>;
	using first_n_result_type = adv::Future<first_n_result_type_type>;

	template<typename Func>
	static first_n_result_type firstN(std::size_t size, Func f)
	{
		std::vector<previous_first_n_result_type> futures;
		futures.push_back(previous::firstN(size, f));
		futures.push_back(previous::firstN(size, f));

		return adv::firstN(std::move(futures), 2);
	}

	using previous_first_n_succ_result_type_type = typename previous::first_n_succ_result_type_type;
	using previous_first_n_succ_result_type = typename previous::first_n_succ_result_type;

	using first_n_succ_result_type_type = std::vector<std::pair<std::size_t, previous_first_n_succ_result_type_type>>;
	using first_n_succ_result_type = adv::Future<first_n_succ_result_type_type>;

	template<typename Func>
	static first_n_succ_result_type firstNSucc(std::size_t size, Func f)
	{
		std::vector<previous_first_n_succ_result_type> futures;
		futures.push_back(previous::firstNSucc(size, f));
		futures.push_back(previous::firstNSucc(size, f));

		return adv::firstNSucc(std::move(futures), 2);
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
	static type collectAllFolly(std::size_t size, Func f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFutureWith(f));
		}

		return folly::collectAll(v.begin(), v.end());
	}

	using folly_collect_vector_type = std::vector<T>;
	using folly_collect_type = folly::Future<folly_collect_vector_type>;

	template<typename Func>
	static folly_collect_type collectFolly(std::size_t size, Func f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFutureWith(f));
		}

		return folly::collect(v.begin(), v.end());
	}

	using folly_collect_n_vector_value_type = std::pair<std::size_t, folly::Try<T>>;
	using folly_collect_n_vector_type = std::vector<folly_collect_n_vector_value_type>;
	using folly_collect_n_type = folly::Future<folly_collect_n_vector_type>;

	template<typename Func>
	static folly_collect_n_type collectNFolly(std::size_t size, Func f, int n)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFutureWith(f));
		}

		return folly::collectN(v.begin(), v.end(), n);
	}

	using folly_collect_any_pair_type = std::pair<size_t, folly::Try<T>>;
	using folly_collect_any_future_pair_type = folly::Future<folly_collect_any_pair_type>;

	template<typename Func>
	static folly_collect_any_future_pair_type collectAnyFolly(std::size_t size, Func f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFutureWith(f));
		}

		return folly::collectAny(v.begin(), v.end());
	}

	using folly_collect_any_without_exception_pair_type = std::pair<size_t, T>;
	using folly_collect_any_without_exception_future_pair_type = folly::Future<folly_collect_any_without_exception_pair_type>;

	template<typename Func>
	static folly_collect_any_without_exception_future_pair_type collectAnyWithoutExceptionFolly(std::size_t size, Func f)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFutureWith(f));
		}

		return folly::collectAnyWithoutException(v.begin(), v.end());
	}

	using future_type_boost = boost::future<T>;
	using vector_type_boost = std::vector<future_type_boost>;
	using type_boost = boost::future<vector_type_boost>;

	template<typename Func>
	static type_boost whenAllBoost(std::size_t size, Func f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(boost::async(f));
		}

		return boost::when_all(v.begin(), v.end());
	}

	template<typename Func>
	static type_boost whenAnyBoost(std::size_t size, Func f)
	{
		vector_type_boost v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(boost::async(f));
		}

		return boost::when_any(v.begin(), v.end());
	}

	template<typename Func>
	static adv::Future<T> orElseCustom(std::size_t size, Func f)
	{
		return adv::Future<T>(folly::makeFutureWith(f)).orElse(adv::Future<T>(folly::makeFutureWith(f)));
	}

	using custom_collect_n_without_exception_vector_value_type = std::pair<std::size_t, T>;
	using custom_collect_n_without_exception_vector_type = std::vector<custom_collect_n_without_exception_vector_value_type>;
	using custom_collect_n_without_exception_type = folly::Future<custom_collect_n_without_exception_vector_type>;

	template<typename Func>
	static custom_collect_n_without_exception_type collectNWithoutExceptionCustom(std::size_t size, Func f, int n)
	{
		std::vector<future_type> v;

		for (std::size_t i = 0; i < size; ++i)
		{
			v.push_back(folly::makeFutureWith(f));
		}

		return collectNWithoutException(v.begin(), v.end(), n);
	}

	template<typename Func>
	static adv::Future<T> firstCustom(std::size_t size, Func f)
	{
		return adv::Future<T>(folly::makeFutureWith(f)).first(adv::Future<T>(folly::makeFutureWith(f)));
	}

	template<typename Func>
	static adv::Future<T> firstSuccCustom(std::size_t size, Func f)
	{
		return adv::Future<T>(folly::makeFutureWith(f)).firstSucc(adv::Future<T>(folly::makeFutureWith(f)));
	}

	using first_n_result_type_type = std::vector<std::pair<std::size_t, adv::Try<T>>>;
	using first_n_result_type = adv::Future<first_n_result_type_type>;

	template<typename Func>
	static first_n_result_type firstN(std::size_t size, Func f)
	{
		std::vector<adv::Future<T>> futures;
		futures.push_back(adv::Future<T>(folly::makeFutureWith(f)));
		futures.push_back(adv::Future<T>(folly::makeFutureWith(f)));

		return adv::firstN(std::move(futures), 2);
	}

	using first_n_succ_result_type_type = std::vector<std::pair<std::size_t, T>>;
	using first_n_succ_result_type = adv::Future<first_n_succ_result_type_type>;

	template<typename Func>
	static first_n_succ_result_type firstNSucc(std::size_t size, Func f)
	{
		std::vector<adv::Future<T>> futures;
		futures.push_back(adv::Future<T>(folly::makeFutureWith(f)));
		futures.push_back(adv::Future<T>(folly::makeFutureWith(f)));

		return adv::firstNSucc(std::move(futures), 2);
	}
};

template<typename T>
struct RecursiveCombinatorType<T, 0>
{
};

using TYPE = int;
constexpr int VECTOR_SIZE = 2;
constexpr int TREE_LEVELS = 12;
static_assert(VECTOR_SIZE == 2, "The custom combinators only support passing two futures.");

using Tree = RecursiveCombinatorType<TYPE, TREE_LEVELS>;

BENCHMARK(FollyCollectAll)
{
	Tree::collectAllFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollect)
{
	Tree::collectFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollectN)
{
	Tree::collectNFolly(VECTOR_SIZE, [] () { return 3; }, VECTOR_SIZE).wait();
}

BENCHMARK(FollyCollectAny)
{
	Tree::collectAnyFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(FollyCollectAnyWithoutException)
{
	Tree::collectAnyWithoutExceptionFolly(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(BoostWhenAll)
{
	Tree::whenAllBoost(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(BoostWhenAny)
{
	Tree::whenAnyBoost(VECTOR_SIZE, [] () { return 3; }).wait();
}

BENCHMARK(CustomCollectNWithoutException)
{
	Tree::collectNWithoutExceptionCustom(VECTOR_SIZE, [] () { return 3; }, VECTOR_SIZE).wait();
}

BENCHMARK(CustomOrElse)
{
	Tree::orElseCustom(VECTOR_SIZE, [] () { return 3; }).get();
}

BENCHMARK(CustomFirst)
{
	Tree::firstCustom(VECTOR_SIZE, [] () { return 3; }).get();
}

BENCHMARK(CustomFirstSucc)
{
	Tree::firstSuccCustom(VECTOR_SIZE, [] () { return 3; }).get();
}

BENCHMARK(CustomFirstN)
{
	Tree::firstN(VECTOR_SIZE, [] () { return 3; }).get();
}

BENCHMARK(CustomFirstNSucc)
{
	Tree::firstNSucc(VECTOR_SIZE, [] () { return 3; }).get();
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}