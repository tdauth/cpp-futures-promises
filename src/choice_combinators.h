#ifndef CHOICECOMBINATORS_H
#define CHOICECOMBINATORS_H

#include <mutex>

#include <folly/futures/Future.h>

#include "random.h"

enum class FirstSuccessState
{
	Start,
	FirstSucceeded,
	FirstFailed
};

template<typename T>
struct FirstSuccessContext
{
	std::mutex m;
	folly::Promise<T> p;
	folly::Future<folly::Unit> f0;
	folly::Future<folly::Unit> f1;
	FirstSuccessState state = FirstSuccessState::Start;
};

template<typename T>
void completeFirstSuccessContext(folly::Try<T> &&t, std::shared_ptr<FirstSuccessContext<T>> ctx)
{
	std::unique_lock<std::mutex> l(ctx->m);

	switch (ctx->state)
	{
		case  FirstSuccessState::Start:
		{
			if (t.hasValue())
			{
				ctx->p.setValue(std::move(t.value()));
				ctx->state = FirstSuccessState::FirstSucceeded;
			}
			else
			{
				ctx->state = FirstSuccessState::FirstFailed;
			}

			break;
		}

		case FirstSuccessState::FirstSucceeded:
		{
			break;
		}

		case FirstSuccessState::FirstFailed:
		{
			if (t.hasValue())
			{
				ctx->p.setValue(std::move(t.value()));
			}
			else
			{
				ctx->p.setException(t.exception());
			}

			break;
		}
	}
}

template<typename T>
folly::Future<T> firstSuccess(folly::Future<T> &&f0, folly::Future<T> &&f1)
{
	std::shared_ptr<FirstSuccessContext<T>> ctx = std::make_shared<FirstSuccessContext<T>>();
	folly::Future<T> r = ctx->p.getFuture();

	ctx->f0 = f0.then(std::bind(completeFirstSuccessContext<T>, std::placeholders::_1, ctx));
	ctx->f1 = f1.then(std::bind(completeFirstSuccessContext<T>, std::placeholders::_1, ctx));

	return r;
}

template<typename T>
folly::Future<T> firstSuccessRandom(folly::Future<T> &&f0, folly::Future<T> &&f1)
{
	std::shared_ptr<FirstSuccessContext<T>> ctx = std::make_shared<FirstSuccessContext<T>>();
	folly::Future<T> r = ctx->p.getFuture();

	// Make a randomized choice between the two futures:
	folly::Future<T> futures[] = {
		std::move(f0),
		std::move(f1)
	};

	const int selection = select();

	ctx->f0 = futures[selection].then(std::bind(completeFirstSuccessContext<T>, std::placeholders::_1, ctx));
	ctx->f1 = futures[1 - selection].then(std::bind(completeFirstSuccessContext<T>, std::placeholders::_1, ctx));

	return r;
}

enum class FirstFailureState
{
	Start,
	FirstSucceeded,
	FirstFailed
};

template<typename T>
struct FirstFailureContext
{
	std::mutex m;
	folly::Promise<T> p;
	folly::Future<folly::Unit> f0;
	folly::Future<folly::Unit> f1;
	FirstFailureState state = FirstFailureState::Start;
};

template<typename T>
void completeFirstFailureContext(folly::Try<T> &&t, std::shared_ptr<FirstFailureContext<T>> ctx)
{
	std::unique_lock<std::mutex> l(ctx->m);

	switch (ctx->state)
	{
		case FirstFailureState::Start:
		{
			if (t.hasValue())
			{
				ctx->state = FirstFailureState::FirstSucceeded;
			}
			else
			{
				ctx->p.setException(t.exception());
				ctx->state = FirstFailureState::FirstFailed;
			}

			break;
		}

		case FirstFailureState::FirstSucceeded:
		{
			if (t.hasException())
			{
				ctx->p.setException(t.exception());
			}

			break;
		}

		case FirstFailureState::FirstFailed:
		{
			break;
		}
	}
}

template<typename T>
folly::Future<T> firstFailure(folly::Future<T> &&f0, folly::Future<T> &&f1)
{
	std::shared_ptr<FirstFailureContext<T>> ctx = std::make_shared<FirstFailureContext<T>>();
	folly::Future<T> r = ctx->p.getFuture();

	ctx->f0 = f0.then(std::bind(completeFirstFailureContext<T>, std::placeholders::_1, ctx));
	ctx->f1 = f1.then(std::bind(completeFirstFailureContext<T>, std::placeholders::_1, ctx));

	return r;
}

template<typename T>
folly::Future<T> firstFailureRandom(folly::Future<T> &&f0, folly::Future<T> &&f1)
{
	std::shared_ptr<FirstFailureContext<T>> ctx = std::make_shared<FirstFailureContext<T>>();
	folly::Future<T> r = ctx->p.getFuture();

	// Make a randomized choice between the two futures:
	folly::Future<T> futures[] = {
		std::move(f0),
		std::move(f1)
	};

	const int selection = select();

	ctx->f0 = futures[selection].then(std::bind(completeFirstFailureContext<T>, std::placeholders::_1, ctx));
	ctx->f1 = futures[1- selection].then(std::bind(completeFirstFailureContext<T>, std::placeholders::_1, ctx));

	return r;
}

template<typename T>
folly::Future<T> firstCompletion(folly::Future<T> &&f0, folly::Future<T> &&f1);

template<typename T>
folly::Future<T> firstCompletiondRandom(folly::Future<T> &&f0, folly::Future<T> &&f1);

#endif