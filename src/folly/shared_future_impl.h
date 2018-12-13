#ifndef ADV_FOLLY_SHAREDFUTUREIMPL_H
#define ADV_FOLLY_SHAREDFUTUREIMPL_H

#include "promise.h"
#include "shared_future.h"

namespace adv_folly
{

template <typename T>
template <typename S>
Promise<S> SharedFuture<T>::createPromise(folly::Executor *ex)
{
	return Promise<S>(ex);
}

template <typename T>
typename SharedFuture<T>::Self SharedFuture<T>::successful(folly::Executor *ex,
                                                           T &&v)
{
	Promise<T> p(ex);
	p.trySuccess(std::move(v));
	return p.future();
}

template <typename T>
template <typename E>
typename SharedFuture<T>::Self SharedFuture<T>::failed(folly::Executor *ex,
                                                       E &&e)
{
	Promise<T> p(ex);
	p.tryFailure(std::move(e));
	return p.future();
}
}

#endif