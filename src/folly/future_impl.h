#ifndef ADV_FOLLY_FUTUREIMPL_H
#define ADV_FOLLY_FUTUREIMPL_H

#include "future.h"
#include "promise.h"

namespace adv_folly
{

template <typename T>
template <typename S>
Promise<S> Core<T>::createPromise()
{
	return Promise<S>();
}
}

#endif