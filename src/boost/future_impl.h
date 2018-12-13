#ifndef ADV_BOOST_FUTUREIMPL_H
#define ADV_BOOST_FUTUREIMPL_H

#include "future.h"
#include "promise.h"

namespace adv_boost
{

template <typename T>
template <typename S>
Promise<S> Core<T>::createPromise(folly::Executor *executor)
{
	return Promise<S>(executor);
}
}

#endif