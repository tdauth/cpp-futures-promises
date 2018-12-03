#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

#include "santa.h"

folly::Future<folly::Unit> sleep(int min, int max)
{
	auto duration = folly::Duration(randomNumber(min, max) * 10);

	return folly::futures::sleep(duration);
}

inline folly::Future<Type> reindeerReady(folly::Executor *executor)
{
    return sleep(REINDEER_SLEEP_MIN, REINDEER_SLEEP_MAX).via(executor).thenValue([](folly::Unit v) { return reindeer(); });
}

inline folly::Future<Type> elfReady(folly::Executor *executor)
{
    return sleep(ELF_SLEEP_MIN, ELF_SLEEP_MAX).via(executor).thenValue([](folly::Unit v) { return elf(); });
}

std::vector<folly::Future<Type>> createReindeer(folly::Executor *executor)
{
	std::vector<folly::Future<Type>> reindeer;
	reindeer.reserve(REINDEER_NUMBER);

	for (std::size_t i = 0; i < REINDEER_NUMBER; ++i)
	{
		reindeer.push_back(reindeerReady(executor));
	}

	return reindeer;
}

std::vector<folly::Future<Type>> createElves(folly::Executor *executor)
{
	std::vector<folly::Future<Type>> elves;
	elves.reserve(ELF_NUMBER);

	for (std::size_t i = 0; i < ELF_NUMBER; ++i)
	{
		elves.push_back(elfReady(executor));
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

	folly::CPUThreadPoolExecutor executor(std::thread::hardware_concurrency());

	for (int i = 0; i < RUNS_NUMBER; ++i)
	{
		auto reindeer = collectNWithoutException(createReindeer(&executor), REINDEER_MATCH_NUMBER);
		auto elves = collectNWithoutException(createElves(&executor), ELF_MATCH_NUMBER);
		// TODO Actually we should use collectAnyWithoutException, reindeer are only preferred if both arrive at the same time.
		auto group = orElse(std::move(reindeer), std::move(elves));
		auto x = std::move(group).thenValue(decideFolly);
		x.wait();
		santaDoesWork();
	}

	std::cout << "Reindeer matches: " << reindeerMatches << std::endl;
	std::cout << "Elf matches: " << elfMatches << std::endl;

	return 0;
}
