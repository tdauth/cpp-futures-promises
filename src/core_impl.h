#ifndef ADV_CORE_IMPL_H
#define ADV_CORE_IMPL_H

#include "lock/core.h"

namespace adv
{

template <typename T>
template <typename S>
Core<S> *Core<T>::create(folly::Executor *executor)
{
	// TODO Support different implementations
	return new adv_lock::Core<S>(executor);
}

template <typename T>
template <typename S>
std::shared_ptr<Core<S>> Core<T>::createShared(folly::Executor *executor)
{
	// TODO Support different implementations
	return std::make_shared<adv_lock::Core<S>>(executor);
}

} // namespace adv

#endif