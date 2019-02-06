#ifndef ADV_CORE_IMPL_H
#define ADV_CORE_IMPL_H

#include "core.h"
#include "lock/core.h"

namespace adv
{

template <typename T>
template <typename S>
typename Core<S>::SharedPtr Core<T>::createShared(folly::Executor *executor)
{
	// TODO Support different implementations
	return typename Core<S>::SharedPtr(new adv_lock::Core<S>(executor));
}

} // namespace adv

#endif