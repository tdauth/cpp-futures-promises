#ifndef ADV_STATE_IMPL_H
#define ADV_STATE_IMPL_H

#include "lock/state.h"

namespace adv
{

template <typename T>
template <typename S>
State<S> *State<T>::create(folly::Executor *executor)
{
	// TODO Support different implementations
	return new adv_lock::State<S>(executor);
}

template <typename T>
template <typename S>
std::shared_ptr<State<S>> State<T>::createShared(folly::Executor *executor)
{
	// TODO Support different implementations
	return std::make_shared<adv_lock::State<S>>(executor);
}

} // namespace adv

#endif