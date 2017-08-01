#ifndef PROMISE_H
#define PROMISE_H

namespace wish
{

template<typename T>
class Future;

template<typename T>
class Try;

template<typename T>
class Promise
{
	public:
		Future<T> future();

		bool tryComplete(Try<T> &&v);
		bool trySuccess(T &&v);
		template<typename Exception>
		bool tryFailure(Exception &&e);

		void tryCompleteWith(Future<T> &&f);
		void trySuccessWith(Future<T> &&f);
		void tryFailureWith(Future<T> &&f);

		Promise();
		Promise(Promise<T> &&other);
		Promise(const Promise<T> &other) = delete;
		Promise<T>& operator=(const Promise<T> &other) = delete;
};

}

#endif