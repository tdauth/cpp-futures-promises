#ifndef ADV_SHAREDFUTURE_H
#define ADV_SHAREDFUTURE_H

namespace adv
{

/**
 * \brief Provides a shared future which can be copied around and has multiple read semantics.
 * Provides the same functions and lifts them to unique futures if possible to avoid redundant code.
 */
template<typename T>
class SharedFuture
{
	public:
		SharedFuture();
		SharedFuture(SharedFuture<T> &&other);
		SharedFuture(const SharedFuture<T> &other) = delete;
		SharedFuture<T>& operator=(const SharedFuture<T> &other) = delete;

		T get();

		bool isReady();

		template<typename Func, typename S>
		SharedFuture<S> then(Func &&f);

		template<typename Func>
		void onComplete(Func &&f); // (D)

		template<typename Func>
		SharedFuture<T> guard(Func &&f); // (D)

		SharedFuture<T> orElse(SharedFuture<T> &&other); // (D)

		SharedFuture<T> first(SharedFuture<T> &&other); // (D)

		SharedFuture<T> firstSucc(SharedFuture<T> &&other); // (D)
};

}

#endif
