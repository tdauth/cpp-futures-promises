#include <boost/thread.hpp>

#include "santa.h"

boost::chrono::milliseconds sleep(int min, int max)
{
	return boost::chrono::milliseconds(randomNumber(min, max) * 10);
}

void setReindeer(boost::promise<Type> &&p)
{
	p.set_value(reindeer());
}

inline boost::future<Type> reindeerReady()
{
	boost::packaged_task<Type()> p(reindeer);
	auto r = p.get_future();
	boost::thread t([p = std::move(p)] () mutable { boost::this_thread::sleep_for(sleep(REINDEER_SLEEP_MIN, REINDEER_SLEEP_MAX)); p(); });
	t.detach();

	return r;
}

inline boost::future<Type> elfReady()
{
	boost::packaged_task<Type()> p(elf);
	auto r = p.get_future();
	boost::thread t([p = std::move(p)] () mutable { boost::this_thread::sleep_for(sleep(ELF_SLEEP_MIN, ELF_SLEEP_MAX)); p(); });
	t.detach();

	return r;
}

std::vector<boost::future<Type>> createReindeer()
{
	std::vector<boost::future<Type>> reindeer;
	reindeer.reserve(REINDEER_NUMBER);

	for (std::size_t i = 0; i < REINDEER_NUMBER; ++i)
	{
		reindeer.push_back(reindeerReady());
	}

	return reindeer;
}

std::vector<boost::future<Type>> createElves()
{
	std::vector<boost::future<Type>> elves;
	elves.reserve(ELF_NUMBER);

	for (std::size_t i = 0; i < ELF_NUMBER; ++i)
	{
		elves.push_back(elfReady());
	}

	return elves;
}

using CollectionType = std::vector<std::pair<std::size_t, Type>>;

inline void decideBoost(boost::future<CollectionType> f)
{
	CollectionType matches = f.get();
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

int main()
{
	/*
	 * Don't use an executor since the resubmitting with a delay does not really work with futures/boost::async().
	 * Just create a thread per elf and reindeer and sleep for the delay.
	 */
	for (int i = 0; i < RUNS_NUMBER; ++i)
	{
		auto reindeer = boost::async(createReindeer).then([]
			(boost::future<std::vector<boost::future<Type>>> collection)
			{
				auto c = collection.get();

				return whenNSucc(c.begin(), c.end(), REINDEER_MATCH_NUMBER).get();
			}
		);
		auto elves = boost::async(createElves).then([]
			(boost::future<std::vector<boost::future<Type>>> collection)
			{
				auto c = collection.get();

				return whenNSucc(c.begin(), c.end(), ELF_MATCH_NUMBER).get();
			}
		);

		auto group = orElse(std::move(reindeer), std::move(elves));
		auto x = group.then(decideBoost);
		x.wait();
		santaDoesWork();
	}

	std::cout << "Reindeer matches: " << reindeerMatches << std::endl;
	std::cout << "Elf matches: " << elfMatches << std::endl;

	return 0;
}