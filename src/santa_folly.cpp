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

using CollectionType = std::vector<std::pair<size_t, Type>>;

inline void decideFolly(CollectionType matches)
{
	decide(matches[0].second);

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
		auto reindeer = folly::via(&executor, createReindeer).then([]
			(std::vector<folly::Future<Type>> collection)
			{
				return collectNWithoutException(std::move(collection), REINDEER_MATCH_NUMBER).get();
			}
		);
		auto elves = folly::via(&executor, createElves).then([]
			(std::vector<folly::Future<Type>> collection)
			{
				return collectNWithoutException(std::move(collection), ELF_MATCH_NUMBER).get();
			}
		);

		auto group = orElse(std::move(reindeer), std::move(elves));
		auto x = group.then(decideFolly);
		x.wait();
		santaDoesWork();
	}

	std::cout << "Reindeer matches: " << reindeerMatches << std::endl;
	std::cout << "Elf matches: " << elfMatches << std::endl;

	return 0;
}