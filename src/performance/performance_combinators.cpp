#include <boost/thread.hpp>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/InlineExecutor.h>

#include "extensions.h"
#include "advanced_futures_folly.h"
#include "advanced_futures_boost.h"
#include "tree.h"

template<typename T, typename Func>
folly::Future<T> follyCollectAll(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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
folly::Future<T> follyCollectN(folly::Executor *executor, std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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
			v.push_back(follyCollectN<T>(executor, treeHeight - 1, childNodes, f));
		}
	}

	return folly::collectN(v.begin(), v.end(), childNodes).via(executor).then([] (std::vector<std::pair<size_t, folly::Try<T>>> v) { return v[0].second.value(); });
}

template<typename T, typename Func>
folly::Future<T> follyCollectAny(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

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
adv_folly::Future<T> customFollyOrElse(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_folly::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFollyOrElse<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].orElse(std::move(v[1]));
}

template<typename T, typename Func>
adv_boost::Future<T> customBoostOrElse(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_boost::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_boost::Future<T>(boost::make_ready_future(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customBoostOrElse<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].orElse(std::move(v[1]));
}

template<typename T, typename Func>
adv_folly::Future<T> customFollyFirst(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_folly::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFollyFirst<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].first(std::move(v[1]));
}

template<typename T, typename Func>
adv_boost::Future<T> customBoostFirst(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_boost::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_boost::Future<T>(boost::make_ready_future(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customBoostFirst<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].first(std::move(v[1]));
}

template<typename T, typename Func>
adv_folly::Future<T> customFollyFirstSucc(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_folly::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFollyFirstSucc<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].firstSucc(std::move(v[1]));
}

template<typename T, typename Func>
adv_boost::Future<T> customBoostFirstSucc(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_boost::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_boost::Future<T>(boost::make_ready_future(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customBoostFirstSucc<T>(treeHeight - 1, childNodes, f));
		}
	}

	return v[0].firstSucc(std::move(v[1]));
}

template<typename T, typename Func>
adv_folly::Future<T> customFollyFirstN(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_folly::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFollyFirstN<T>(treeHeight - 1, childNodes, f));
		}
	}

	return adv_folly::firstN(std::move(v), childNodes).then([] (adv_folly::Try<std::vector<std::pair<std::size_t, adv_folly::Try<T>>>> t) { return t.get()[0].second.get(); });
}

template<typename T, typename Func>
adv_boost::Future<T> customBoostFirstN(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_boost::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_boost::Future<T>(boost::make_ready_future(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customBoostFirstN<T>(treeHeight - 1, childNodes, f));
		}
	}

	return adv_boost::firstN(std::move(v), childNodes).then([] (adv_boost::Try<std::vector<std::pair<std::size_t, adv_boost::Try<T>>>> t) { return t.get()[0].second.get(); });
}

template<typename T, typename Func>
adv_folly::Future<T> customFollyFirstNSucc(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_folly::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_folly::Future<T>(folly::makeFuture(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customFollyFirstNSucc<T>(treeHeight - 1, childNodes, f));
		}
	}

	return adv_folly::firstNSucc(std::move(v), childNodes).then([] (adv_folly::Try<std::vector<std::pair<std::size_t, T>>> t) { return t.get()[0].second; });
}

template<typename T, typename Func>
adv_boost::Future<T> customBoostFirstNSucc(std::size_t treeHeight, std::size_t childNodes, Func f)
{
	std::vector<adv_boost::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(adv_boost::Future<T>(boost::make_ready_future(f())));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(customBoostFirstNSucc<T>(treeHeight - 1, childNodes, f));
		}
	}

	return adv_boost::firstNSucc(std::move(v), childNodes).then([] (adv_boost::Try<std::vector<std::pair<std::size_t, T>>> t) { return t.get()[0].second; });
}

inline int initFuture()
{
	return 3;
}

BENCHMARK(FollyCollectAll)
{
	follyCollectAll<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(FollyCollect)
{
	follyCollect<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS,  initFuture).wait();
}

BENCHMARK(FollyCollectN)
{
	// TODO Why does folly::collectN produce a foilly::SemiFuture and not a folly::Future like the other non-blocking combinators?
	folly::InlineExecutor ex;
	follyCollectN<TREE_TYPE>(&ex, TREE_HEIGHT, TREE_CHILDS,  initFuture).wait();
}

BENCHMARK(FollyCollectAny)
{
	follyCollectAny<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS,  initFuture).wait();
}

BENCHMARK(FollyCollectAnyWithoutException)
{
	follyCollectAnyWithoutException<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(BoostWhenAll)
{
	boostWhenAll<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS,  initFuture).wait();
}

BENCHMARK(BoostWhenAny)
{
	boostWhenAny<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS,  initFuture).wait();
}

BENCHMARK(CustomCollectNWithoutException)
{
	customCollectNWithoutException<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS,  initFuture).wait();
}

BENCHMARK(AdvFollyOrElse)
{
	customFollyOrElse<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvBoostOrElse)
{
	customBoostOrElse<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvFollyFirst)
{
	customFollyFirst<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvBoostFirst)
{
	customBoostFirst<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvFollyFirstSucc)
{
	customFollyFirstSucc<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvBoostFirstSucc)
{
	customBoostFirstSucc<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvFollyFirstN)
{
	customFollyFirstN<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvBoostFirstN)
{
	customBoostFirstN<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvFollyFirstNSucc)
{
	customFollyFirstNSucc<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvBoostFirstNSucc)
{
	customBoostFirstNSucc<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}