#ifndef TREE_H
#define TREE_H

/**
 * \brief Simple calculation of the power at compile time.
 *
 * Source: https://stackoverflow.com/a/27270738
 */
template <int A, int B>
struct get_power
{
	static const int value = A * get_power<A, B - 1>::value;
};
template <int A>
struct get_power<A, 0>
{
	static const int value = 1;
};

using TREE_TYPE = int;
constexpr int TREE_DEPTH = 12;
constexpr int TREE_CHILDS = 2;
static_assert(TREE_CHILDS == 2, "The custom combinators only support passing two futures.");
/**
 * The number of future nodes in the tree started at this level.
 * Note that the root node is included.
 * TODO the formula is wrong for only 1 node per level?
 */
constexpr int NODES = get_power<TREE_CHILDS, TREE_DEPTH + 1>::value - 1;

#endif