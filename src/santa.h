#ifndef SANTA_H
#define SANTA_H

/**
 * A variation of the Santa Claus problem described by John A. Trono:
 * https://crsr.net/files/ANewExerciseInConcurrency.pdf
 *
 * The reindeer are preferred when three elves and nine reindeer are at Santa's door.
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>

#include "random.h"
#include "extensions.h"

/**
 * The number of runs of this scenario.
 * In every run either the reindeer or the elves are handled by Santa.
 */
const int RUNS_NUMBER = 100;
bool santaSleeps = true;
/**
 * Counters for the matches.
 * Every run must match one group.
 * \{
 */
int reindeerMatches = 0;
int elfMatches = 0;
/**
 * \}
 */

const std::size_t REINDEER_NUMBER = 9;
const std::size_t ELF_NUMBER = 10;
/**
 * The number of reindeer which are required to wake up Santa Claus:
 */
const std::size_t REINDEER_MATCH_NUMBER = 9;
/**
 * The number of elves which are required to wake up Santa Claus:
 */
const std::size_t ELF_MATCH_NUMBER = 3;
const int REINDEER_SLEEP_MIN = 1;
const int REINDEER_SLEEP_MAX = 4;
const int ELF_SLEEP_MIN = 3;
const int ELF_SLEEP_MAX = 6;
const std::chrono::milliseconds SANTA_DELAY = std::chrono::milliseconds(50);

enum class Type
{
	Reindeer,
	Elf
};

inline Type reindeer()
{
	return Type::Reindeer;
}

inline Type elf()
{
	return Type::Elf;
}

inline Type teachElves()
{
	++elfMatches;

	std::cout << "R&D" << std::endl;

	return Type::Elf;
}

inline Type deliverToys()
{
	++reindeerMatches;

	std::cout << "Deliver toys" << std::endl;

	return Type::Reindeer;
}

void decide(Type f)
{
	switch (f)
	{
		case Type::Elf:
		{
			teachElves();

			break;
		}

		case Type::Reindeer:
		{
			deliverToys();

			break;
		}
	}
}

void santa_does_work()
{
	santaSleeps = false;
	std::this_thread::sleep_for(SANTA_DELAY);
	santaSleeps = true;
}

#endif