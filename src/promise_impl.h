#ifndef ADV_PROMISE_IMPL_H
#define ADV_PROMISE_IMPL_H

#include "promise.h"

namespace adv
{

template <typename T>
adv::Future<T> Promise<T>::future()
{
	return adv::Future<T>(core);
}

} // namespace adv

#endif