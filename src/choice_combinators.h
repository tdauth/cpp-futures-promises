#ifndef CHOICECOMBINATORS_H
#define CHOICECOMBINATORS_H

#include <folly/futures/Future.h>

template<typename T>
folly::Future<T> firstSuccess(folly::Future<T> &&f0, folly::Future<T> &&f1);

template<typename T>
folly::Future<T> firstSuccessRandom(folly::Future<T> &&f0, folly::Future<T> &&f1);

template<typename T>
folly::Future<T> firstSuccessOnly(folly::Future<T> &&f0, folly::Future<T> &&f1);

#endif