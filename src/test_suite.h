#ifndef ADV_TESTSUITE_H
#define ADV_TESTSUITE_H

#include <boost/test/included/unit_test.hpp>

#include <atomic>
#include <future>
#include <string>

#include <boost/thread/synchronized_value.hpp>

#include "core_impl.h"
#include "future.h"
#include "future_impl.h"
#include "promise.h"
#include "promise_impl.h"

namespace adv
{

template <typename StateType, typename StateTypeString>
class TestSuite
{
	public:
	/*
	 * Do not use multiple executor threads to detect invalid blocking.
	 * The inline executor ensures the immediate execution of callbacks
	 * in the main thread. This simplifies testing.
	 */
	TestSuite() : ex(new folly::InlineExecutor())
	{
	}

	~TestSuite()
	{
		delete ex;
		ex = nullptr;
	}

	void testTryNotInitialized()
	{
		Try<int> t;
		BOOST_REQUIRE(!t.hasValue());
		BOOST_REQUIRE(!t.hasException());
		BOOST_CHECK_THROW(t.get(), UsingUninitializedTry);
	}

	void testTryRuntimeError()
	{
		Try<int> t(std::make_exception_ptr(std::runtime_error("Error")));
		BOOST_REQUIRE(t.hasException());
		BOOST_CHECK_THROW(t.get(), std::runtime_error);
	}

	void testTryValue()
	{
		Try<int> t(10);
		BOOST_REQUIRE(t.hasValue());
		BOOST_CHECK_EQUAL(10, t.get());
	}

	void testOnComplete()
	{
		auto p = createPromiseInt();
		auto f = p.future();
		BOOST_REQUIRE(!f.isReady());
		BOOST_REQUIRE(p.trySuccess(10));
		BOOST_REQUIRE(f.isReady());
		/*
		 * We have to use a synchronized value here since Boost.Thread's
		 * implementation does not execute the callback in the specified executor
		 * immediately.
		 */
		std::promise<int> valuePromise;
		f.onComplete(
		    [&valuePromise](const Try<int> &t) { valuePromise.set_value(t.get()); });

		auto r = valuePromise.get_future().get();
		BOOST_CHECK_EQUAL(10, r);

		std::atomic<int> c(0);
		f.onComplete([&c](const Try<int> &) { ++c; });
		f.onComplete([&c](const Try<int> &) { ++c; });
		f.onComplete([&c](const Try<int> &) { ++c; });
		BOOST_CHECK_EQUAL(3, c);
	}

	// implementations are quite different here.
	void testOnCompleteIsReadyAndGet()
	{
		auto p = createPromiseString();
		auto f = p.future();
		BOOST_REQUIRE(!f.isReady());
		BOOST_REQUIRE(p.trySuccess("10"));
		BOOST_REQUIRE(f.isReady());

		f.onComplete([](const Try<std::string> &) {});

		// allow multiple retrievals
		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(Try<std::string>("10"), f.get());
	}

	void testOnSuccess()
	{
		std::string v;
		auto p = createPromiseString();
		BOOST_REQUIRE(p.trySuccess("10"));
		auto f = p.future();
		f.onSuccess([&v](const std::string &t) { v = t; });

		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL("10", v);
	}

	void testOnFailure()
	{
		std::string v;
		auto p = createPromiseString();
		BOOST_REQUIRE(p.tryFailure(std::runtime_error("Failure!")));
		auto f = p.future();
		f.onFailure([&v](const std::exception_ptr &e) {
			try
			{
				std::rethrow_exception(e);
			}
			catch (std::exception &e)
			{
				v = e.what();
			}
		});

		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL("Failure!", v);
	}

	void testGet()
	{
		auto p = createPromiseInt();
		p.trySuccess(10);
		auto f = p.future();

		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
		BOOST_CHECK(f.isReady());
	}

	void testIsReady()
	{
		auto p = createPromiseInt();
		auto f = p.future();

		BOOST_REQUIRE(!f.isReady());

		p.trySuccess(10);

		BOOST_CHECK(f.isReady());
	}

	void testThen()
	{
		auto p = createPromiseInt();
		p.trySuccess(10);
		auto f0 = p.future();
		BOOST_CHECK(f0.isReady());
		BOOST_CHECK_EQUAL(Try<int>(10), f0.get());
		auto f = f0.then([](const Try<int> &t) {
			if (t.hasValue())
			{
				return std::to_string(t.get());
			}

			return std::string("Failure!");
		});

		BOOST_CHECK_EQUAL(Try<std::string>("10"), f.get());
	}

	void testThenWith()
	{
		auto p0 = createPromiseString();
		p0.trySuccess("11");
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.trySuccess(10);
		auto f1 = p1.future();
		auto f = f1.thenWith([f0](const Try<int> &) { return f0; });

		BOOST_CHECK_EQUAL(Try<std::string>("11"), f.get());
	}

