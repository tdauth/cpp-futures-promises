#ifndef FUTURE_H
#define FUTURE_H

#include <vector>
#include <utility>

namespace xtn
{

template<typename T>
class Try
{
	public:
		Try();
		Try(T &&v);
		Try(std::exception_ptr &&e);

		T get();
		bool hasValue();
		bool hasException();
};

class Executor
{
	public:
		template<typename Func>
		void submit(Func &&f);
};

template<typename T>
class Future
{
	public:
		T get();

		template<typename Func>
		void onComplete(Func &&f);

		bool isReady();

		template<typename Func>
		Future<T> guard(Func &&f);

		template<typename Func, typename S>
		Future<S> then(Func &&f);

		Future<T> orElse(Future<T> &&other);

		Future<T> first(Future<T> &&other);

		Future<T> firstSucc(Future<T> &&other);

		Future();
		Future(Future<T> &&other);
		Future(const Future<T> &other) = delete;
		Future<T>& operator=(const Future<T> &other) = delete;
};

template<typename Func, typename T>
Future<T> async(Executor *ex, Func &&f);

template<typename T>
Future<std::vector<std::pair<std::size_t, Future<T>>>> firstN(std::vector<Future<T>> &&c, std::size_t n);

template<typename T>
Future<std::vector<std::pair<std::size_t, T>>> firstNSucc(std::vector<Future<T>> &&c, std::size_t n);

template<typename T>
class SharedFuture
{
};

}

#endif