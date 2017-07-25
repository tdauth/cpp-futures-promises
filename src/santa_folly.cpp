#include <folly/init/Init.h>
#include <folly/futures/Future.h>

#include <wangle/concurrent/CPUThreadPoolExecutor.h>

#include "santa.h"

folly::Future<folly::Unit> sleep(int min, int max)
{
	auto duration = folly::Duration(random_number(min, max) * 10);

	return folly::futures::sleep(duration);
}

inline folly::Future<Type> reindeerReady()
{
	return sleep(REINDEER_SLEEP_MIN, REINDEER_SLEEP_MAX).then(reindeer);
}

inline folly::Future<Type> elfReady()
{
	return sleep(ELF_SLEEP_MIN, ELF_SLEEP_MAX).then(elf);
}

std::vector<folly::Future<Type>> createReindeer()
{
	std::vector<folly::Future<Type>> reindeer;
	reindeer.reserve(REINDEER_NUMBER);

	for (std::size_t i = 0; i < REINDEER_NUMBER; ++i)
	{
		reindeer.push_back(reindeerReady());
	}

	return reindeer;
}

std::vector<folly::Future<Type>> createElves()
{
	std::vector<folly::Future<Type>> elves;
	elves.reserve(ELF_NUMBER);

	for (std::size_t i = 0; i < ELF_NUMBER; ++i)
	{
		elves.push_back(elfReady());
	}

	return elves;
}

using CollectionType = std::vector<std::pair<size_t, folly::Try<Type>>>;

inline CollectionType collectOneTypeFromN(std::vector<folly::Future<Type>> collection, int n)
{
	return folly::collectN(collection, n).get();
}

inline void decideFolly(std::pair<std::size_t, folly::Try<CollectionType>> p)
{
	CollectionType matches = p.second.value();
	decide(matches[0].second.value());

	std::cout << "With members: ";

	for (std::size_t i = 0; i < matches.size(); ++i)
	{
		std::cout << matches[i].first;

		if (i < matches.size() - 1)
		{
			std::cout << ", ";
		}
	}

	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	folly::init(&argc, &argv);

	wangle::CPUThreadPoolExecutor executor(std::thread::hardware_concurrency());

	for (int i = 0; i < RUNS_NUMBER; ++i)
	{
		auto reindeer = folly::via(&executor, createReindeer).then(std::bind(collectOneTypeFromN, std::placeholders::_1, REINDEER_MATCH_NUMBER));
		auto elves = folly::via(&executor, createElves).then(std::bind(collectOneTypeFromN, std::placeholders::_1, ELF_MATCH_NUMBER));
		std::vector<folly::Future<CollectionType>> futures;
		futures.push_back(std::move(reindeer));
		futures.push_back(std::move(elves));
		auto group = folly::collectAny(futures.begin(), futures.end()); // TODO prefer reindeer if both have enough ready
		auto x = group.then(decideFolly);
		x.wait();
		santa_does_work();
	}

	std::cout << "Reindeer matches: " << reindeerMatches << std::endl;
	std::cout << "Elf matches: " << elfMatches << std::endl;

	return 0;
}
