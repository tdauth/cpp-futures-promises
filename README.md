# Advanced Futures and Promises with C++
This project provides an advanced futures and promises library based on our papers [Advanced Futures and Promises in C++](http://www.home.hs-karlsruhe.de/~suma0002/publications/advanced-futures-promises-cpp.pdf) and [Futures and Promises in Haskell and Scala](https://www.researchgate.net/publication/330066278_Futures_and_promises_in_Haskell_and_Scala).
The library offers functions which are missing from other C++ futures and promises libraries.
It is implemented with the help of only a few core operations which allows a much easier adaption to different implementations.
Advanced futures and promises are shared by default.
Currently, the library has one reference implementation which uses a lock-based approach.

The derived features were inspired by [Scala library for futures and promises](http://docs.scala-lang.org/overviews/core/futures.html) and Folly.

## Automatic Build with TravisCI
[![Build Status](https://travis-ci.org/tdauth/cpp-futures-promises.svg?branch=single-read-and-bugfixes)](https://travis-ci.org/tdauth/cpp-futures-promises)
[![Code Coverage](https://img.shields.io/codecov/c/github/tdauth/cpp-futures-promises/master.svg)](https://codecov.io/github/tdauth/cpp-futures-promises?branch=single-read-and-bugfixes)

## Manual Build
To compile the project run one of the the following Bash scripts on Linux:
* [build.sh](./build.sh)
* [buildcoverage.sh](./buildcoverage.sh)
* [buildrelease.sh](./buildrelease.sh)

They will compile the project.
The first two will run all unit tests.
The third will create an RPM package.
Note that all targets are added as CTest unit tests which simplifies their execution.
The dependencies will be downloaded and compiled automatically.
Therefore, you need Internet access when building for the first time.

## Dependencies
The project requires the GCC with C++17 support and CMake to be built.

The project requires the following libraries:
* [Boost](http://www.boost.org/)
* [Folly](https://github.com/facebook/folly)

It will download and compile these libraries automatically when being compiled.
The versions of the libraries are specified in the [CMakeLists.txt](./CMakeLists.txt) file in the top level directory of the project.

Furthermore, the project requires the following packages on Fedora 29:
* cmake
* gcc-c++
* libatomic
* bash
* which
* rpm-build
* valgrind
* lcov
* cppcheck
* cppcheck-htmlreport

Folly dependencies on Fedora 29:
* glog-devel
* gflags-devel
* autoconf
* automake
* autoconf
* automake
* libtool
* zlib-devel
* lzma-devel
* snappy-devel
* double-conversion-devel
* openssl-devel
* libevent-devel

These dependencies can be installed with the script [install_fedora_dependencies.sh](./install_fedora_dependencies.sh).

## Core of the Art
There is several C++ libraries for futures and promises:
* C++17 thread support library (standard library)
* Boost.Thread
* Folly

Qt provides futures support as well but no promises.
All of these libraries are missing non-blocking combinators, especially compared to Scala's futures and promises library.
Folly seems to be the most advanced library by far but is also missing non-blocking combinators.
Therefore, we need to provide a C++ library with all the missing non-blocking combinators.
The new library should also clarify the semantics of the different non-blocking combinators as well as shared and non-shared futures and promises.

## Advanced Futures and Promises
The project provides advanced futures and promises which are implemented with the help of Folly and Boost.Thread only at the moment.
The advanced futures and promises are declared in the namespace `adv` and provide the a basic functionality with extensions
which are missing from Folly and Scala.
The following classes and class templates are provided by the library:
* `adv::Try<T>` - Holds either nothing, a value or an exception and is used to store a future's result. We could use `folly::Try<T>` here instead of a custom type.
* `adv::Future<T>` - Class template for shared futures. Allows copying and multiple read semantics as well as registering multiple callbacks.
* `adv::Promise<T>` - Class template for shared promises. Allows copying.

With the help of only four core operations (`get`, `onComplete`, `isReady` and `tryComplete`) all other functions can be implemented.
Therefore, every library which is used to implement the advanced futures and promises has only to support these core operations.

#### Abstraction of the Core Operations
The class template `adv::Core<T>` has to be implemented.

## Performance Tests
The project provides several performance tests using the benchmark suite from Folly:
* [Shared vs unique future and promise creation](./src/performance/performance_shared.cpp) - Creates n unique and shared futures and promises from all three C++ libraries and compares the performance.
* [Recursive non-blocking combinator calls](./src/performance/performance_combinators.cpp) - Compares the performance of the different non-blocking combinators. It creates a binary tree with a fixed height per test case. Every node in the tree is the call of a non-blocking combinator.

### C++ Paper
We have written a paper about the advanced futures and promises called [Advanced Futures and Promises in C++](http://www.home.hs-karlsruhe.de/~suma0002/publications/advanced-futures-promises-cpp.pdf).
Here are some TODOs for the paper:
* Improve the description of the C++ syntax.
* Show examples in other programming languages like ConcurrentML etc. as comparison.
* Clarify the semantics of `Try`. `Try::get` throws an exception if the object is not initialized yet. Add the type `adv::UsingUninitializedTry`. Can the Try trait in Scala even be empty? In Folly it can be empty. What happens if `hasValue` and `hasException` are called? They should return `false` if the Try instance is empty. Make sure that `tryComplete` does also complete the promise/future with this exception when an empty Try is passed.
* Clarify the semantics of `Future::get`, `Future::then`, `Future::guard` etc. which should make the current future invalid (as in Folly). The latest version of Folly requires move semantics on the current future.
* Make sure that the second `get` call throws a not initialized exception (`adv::UsingUninitializedTry`? Or another exception type?)
* Apparently, `folly::SharedPromise::getFuture` makes a copy of the result value and therefore requires a copy-constructor for the inner type. It uses the copy constructor of `folly::Try`. Therefore, the signature of `adv_folly::SharedFuture::get` should not return a const reference but a copy since this what you get from Folly. The latest version of Folly returns a `SemiFuture`, so maybe update the whole class template. Besides, the class template `Try` needs a copy-constructor and an assignment operator in its declaration.
* Describe that `tryComplete` does not complete the promise when the `Try` object has not been initialized yet.
* Mention that `tryCompleteWith` relies on a longer lifetime of the promise than the future. What happens if the promise is destructed before the future is completed? Note that the implementation calls `onComplete` on the given future parameter and simply passes a copy of `this` (the promise). This pointer should become invalid when the promise is destructed. Fix the implementation by using a safe pointer which is set to null when the promise is destructed! Create a unit test for this case.
* Define a class `adv::BrokenPromise` exception which is thrown when `get` is called but the promise is destructed before completing the future.
* Add the method `SharedFuture<T> share();` for the later shown SharedFutures to the class `Future`.
* Unify the order of methods in all shown classes: First constructors and assignment operators, then basic methods and then derived methods.
* The two derived combinators `firstN` and `firstNSucc` are missing parameter names for the future vectors.
* The `Executor` type has to be usable for Boost.Thread which requires the template type of the used executor or hide the template type in the Boost.Thread implementation.
* Update the line `ctx−>v.emplace_back(i, std::move(t.get()));` of the `firstN` implementation. It should be `ctx−>v.emplace_back(i, std::move(t));` instead.
* Update the implementations of `firstN` and `firstNSucc` which had a possible data race when completing the promise with the vector.
* Since it is allowed to register only one callback per future, it should move out the state and maybe use && similiar to Folly which would require a `std::move` on every future.
* Instead of `then` we can use `onComplete` as basic method.
* The Scala FP `find` is implemented the way that one future after another is searched for the result. Hence, it cannot be used to implement `firstSucc` with a guard. TODO Look again at the implementation.
* Introduce the derived method `thenWith` similiar to Scala FP's `transformWith`. It has to be used to implement `orElse`. Do not ever use `get` to implement `orElse` since it is blocking. This would lead to a deadlock if there are not enough threads.

#### Boost.Thread Implementation
* Describe the implementation with the help of Boost.Thread.
* Mention that the Boost.Thread implementation has to use `boost::current_exception` instead of `std::current_exception`: <https://svn.boost.org/trac10/ticket/9710> and <https://stackoverflow.com/questions/52043506/how-to-rethrow-the-original-exception-stored-by-an-stdexception-ptr-with-a-boo>

#### Folly Changes
* Consider the updated Folly library which does now provide the type `folly::SemiFuture` and the executors which previously belonged to Wangle.
* The changes of Folly do also include changes of the non-blocking combinators such as `collectN` where the data race bug probably has been removed with the current implementation. Recheck it!
* Consider the following changes in the latest Folly version (done in August 2018):
```cpp
[[deprecated("must be rvalue-qualified, e.g., std::move(future).get()")]] T
get() & = delete;
```cpp
And also the following change:
```cpp
template <typename F, typename R = futures::detail::callableResult<T, F>>
  [[deprecated("must be rvalue-qualified, e.g., std::move(future).then(...)")]]
      typename R::Return
then(F&& func) & = delete;
```
* Besides, `folly::Future::filter` moves the current future and makes it invalid, too.
* `folly::Timeout` has been replaced by `folly::FutureTimeout`.
* The latest README.md file can be found [here](https://github.com/facebook/folly/blob/master/folly/docs/Futures.md) rather than in the futures source code directory.

#### Performance Analysis
* Update the performance analysis. Create several tables or plots: Folly, Boost.Thread, Adanced Futures and Promises implemented with Folly, Adanced Futures and Promises implemented with Boost.Thread.
* Test several executors in the empirical results section to show the usage of multiple cores.
* Produce longer durations rather than small ms durations.

#### Better Abstraction
These changes bring a lot of performance issues with them.
You should look into Rust futures which use traits -> more like template meta programming.
Maybe this approach would be better to avoid virtual methods and heap allocation.

* Make the classes `Try`, `Executor`, `Future`, `Promise` and `SharedFuture` abstract with virtual methods. The Folly and Boost.Thread implementations should inherit these classes. The abstract classes should be part of the namespace `adv`. The Folly and Boost.Thread implementations should have the namespaces `adv_folly` and `adv_boost`. They share generic classes such as `adv::PredicateNotFulfilled`. However, this is probably not possible for the classes `Future` and `SharedFuture` since the methods return stack-allocated objects of abstract types. It should also decrease the performance due to virtual methods.
* After making all basic classes abstract, try to implement the derived methods already in the abstract classes since they only require the basic methods.

#### New Features
Maybe add the new derived methods to `adv::Future`:
* `onSuccess`: Registers a callback which takes the successful result value. If the future has failed, it is not called. This method could be used in listing 4 to simplify the code.
* `onFail`: Takes the failed exception. If the future has been completed successfully, it is not called.