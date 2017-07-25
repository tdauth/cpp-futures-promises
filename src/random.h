#ifndef RANDOM_H
#define RANDOM_H

#include <random>

/**
 * Chooses a random number between 0 and 1.
 */
int select()
{
	static std::random_device rand;
	static std::default_random_engine e(rand());
	std::uniform_int_distribution<int> uniform_dist(0, 1);

	return uniform_dist(e);
}

int random_number(int min, int max)
{
	static std::random_device rand;
	static std::default_random_engine e(rand());
	std::uniform_int_distribution<int> uniform_dist(min, max);

	return uniform_dist(e);
}

#endif