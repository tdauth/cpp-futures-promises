# cpp-futures-promises
Examples and extensions of C++ futures and promises based on C++17, Boost.Thread and Folly.
The extensions are mainly based on the [Scala library for futures and promises](http://docs.scala-lang.org/overviews/core/futures.html) and functions missing from Folly.
This project provides an advanced futures and promises API based on our paper [Advanced Futures and Promises in C++](http://www.home.hs-karlsruhe.de/~suma0002/publications/advanced-futures-promises-cpp.pdf) which provides missing functions of C++ futures and promises.
The API is implemented with the help of only a few basic functions.
The API has a reference implementation which uses Folly.

## Automatic Build with TravisCI
[![Build Status](https://travis-ci.org/tdauth/cpp-futures-promises.svg?branch=master)](https://travis-ci.org/tdauth/cpp-futures-promises)

## Manual Build
To compile the project run the following script on Linux:
`bash ./build.sh`
It will compile the project, run all unit tests and create an RPM package.
Note that all targets are added as CTest unit tests which makes their execution easier.
The dependencies will be downloaded automatically.
Therefore, you need Internet access when building for the first time.

## Dependencies
The project requires the GCC with C++17 support and CMake to be built.

The project requires the following libraries:
* Boost
* Folly
* Wangle

It will download and compile these libraries automatically when being compiled.
The versions of the libraries are specified in the `CMakeLists.txt` file in the top level directory of the project.

Furthermore, the project requires the following packages on Fedora 25:
* cmake
* gcc-c++
* libatomic
* bash
* which
* rpm-build

Folly dependencies on Fedora 25:
* glog-devel
* gflags-devel
* valgrind
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

## Motivation
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
The project provides advanced futures and promises which are implemented for Folly and Boost.Thread only at the moment.
The advanced futures and promises are declared in the namespace `adv` and provide the a basic functionality with extensions
which are missing from Folly and Scala.
The following classes and class templates are provided by the library:
* `adv::Try<T>` - Holds either nothing, a value or an exception and is used to store a future's result.
* `adv::Executor` - Schedules the execution of concurrent function calls.
* `adv::Future<T>` - Non-sharable template class for futures. Allows only moving (not copying) and read once semantics as well as registering only exactly one callback.
* `adv::Promise<T>` - Non-sharable template class for promises. Allows only moving (not copying) and read once semantics as well as registering only exactly one callback.

With the help of only four basic functions (`get`, `then`, `isReady` and `tryComplete`) all other functions can be implemented.
Therefore, every library which is used to implement the advanced futures and promises has only to support these basic functions.

### Folly Implementation
The advanced futures and promises are implemented for the library Folly in this project since it provides the most extended interface for futures and promises in C++.
To use them you have to include the file `advanced/advanced_futures_folly.h`.
The classes wrap classes of Folly itself.

### Shared Futures for Folly
The advanced futures provide the shared future class template `adv::SharedFuture<T>`.
It allows copying the future around and multiple read semantics with `get` by default.
It does also allow registering more than one callback.
It is realized with the help of `folly::SharedPromise<T>` for the implementation of the library with Folly.

### Boost.Thread Implementation
The advanced futures and promises are also implemented for the library Boost.Thread in this project since it provides another extended interface for futures and promises in C++.
To use them you have to include the file `advanced/advanced_futures_boost.h`.
The classes wrap classes of Boost.Thread itself.
The Boost.Thread implementation uses the namespace `adv_boost` to distinguish it from the Folly implementation.

## Extensions
The project does also provide extensions for the libraries Folly and Boost.Thread.
The use them include the header `extensions.h`.

### Future Extensions for Folly
The project provides a number of non-blocking combinators which are missing from the existing C++ libraries:
* `orElse` - The same as `fallbackTo` in Scala.
* `first` - Returns the first completed future from two.
* `firstRandom` - The same but with shuffling.
* `firstOnlySucc` - Returns the first successfully completed future from two. If both fail, the program will starve.
* `firstOnlySuccRandom` - The same but with shuffling.
* `firstSucc` - Returns the first succesfully completed future from two. If both fail, the returned future will also fail. Implemented with the help of two orElse calls.
* `firstSuccRandom` - The same but with shuffling.
* `firstSucc2` - The same semantics but implemented differently with `orElse` and `onError`.
* `firstSuccRandom` - The same but with shuffling.
* `collectNWithoutException` - Like `folly::collectN` but collects n futures which have been completed successfully. It ignores failed futures. The returned future fails if the collection is two small or too many futures failed.

### Promise Extensions for Folly
The project provides extensions for promises which do exist in Scala but are missing from Folly:
* `tryComplete` - Tries to complete a promise with a `folly::Try` object. Returns if the completion was successful.
* `tryCompleteWith` - Tries to complete a promise with a `folly::Future` object in a non-blocking way. Returns the promise.
* `tryCompleteSuccess` - Tries to complete a promise with a value. Returns if the completion was successful.
* `tryCompleteSuccessWith` - Tries to complete a promise with a `folly::Future` object's successful result value in a non-blocking way. Returns the promise. If the future fails, the promise is not completed by it.
* `tryCompleteFailure` - Tries to complete a promise with an exception. Returns if the completion was successful.
* `tryCompleteFailureWith` - Tries to complete a promise with a `folly::Future` object's exception in a non-blocking way. Returns the promise. If the future succeeds, the promise is not completed by it.
* `fromTry` - Creates a completed promise from a `folly::Try` object.
* `failed` - Creates a failed promise from an exception.
* `successful` - Creates a successful promise from a value.

### Future Extensions for Boost.Thread
The project does also provide some of the extensions for Boost.Thread:
* `orElse - The same as `fallbackTo` in Scala.
* `whenN` - The same as `folly::collectN` but with `boost::future` instances in the resulting vector.
* `whenNSucc` - The same as `collectNWithoutException`.
* `whenAny` - Returns a future containing a pair of the completed future and its index similar to `folly::collectAny`. Boost.Thread's `boost::when_any` returns a future with the whole collection of futures instead.
* `whenAnySucc` - Returns a future containg a pair of the completed future's result value and its index similar to `folly::collectAnyWithoutException`.

## Unit Tests
To test the functionality, several unit tests are provided for the extensions.
Bear in mind that concurrent programs can behave differently every time due to scheduling.
Therefore, simple unit tests won't proof that there are no bugs.

## Performance Tests
The project provides several performance tests using the benchmark suite from Folly:
* Shared vs unique future and promise creation - Creates n unique and shared futures and promises from all three C++ libraries and compares the performance.
* Recursive non-blocking combinator calls - Compares the performance of the different non-blocking combinators. It creates a binary tree with a fixed height per test case. Every node in the tree is the call of a non-blocking combinator.

## Use Cases
The project provides several use cases for futures and promises to demonstrate their usage for solving problems.

### Currency Exchange
A simple use case of exchanging currencies with the help of futures and promises.
It is extended with guards in a second variant and finally with a choice between two exchanges in a third variant, but only for Folly.
This use case shows the limitations of the libraries.

### Santa Claus
Santa Claus sleeps and is woken up either by a group of reindeer or a group of elves ([Original Paper](http://dl.acm.org/citation.cfm?id=187391)).
In our variant he prefers to wait for the reindeer first and only if they could not make it he waits for the elves.
This is realized with the usage of `orElse`.
Depending on which group wakes up Santa, he either delivers toys or constructs them.