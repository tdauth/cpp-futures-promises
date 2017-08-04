#include <boost/thread.hpp>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/futures/Future.h>

#include "extensions.h"
#include "advanced_futures_folly.h"
#include "tree.h"

template<typename T, typename Func>
folly::Future<T> follyCollectAll(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(follyCollectAll<T>(treeHeight - 1, childNodes, f));
		}
	}

	return folly::collectAll(v.begin(), v.end()).then([] (std::vector<folly::Try<T>> v) { return v[0].value(); });
}

template<typename T, typename Func>
folly::Future<T> follyCollect(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(follyCollect<T>(treeHeight - 1, childNodes, f));
		}
	}

	return folly::collect(v.begin(), v.end()).then([] (std::vector<T> v) { return v[0]; });
}

template<typename T, typename Func>
folly::Future<T> follyCollectN(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(follyCollectN<T>(treeHeight - 1, childNodes, f));
		}
	}

	return folly::collectN(v.begin(), v.end(), childNodes).then([] (std::vector<std::pair<size_t, folly::Try<T>>> v) { return v[0].second.value(); });
}

template<typename T, typename Func>
folly::Future<T> follyCollectAny(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(follyCollectAny<T>(treeHeight - 1, childNodes, f));
		}
	}

	return folly::collectAny(v.begin(), v.end()).then([] (std::pair<size_t, folly::Try<T>> v) { return v.second.value(); });
}

template<typename T, typename Func>
folly::Future<T> follyCollectAnyWithoutException(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(follyCollectAnyWithoutException<T>(treeHeight - 1, childNodes, f));
		}
	}

	return folly::collectAnyWithoutException(v.begin(), v.end()).then([] (std::pair<size_t, T> v) { return v.second; });
}

template<typename T, typename Func>
boost::future<T> boostWhenAll(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<boost::future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(boost::make_ready_future(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(boostWhenAll<T>(treeHeight - 1, childNodes, f));
		}
	}

	return boost::when_all(v.begin(), v.end()).then([] (boost::future<std::vector<boost::future<T>>> v) { return v.get()[0].get(); });
}

template<typename T, typename Func>
boost::future<T> boostWhenAny(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<boost::future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(boost::make_ready_future(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(boostWhenAny<T>(treeHeight - 1, childNodes, f));
		}
	}

	return boost::when_any(v.begin(), v.end()).then([] (boost::future<std::vector<boost::future<T>>> v) { return v.get()[0].get(); });
}

template<typename T, typename Func>
folly::Future<T> customCollectNWithoutException(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(folly::makeFuture(f()));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customCollectNWithoutException<T>(treeHeight - 1, childNodes, f));
		}
	}

	return collectNWithoutException(v.begin(), v.end(), childNodes).then([] (std::vector<std::pair<size_t, T>> v) { return v[0].second; });
}

template<typename T, typename Func>
adv::Future<T> customOrElse(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customOrElse<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].orElse(std::move(v[1]));
}

template<typename T, typename Func>
adv::Future<T> customFirst(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFirst<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].first(std::move(v[1]));
}

template<typename T, typename Func>
adv::Future<T> customFirstSucc(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFirstSucc<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].firstSucc(std::move(v[1]));
}

template<typename T, typename Func>
adv::Future<T> customFirstN(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFirstN<T>(treeHeight - 1, childNodes, f));
		}
	}

	return adv::firstN(std::move(v), childNodes).then([] (adv::Try<std::vector<std::pair<std::size_t, adv::Try<T>>>> t) { return t.get()[0].second.get(); });
}

template<typename T, typename Func>
adv::Future<T> customFirstNSucc(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFirstNSucc<T>(treeHeight - 1, childNodes, f));
		}
	}

	return adv::firstNSucc(std::move(v), childNodes).then([] (adv::Try<std::vector<std::pair<std::size_t, T>>> t) { return t.get()[0].second; });
}

BENCHMARK(FollyCollectAll)
{
	follyCollectAll<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(FollyCollect)
{
	follyCollect<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(FollyCollectN)
{
	follyCollectN<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(FollyCollectAny)
{
	follyCollectAny<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(FollyCollectAnyWithoutException)
{
	follyCollectAnyWithoutException<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(BoostWhenAll)
{
	boostWhenAll<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(BoostWhenAny)
{
	boostWhenAny<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(CustomCollectNWithoutException)
{
	customCollectNWithoutException<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(CustomOrElse)
{
	customOrElse<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(CustomFirst)
{
	customFirst<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(CustomFirstSucc)
{
	customFirstSucc<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(CustomFirstN)
{
	customFirstN<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

BENCHMARK(CustomFirstNSucc)
{
	customFirstNSucc<TREE_TYPE>(TREE_LEVELS, TREE_VECTOR_SIZE,  [] () { return 3; });
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}