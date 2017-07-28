#ifndef RANDOM_H
#define RANDOM_H

#include <random>

/**
 * Calculates a random number between \p min and \p max (inclusive boundaries).
 * \param min The lower inclusive bound.
 * \param max The upper inclusive bound.
 * \return Returns the calculated random number.
 */
int randomNumber(int min, int max)
{
	static std::random_device rand;
	static std::default_random_engine e(rand());
	std::uniform_int_distribution<int> uniform_dist(min, max);

	return uniform_dist(e);
}

#endif