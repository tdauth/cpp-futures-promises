#ifndef CHOICECOMBINATORS_H
#define CHOICECOMBINATORS_H

#include <random>
#include <mutex>

#include <folly/futures/Future.h>

enum class FirstSuccessState
{
	Start,
	FirstSucceeded,
	FirstFailed
};

template<typename T>
struct FirstSuccessSharedState
{
	std::mutex m;
	folly::Promise<T> p;
	folly::Future<folly::Unit> f0;
	folly::Future<folly::Unit> f1;
	FirstSuccessState state = FirstSuccessState::Start;
};

template<typename T>
void completeFirstSuccessSharedState(folly::Try<T> &&t, FirstSuccessSharedState<T> *st)
{
	std::unique_lock<std::mutex> l(st->m);

	if (t.hasValue())
	{
		if (st->state == FirstSuccessState::Start)
		{
			st->p.setValue(std::move(t.value()));
			st->state = FirstSuccessState::FirstSucceeded;
		}
		else if (st->state == FirstSuccessState::FirstFailed)
		{
			st->p.setValue(std::move(t.value()));

			delete st;
		}
	}
	else
	{
		if (st->state == FirstSuccessState::FirstFailed)
		{
			st->p.setException(t.exception());

			delete st;
		}
		else
		{
			st->state = FirstSuccessState::FirstFailed;
		}
	}
}

template<typename T>
folly::Future<T> firstSuccess(folly::Future<T> &&f0, folly::Future<T> &&f1)
{
	FirstSuccessSharedState<T> *st = new FirstSuccessSharedState<T>();
	folly::Future<T> r = st->p.getFuture();

	st->f0 = f0.then(std::bind(completeFirstSuccessSharedState<T>, std::placeholders::_1, st));
	st->f1 = f1.then(std::bind(completeFirstSuccessSharedState<T>, std::placeholders::_1, st));

	return r;
}

enum class FirstSuccessRandomState
{
	Start,
	FirstSucceeded,
	FirstFailed
};

template<typename T>
struct FirstSuccessRandomSharedState
{
	std::mutex m;
	folly::Promise<T> p;
	folly::Future<folly::Unit> f0;
	folly::Future<folly::Unit> f1;
	FirstSuccessState state = FirstSuccessState::Start;
};

template<typename T>
void completeFirstSuccessRandomSharedState(folly::Try<T> &&t, FirstSuccessRandomSharedState<T> *st)
{
	std::unique_lock<std::mutex> l(st->m);

	if (t.hasValue())
	{
		if (st->state == FirstSuccessState::Start)
		{
			st->p.setValue(std::move(t.value()));
			st->state = FirstSuccessState::FirstSucceeded;
		}
		else if (st->state == FirstSuccessState::FirstFailed)
		{
			st->p.setValue(std::move(t.value()));

			delete st;
		}
	}
	else
	{
		if (st->state == FirstSuccessState::FirstFailed)
		{
			st->p.setException(t.exception());

			delete st;
		}
		else
		{
			st->state = FirstSuccessState::FirstFailed;
		}
	}
}

template<typename T>
folly::Future<T> firstSuccessRandom(folly::Future<T> &&f0, folly::Future<T> &&f1)
{
	FirstSuccessRandomSharedState<T> *st = new FirstSuccessRandomSharedState<T>();
	folly::Future<T> r = st->p.getFuture();

	// Make a randomized choice between the two futures:
	folly::Future<T> futures[] = {
		std::move(f0),
		std::move(f1)
	};

	static std::random_device rand;
	// Choose a random mean between 1 and 6
	static std::default_random_engine e1(rand());
	std::uniform_int_distribution<int> uniform_dist(0, 1);
	const int selection = uniform_dist(e1);

	st->f0 = futures[selection].then(std::bind(completeFirstSuccessRandomSharedState<T>, std::placeholders::_1, st));
	st->f1 = futures[-(selection - 1)].then(std::bind(completeFirstSuccessRandomSharedState<T>, std::placeholders::_1, st));

	return r;
}

template<typename T>
folly::Future<T> firstSuccessOnly(folly::Future<T> &&f0, folly::Future<T> &&f1);

template<typename T>
folly::Future<T> firstSuccessOnlyRandom(folly::Future<T> &&f0, folly::Future<T> &&f1);

#endif