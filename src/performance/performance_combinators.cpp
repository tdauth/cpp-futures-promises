#include <boost/thread.hpp>

#include <folly/Benchmark.h>
#include <folly/executors/InlineExecutor.h>
#include <folly/futures/Future.h>
#include <folly/init/Init.h>

#include "advanced_futures_promises.h"

using TREE_TYPE = int;
constexpr int TREE_HEIGHT = 12;
constexpr int TREE_CHILDS = 2;
static_assert(TREE_CHILDS == 2,
              "The custom combinators only support passing two futures.");

template <typename T, typename Func>
folly::Future<T> follyCollectAll(std::size_t treeHeight, std::size_t childNodes,
                                 Func f)
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

	return folly::collectAll(v.begin(), v.end())
	    .thenValue([](std::vector<folly::Try<T>> v) { return v[0].value(); });
}

template <typename T, typename Func>
folly::Future<T> follyCollect(std::size_t treeHeight, std::size_t childNodes,
                              Func f)
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

	return folly::collect(v.begin(), v.end()).thenValue([](std::vector<T> v) {
		return v[0];
	});
}

template <typename T, typename Func>
folly::Future<T> follyCollectN(folly::Executor *executor,
                               std::size_t treeHeight, std::size_t childNodes,
                               Func f)
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

	return folly::collectN(v.begin(), v.end(), childNodes)
	    .via(executor)
	    .thenValue([](std::vector<std::pair<size_t, folly::Try<T>>> v) {
		    return v[0].second.value();
	    });
}

template <typename T, typename Func>
folly::Future<T> follyCollectAny(std::size_t treeHeight, std::size_t childNodes,
                                 Func f)
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

	return folly::collectAny(v.begin(), v.end())
	    .thenValue(
	        [](std::pair<size_t, folly::Try<T>> v) { return v.second.value(); });
}

template <typename T, typename Func>
folly::Future<T> follyCollectAnyWithoutException(folly::Executor *executor,
                                                 std::size_t treeHeight,
                                                 std::size_t childNodes, Func f)
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
			v.push_back(follyCollectAnyWithoutException<T>(executor, treeHeight - 1,
			                                               childNodes, f));
		}
	}

	return folly::collectAnyWithoutException(v.begin(), v.end())
	    .via(executor)
	    .thenValue([](std::pair<size_t, T> v) { return v.second; });
}

template <typename T, typename Func>
boost::future<T> boostWhenAll(std::size_t treeHeight, std::size_t childNodes,
                              Func f)
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

	return boost::when_all(v.begin(), v.end())
	    .then([](boost::future<std::vector<boost::future<T>>> v) {
		    return v.get()[0].get();
	    });
}

template <typename T, typename Func>
boost::future<T> boostWhenAny(std::size_t treeHeight, std::size_t childNodes,
                              Func f)
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

	return boost::when_any(v.begin(), v.end())
	    .then([](boost::future<std::vector<boost::future<T>>> v) {
		    return v.get()[0].get();
	    });
}

template <typename T, typename Func>
adv::Future<T> advFirstN(folly::Executor *ex, std::size_t treeHeight,
                         std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			adv::Promise<T> p(ex);
			p.trySuccess(f());
			auto f = p.future();
			v.push_back(std::move(f));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(advFirstN<T, Func>(ex, treeHeight - 1, childNodes, f));
		}
	}

	using ResultType = std::vector<std::pair<std::size_t, adv::Try<T>>>;
	adv::Future<ResultType> first = adv::firstN<T>(ex, std::move(v), childNodes);
	adv::Future<T> r = first.then(
	    [](const adv::Try<ResultType> &t) { return t.get()[0].second.get(); });

	return r;
}

template <typename T, typename Func>
adv::Future<T> advFirstNSucc(folly::Executor *ex, std::size_t treeHeight,
                             std::size_t childNodes, Func f)
{
	std::vector<adv::Future<T>> v;

	BENCHMARK_SUSPEND
	{
		v.reserve(childNodes);
	}

	if (treeHeight == 0)
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			adv::Promise<T> p(ex);
			p.trySuccess(f());
			auto f = p.future();
			v.push_back(std::move(f));
		}
	}
	else
	{
		for (std::size_t i = 0; i < childNodes; ++i)
		{
			v.push_back(advFirstNSucc<T, Func>(ex, treeHeight - 1, childNodes, f));
		}
	}

	using ResultType = std::vector<std::pair<std::size_t, T>>;
	adv::Future<ResultType> first =
	    adv::firstNSucc<T>(ex, std::move(v), childNodes);
	adv::Future<T> r = first.then(
	    [](const adv::Try<ResultType> &t) { return t.get()[0].second; });

	return r;
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
	follyCollect<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(FollyCollectN)
{
	// TODO Why does folly::collectN produce a foilly::SemiFuture and not a
	// folly::Future like the other non-blocking combinators?
	folly::InlineExecutor ex;
	follyCollectN<TREE_TYPE>(&ex, TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(FollyCollectAny)
{
	follyCollectAny<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(FollyCollectAnyWithoutException)
{
	// TODO Why does folly::collectAnyWithoutException produce a foilly::SemiFuture
	// and not a folly::Future like the other non-blocking combinators?
	folly::InlineExecutor ex;
	follyCollectAnyWithoutException<TREE_TYPE>(&ex, TREE_HEIGHT, TREE_CHILDS,
	                                           initFuture)
	    .wait();
}

BENCHMARK(BoostWhenAll)
{
	boostWhenAll<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(BoostWhenAny)
{
	boostWhenAny<TREE_TYPE>(TREE_HEIGHT, TREE_CHILDS, initFuture).wait();
}

BENCHMARK(AdvFirstN)
{
	folly::InlineExecutor ex;
	advFirstN<TREE_TYPE>(&ex, TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

BENCHMARK(AdvFirstNSucc)
{
	folly::InlineExecutor ex;
	advFirstNSucc<TREE_TYPE>(&ex, TREE_HEIGHT, TREE_CHILDS, initFuture).get();
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);

	folly::runBenchmarks();

	return 0;
}