	void testGuard()
	{
		auto p = createPromiseInt();
		p.trySuccess(10);
		auto f = p.future().guard([](const int &v) { return v == 10; });

		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
	}

	void testGuardFails()
	{
		auto p = createPromiseInt();
		p.trySuccess(10);
		auto f = p.future().guard([](const int &v) { return v != 10; });

		BOOST_CHECK_THROW(f.get().get(), PredicateNotFulfilled);
	}

	void testOrElseFirstSuccessful()
	{
		auto p0 = createPromiseInt();
		p0.trySuccess(10);
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.orElse(std::move(f1));

		BOOST_CHECK_EQUAL(Try<int>(10), f2.get());
	}

	void testOrElseSecondSuccessful()
	{
		auto p0 = createPromiseInt();
		p0.tryFailure(std::runtime_error("Failure!"));
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.orElse(std::move(f1));

		BOOST_CHECK_EQUAL(Try<int>(11), f2.get());
	}

	void testOrElseBothFail()
	{
		auto p0 = createPromiseInt();
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.tryFailure(std::runtime_error("Failure 1!"));
		auto f1 = p1.future();
		auto f2 = f0.orElse(std::move(f1));

		BOOST_CHECK_THROW(f2.get(), std::runtime_error);
	}

	void testFirst()
	{
		auto p0 = createPromiseInt();
		p0.trySuccess(10);
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.first(f1);
		auto r = f2.get();

		BOOST_CHECK_EQUAL(Try<int>(10), r);
	}

	void testFirstWithException()
	{
		auto p0 = createPromiseInt();
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.tryFailure(std::runtime_error("Failure 1!"));
		auto f1 = p1.future();
		auto f2 = f0.first(f1);

		try
		{
			f2.get();
			BOOST_FAIL("Expecting exception.");
		}
		catch (const std::runtime_error &e)
		{
			BOOST_CHECK_EQUAL("Failure 0!", e.what());
		}
	}

	void testFirstSucc()
	{
		auto p0 = createPromiseInt();
		p0.trySuccess(10);
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.firstSucc(f1);
		auto r = f2.get();

		BOOST_CHECK_EQUAL(Try<int>(10), r);
	}

	void testFirstSuccWithException()
	{
		auto p0 = createPromiseInt();
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.firstSucc(f1);
		auto r = f2.get();

		BOOST_CHECK_EQUAL(Try<int>(11), r);
	}

	void testFirstSuccBothFail()
	{
		auto p0 = createPromiseInt();
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = createPromiseInt();
		p1.tryFailure(std::runtime_error("Failure 1!"));
		auto f1 = p1.future();
		auto f2 = f0.firstSucc(f1);

		try
		{
			f2.get();
			BOOST_FAIL("Expecting exception.");
		}
		catch (const std::exception &e)
		{
			BOOST_CHECK_EQUAL("Failure 1!", e.what());
		}
	}

	void testAsync()
	{
		auto f = async<>(ex, []() { return 10; });
		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
	}

	void testBrokenPromise()
	{
		Promise<int> *p = new Promise<int>(createPromiseInt());
		Future<int> f = p->future();
		delete p;
		p = nullptr;

		BOOST_CHECK_THROW(f.get(), BrokenPromise);
	}

	void testTryComplete()
	{
		auto p = createPromiseInt();
		auto f = p.future();

		BOOST_REQUIRE(!f.isReady());

		bool result = p.tryComplete(Try<int>(10));

		BOOST_REQUIRE(result);
		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
	}

	void testTrySuccess()
	{
		auto p = createPromiseInt();
		auto f = p.future();

		BOOST_REQUIRE(!f.isReady());

		bool result = p.trySuccess(10);

		BOOST_REQUIRE(result);
		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
	}

	void testTryFailure()
	{
		auto p = createPromiseInt();
		auto f = p.future();

		BOOST_REQUIRE(!f.isReady());

		bool result = p.tryFailure(std::runtime_error("Failure!"));

		BOOST_CHECK(result);
		BOOST_REQUIRE(f.isReady());

		try
		{
			f.get();
			BOOST_FAIL("Expected exception");
		}
		catch (const std::exception &e)
		{
			BOOST_CHECK_EQUAL("Failure!", e.what());
		}
	}

	void testTryCompleteWith()
	{
		auto p = createPromiseInt();
		auto f = p.future();
		auto completingPromise = createPromiseInt();
		completingPromise.trySuccess(10);
		auto completingFuture = completingPromise.future();
		std::move(p).tryCompleteWith(completingFuture);

		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
	}

