#ifndef ADV_TESTSUITE_H
#define ADV_TESTSUITE_H

#include <boost/test/unit_test.hpp>

#include <atomic>
#include <string>

#include "future.h"
#include "future_impl.h"
#include "promise.h"

namespace adv
{

template <typename FutureType>
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
		adv::Try<int> t;
		BOOST_REQUIRE(!t.hasValue());
		BOOST_REQUIRE(!t.hasException());
		BOOST_CHECK_THROW(t.get(), adv::UsingUninitializedTry);
	}

	void testTryRuntimeError()
	{
		adv::Try<int> t(std::make_exception_ptr(std::runtime_error("Error")));
		BOOST_REQUIRE(t.hasException());
		BOOST_CHECK_THROW(t.get(), std::runtime_error);
	}

	void testTryValue()
	{
		adv::Try<int> t(10);
		BOOST_REQUIRE(t.hasValue());
		BOOST_CHECK_EQUAL(10, t.get());
	}

	void testOnComplete()
	{
		int v = 5;
		auto p = FutureType::template createPromise<int>(ex);
		auto f = p.future();
		BOOST_REQUIRE(!f.isReady());
		BOOST_REQUIRE(p.trySuccess(10));
		BOOST_REQUIRE(f.isReady());

		f.onComplete([&v](adv::Try<int> &&t) { v = t.get(); });

		BOOST_CHECK_EQUAL(10, v);

		if (!FutureType::IsShared)
		{
			// multiple callbacks are not allowed
			BOOST_CHECK_THROW(f.onComplete([](adv::Try<int> &&) {}),
			                  adv::OnlyOneCallbackPerFuture);
		}
		else
		{
			f.onComplete([&v](adv::Try<int> &&) { v = v + 1; });
			f.onComplete([&v](adv::Try<int> &&) { v = v + 1; });
			f.onComplete([&v](adv::Try<int> &&) { v = v + 1; });
			BOOST_CHECK_EQUAL(11, v);
		}
	}

	// TODO Specify the semantics for this behaviour. Boost.Thread and Folly
	// implementations are quite different here.
	void testOnCompleteIsReadyAndGet()
	{
		auto p = FutureType::template createPromise<std::string>(ex);
		auto f = p.future();
		BOOST_REQUIRE(!f.isReady());
		BOOST_REQUIRE(p.trySuccess("10"));
		BOOST_REQUIRE(f.isReady());

		f.onComplete([](adv::Try<std::string> &&) {});

		BOOST_REQUIRE(f.isReady());

		if (!FutureType::IsShared)
		{
			// after registering the one callback, it became invalid
			BOOST_CHECK_THROW(f.get(), adv::FutureIsInvalid);
		}
		else
		{
			BOOST_CHECK_EQUAL("10", f.get());
		}
	}

	void testOnSuccess()
	{
		std::string v = "5";
		auto p = FutureType::template createPromise<std::string>(ex);
		BOOST_REQUIRE(p.trySuccess("10"));
		auto f = p.future();
		f.onSuccess([&v](std::string &&t) { v = std::move(t); });

		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL("10", v);
	}

	void testOnFailure()
	{
		std::optional<adv::Try<std::string>> t;
		auto p = FutureType::template createPromise<std::string>(ex);
		BOOST_REQUIRE(p.tryFailure(std::runtime_error("Failure!")));
		auto f = p.future();
		f.onFailure([&t](std::exception_ptr &&e) {
			t.emplace(adv::Try<std::string>(std::move(e)));
		});

		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK(t.has_value());
		auto r = std::move(t.value());
		BOOST_CHECK(r.hasException());

		try
		{
			r.get();
			BOOST_FAIL("Expected exception.");
		}
		catch (std::runtime_error &e)
		{
			BOOST_CHECK_EQUAL("Failure!", e.what());
		}
	}

	void testGet()
	{
		auto p = FutureType::template createPromise<int>(ex);
		p.trySuccess(10);
		auto f = p.future();

		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(10, f.get());

		if (!FutureType::IsShared)
		{
			// Multiple gets are not allowed:
			BOOST_CHECK_THROW(f.get(), adv::FutureIsInvalid);
			BOOST_CHECK_THROW(f.isReady(), adv::FutureIsInvalid);
		}
		else
		{
			BOOST_CHECK_EQUAL(10, f.get());
			BOOST_CHECK(f.isReady());
		}
	}

	void testIsReady()
	{
		auto p = FutureType::template createPromise<int>(ex);
		auto f = p.future();

		BOOST_REQUIRE(!f.isReady());

		p.trySuccess(10);

		BOOST_CHECK(f.isReady());
	}

	void testInvalidFuture()
	{
		auto p = FutureType::template createPromise<int>(ex);
		p.trySuccess(10);
		auto f = p.future();
		// makes the future invalid
		f.get();

		// Multiple gets are not allowed:
		BOOST_CHECK_THROW(f.get(), adv::FutureIsInvalid);
		BOOST_CHECK_THROW(f.isReady(), adv::FutureIsInvalid);
		BOOST_CHECK_THROW(f.onComplete([](adv::Try<int> &&) {}),
		                  adv::FutureIsInvalid);
	}

	void testThen()
	{
		auto p = FutureType::template createPromise<int>(ex);
		p.trySuccess(10);
		auto f = p.future().then([](adv::Try<int> &&t) {
			if (t.hasValue())
			{
				return std::to_string(t.get());
			}

			return std::string("Failure!");
		});

		BOOST_CHECK_EQUAL("10", f.get());
	}

	void testThenWith()
	{
		auto p0 = FutureType::template createPromise<std::string>(ex);
		p0.trySuccess("11");
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.trySuccess(10);
		auto f1 = p1.future();
		auto f = f1.thenWith([f0 = std::move(f0)](adv::Try<int> &&) mutable {
			return std::move(f0);
		});

		BOOST_CHECK_EQUAL("11", f.get());
	}

	void testGuard()
	{
		auto p = FutureType::template createPromise<int>(ex);
		p.trySuccess(10);
		auto f = p.future().guard([](const int &v) { return v == 10; });

		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testGuardFails()
	{
		auto p = FutureType::template createPromise<int>(ex);
		p.trySuccess(10);
		auto f = p.future().guard([](const int &v) { return v != 10; });

		BOOST_CHECK_THROW(f.get(), adv::PredicateNotFulfilled);
	}

	void testOrElseFirstSuccessful()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.trySuccess(10);
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.orElse(std::move(f1));

		BOOST_CHECK_EQUAL(10, f2.get());
	}

	void testOrElseSecondSuccessful()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.tryFailure(std::runtime_error("Failure!"));
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.orElse(std::move(f1));

		BOOST_CHECK_EQUAL(11, f2.get());
	}

	void testOrElseBothFail()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.tryFailure(std::runtime_error("Failure 1!"));
		auto f1 = p1.future();
		auto f2 = f0.orElse(std::move(f1));

		BOOST_CHECK_THROW(f2.get(), std::runtime_error);
	}

	void testFirst()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.trySuccess(10);
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.first(f1);
		auto r = f2.get();

		BOOST_CHECK_EQUAL(10, r);
	}

	void testFirstWithException()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
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
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.trySuccess(10);
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.firstSucc(f1);
		auto r = f2.get();

		BOOST_CHECK_EQUAL(10, r);
	}

	void testFirstSuccWithException()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.trySuccess(11);
		auto f1 = p1.future();
		auto f2 = f0.firstSucc(f1);
		auto r = f2.get();

		BOOST_CHECK_EQUAL(11, r);
	}

	void testFirstSuccBothFail()
	{
		auto p0 = FutureType::template createPromise<int>(ex);
		p0.tryFailure(std::runtime_error("Failure 0!"));
		auto f0 = p0.future();
		auto p1 = FutureType::template createPromise<int>(ex);
		p1.tryFailure(std::runtime_error("Failure 1!"));
		auto f1 = p1.future();
		auto f2 = f0.firstSucc(f1);

		try
		{
			f2.get();
			BOOST_FAIL("Expecting exception.");
		}
		catch (const adv::BrokenPromise &e)
		{
		}
	}

	void testSuccessful()
	{
		auto f = FutureType::successful(ex, 10);
		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testFailed()
	{
		auto f = FutureType::failed(ex, std::runtime_error("Failure!"));
		BOOST_CHECK_THROW(f.get(), std::runtime_error);
	}

	void testAsync()
	{
		auto f = adv::async<typename FutureType::template PromiseType<int>>(
		    ex, []() { return 10; });
		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testBrokenPromise()
	{
		using P = typename FutureType::template PromiseType<int>;
		P *p = new P(ex);
		FutureType f = p->future();
		delete p;
		p = nullptr;

		BOOST_CHECK_THROW(f.get(), adv::BrokenPromise);
	}

	void testTryComplete()
	{
		auto p = FutureType::template createPromise<int>(ex);
		FutureType f = p.future();

		BOOST_REQUIRE(!f.isReady());

		bool result = p.tryComplete(adv::Try<int>(10));

		BOOST_REQUIRE(result);
		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testTrySuccess()
	{
		auto p = FutureType::template createPromise<int>(ex);
		FutureType f = p.future();

		BOOST_REQUIRE(!f.isReady());

		bool result = p.trySuccess(10);

		BOOST_REQUIRE(result);
		BOOST_REQUIRE(f.isReady());
		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testTryFailure()
	{
		auto p = FutureType::template createPromise<int>(ex);
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
		auto p = FutureType::template createPromise<int>(ex);
		auto f = p.future();
		auto completingPromise = FutureType::template createPromise<int>(ex);
		completingPromise.trySuccess(10);
		auto completingFuture = completingPromise.future();
		std::move(p).tryCompleteWith(completingFuture);

		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testTryCompleteWithFailure()
	{
		auto p = FutureType::template createPromise<int>(ex);
		auto f = p.future();
		auto completingPromise = FutureType::template createPromise<int>(ex);
		completingPromise.tryFailure(std::runtime_error("Failure!"));
		auto completingFuture = completingPromise.future();

		std::move(p).tryCompleteWith(completingFuture);

		BOOST_CHECK_THROW(f.get(), std::runtime_error);
	}

	void testTrySuccessWith()
	{
		auto p = FutureType::template createPromise<int>(ex);
		auto f = p.future();
		auto completingPromise = FutureType::template createPromise<int>(ex);
		completingPromise.trySuccess(10);
		auto completingFuture = completingPromise.future();

		std::move(p).trySuccessWith(completingFuture);

		BOOST_CHECK_EQUAL(10, f.get());
	}

	void testTryFailureWith()
	{
		auto p = FutureType::template createPromise<int>(ex);
		auto f = p.future();
		auto completingPromise = FutureType::template createPromise<int>(ex);
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
		testInvalidFuture();
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
		testSuccessful();
		testFailed();
		testAsync();
		testBrokenPromise();
		testTryComplete();
		testTrySuccess();
		testTryFailure();
		testTryCompleteWith();
		testTryCompleteWithFailure();
		testTrySuccessWith();
		testTryFailureWith();
	}

	private:
	folly::InlineExecutor *ex;
};
}

#endif