	void testTryCompleteWithFailure()
	{
		auto p = createPromiseInt();
		auto f = p.future();
		auto completingPromise = createPromiseInt();
		completingPromise.tryFailure(std::runtime_error("Failure!"));
		auto completingFuture = completingPromise.future();

		std::move(p).tryCompleteWith(completingFuture);

		BOOST_CHECK_THROW(f.get(), std::runtime_error);
	}

	void testTrySuccessWith()
	{
		auto p = createPromiseInt();
		auto f = p.future();
		auto completingPromise = createPromiseInt();
		completingPromise.trySuccess(10);
		auto completingFuture = completingPromise.future();

		std::move(p).trySuccessWith(completingFuture);

		BOOST_CHECK_EQUAL(Try<int>(10), f.get());
	}

	void testTryFailureWith()
	{
		auto p = createPromiseInt();
		auto f = p.future();
		auto completingPromise = createPromiseInt();
		completingPromise.tryFailure(std::runtime_error("Failure!"));
		auto completingFuture = completingPromise.future();

		std::move(p).tryFailureWith(completingFuture);

		try
		{
			f.get();
			BOOST_FAIL("Expected exception");
		}
		catch (const std::exception &e)
		{
			BOOST_CHECK_EQUAL("Failure!", e.what());
		}
	}

	void testFirstN()
	{
		std::vector<Future<int>> futures;
		futures.push_back(successful(ex, 10));
		futures.push_back(failed(ex, std::runtime_error("Failure!")));
		futures.push_back(successful(ex, 12));
		futures.push_back(successful(ex, 13));

		auto f = firstN(ex, std::move(futures), 3);
		auto v = f.get().get();

		BOOST_CHECK_EQUAL(3u, v.size());
		auto v0 = std::move(v[0]);
		BOOST_CHECK_EQUAL(0u, v0.first);
		BOOST_CHECK_EQUAL(10, v0.second.get());
		auto v1 = std::move(v[1]);
		BOOST_CHECK_EQUAL(1u, v1.first);
		BOOST_CHECK_THROW(v1.second.get(), std::runtime_error);
		auto v2 = std::move(v[2]);
		BOOST_CHECK_EQUAL(2u, v2.first);
		BOOST_CHECK_EQUAL(12, v2.second.get());
	}

	void testFirstNSucc()
	{
		std::vector<Future<int>> futures;
		futures.push_back(successful(ex, 10));
		futures.push_back(failed(ex, std::runtime_error("Failure!")));
		futures.push_back(successful(ex, 12));
		futures.push_back(successful(ex, 13));

		auto f = firstNSucc(ex, std::move(futures), 3);
		auto v = f.get().get();

		BOOST_CHECK_EQUAL(3u, v.size());
		auto v0 = std::move(v[0]);
		BOOST_CHECK_EQUAL(0u, v0.first);
		BOOST_CHECK_EQUAL(10, v0.second);
		auto v1 = std::move(v[1]);
		BOOST_CHECK_EQUAL(2u, v1.first);
		BOOST_CHECK_EQUAL(12, v1.second);
		auto v2 = std::move(v[2]);
		BOOST_CHECK_EQUAL(3u, v2.first);
		BOOST_CHECK_EQUAL(13, v2.second);
	}

	void testAll()
	{
		testTryNotInitialized();
		testTryRuntimeError();
		testTryValue();
		testOnComplete();
		testOnCompleteIsReadyAndGet();
		testOnSuccess();
		testOnFailure();
		testGet();
		testIsReady();
		testThen();
		testThenWith();
		testGuard();
		testGuardFails();
		testOrElseFirstSuccessful();
		testOrElseSecondSuccessful();
		testOrElseBothFail();
		testFirst();
		testFirstWithException();
		testFirstSucc();
		testFirstSuccWithException();
		testFirstSuccBothFail();
		testAsync();
		testBrokenPromise();
		testTryComplete();
		testTrySuccess();
		testTryFailure();
		testTryCompleteWith();
		testTryCompleteWithFailure();
		testTrySuccessWith();
		testTryFailureWith();
		testFirstN();
		testFirstNSucc();
	}

	private:
	folly::InlineExecutor *ex;

	Promise<int> createPromiseInt()
	{
		return Promise<int>(ex);
	}

	Promise<std::string> createPromiseString()
	{
		return Promise<std::string>(ex);
	}

	Future<int> successful(folly::Executor *ex, int &&v)
	{
		auto p = createPromiseInt();
		p.trySuccess(std::move(v));
		return p.future();
	}

	Future<int> failed(folly::Executor *ex, std::exception &&e)
	{
		auto p = createPromiseInt();
		p.tryFailure(std::move(e));
		return p.future();
	}
};
} // namespace adv

#endif