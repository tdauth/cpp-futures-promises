[![Build Status](https://travis-ci.org/tdauth/cpp-futures-promises.svg?branch=master)](https://travis-ci.org/tdauth/cpp-futures-promises)

# cpp-futures-promises
Examples and extensions of C++ futures and promises based on C++17, Boost.Thread and Folly.
The extensions are mainly based on the [Scala library for futures and promises](http://docs.scala-lang.org/overviews/core/futures.html) and functions missing from Folly.
This project provides an advanced futures and promises API based on our paper [Advanced Futures and Promises in C++](http://www.home.hs-karlsruhe.de/~suma0002/publications/advanced-futures-promises-cpp.pdf) which adds missing functions of C++ futures and promises.
The API is implemented with the help of only a few basic functions. The API has a reference implementation with Folly.

## Compile
To compile the project run the following script on Linux:
`bash ./build.sh`
It will compile the project, run all unit tests and create an RPM package.
Note that all targets are added as CTest unit tests which makes their execution easier.

## Dependencies
The project requires the GCC with C++17 support and CMake to be built.

The project requires the following libraries:
* Boost
* Folly
* Wangle

It will download and compile these libraries automatically when being compiled.
The versions of the libraries are specified in the CMakeLists.txt file in the top level directory of the project.

Furthermore, it requires the following packages on Fedora 25:
* cmake
* gcc-c++
* bash
* which

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
There is several C++ libraries for futures and promises: The C++17 thread support library (standard library), Boost.Thread and Folly.
Qt provides futures support as well but no promises.
All of these libraries are missing functions especially compared to Scala's futures and promises library.
Folly seems to be the most advanced library by far but is also missing functions.
Therefore, we need to provide a C++ library with all the missing combinators which calrifies the semantics of the different combinators
and shared and non-shared futures and promises.

## Advanced Futures and Promises
The project provides advanced futures and promises which are implemented for Folly only at the moment.
The advanced futures and promises are declared in the namespace `adv` and provide the a basic functionality with extensions
which are missing from Folly and Scala.
The following classes are provided:
* adv::Try<T> - Holds either nothing, a value or an exception and is used to store a future's result.
* adv::Executor - Schedules the execution of concurrent function calls.
* adv::Future<T> - Non-sharable template class for futures. Allows only moving (not copying) and read once semantics as well as registering only exactly one callback.
* adv::Promise<T> - Non-sharable template class for promises. Allows only moving (not copying) and read once semantics as well as registering only exactly one callback.

With the help of only three basic functions (get, then, isReady and tryComplete) all other functions can be implemented.
Therefore, every library which implements the advanced futures and promises has only to support these basic functions.

### Folly Implementation
The advanced futures and promises are only implemented for the library Folly at the moment since it provides the most extended interface for futures and promises in C++.
To use them you have to include the file `advanced/advanced_futures_folly.h`.
The classes wrap objects of Folly classes.

### Shared Futures for Folly
The advanced futures provide the shared future class template `adv::SharedFuture<T>` for Folly. It allows copying the future around and
multiple read semantics with `get` by default. It does also allow registering more than one callback.
It is realized with the help of `folly::SharedPromise<T>`.

## Extensions
The project provides extensions for the libraries Folly and Boost.Thread.
The use them include the header `extensions.h`.

### Future extensions for Folly
The project provides a number of non-blocking combinators which are missing from the existing C++ libraries:
* orElse - The same as fallbackTo in Scala.
* first - Returns the first completed future from two.
* firstRandom - The same but with shuffling.
* firstOnlySucc - Returns the first successfully completed future from two. If both fail, the program will starve.
* firstOnlySuccRandom - The same but with shuffling.
* firstSucc- Returns the first succesfully completed future from two. If both fail, the returned future will also fail. Implemented with the help of two orElse calls.
* firstSuccRandom - The same but with shuffling.
* firstSucc2 - The same semantics but implemented differently with orElse and onError.
* firstSuccRandom - The same but with shuffling.
* collectNWithoutException - Like folly::collectN but collects n futures which have been completed successfully. It ignores failed futures. The returned future fails if the collection is two small or too many futures failed.

### Promise extensions for Folly
The project provides extensions for promises which do exist in Scala but are missing from Folly:
* tryComplete - Tries to complete a promise with a folly::Try object. Returns if the completion was successful.
* tryCompleteWith - Tries to complete a promise with a folly::Future object in a non-blocking way. Returns the promise.
* tryCompleteSuccess - Tries to complete a promise with a value. Returns if the completion was successful.
* tryCompleteSuccessWith - Tries to complete a promise with a folly::Future object's successful result value in a non-blocking way. Returns the promise. If the future fails, the promise is not completed by it.
* tryCompleteFailure - Tries to complete a promise with an exception. Returns if the completion was successful.
* tryCompleteFailureWith - Tries to complete a promise with a folly::Future object's exception in a non-blocking way. Returns the promise. If the future succeeds, the promise is not completed by it.
* fromTry - Creates a completed promise from a folly::Try object.
* failed - Creates a failed promise from an exception.
* successful - Creates a successful promise from a value.

### Future extensions for Boost.Thread
The project provides some of the extensions also for Boost.Thread:
* orElse - The same as fallbackTo in Scala.
* whenN - The same as folly::collectN but with boost::future instances in the resulting vector.
* whenNSucc - The same as collectNWithoutException.
* whenAny - Returns a future containing a pair of the completed future and its index similar to folly::collectAny. Boost.Thread's boost::when_any() returns a future with the whole collection of futures instead.

## Unit Tests
To test the functionality, several unit tests are provided for the extensions.
Bear in mind that concurrent programs can behavior differently every time due to scheduling.
Therefore, simple unit tests won't proof that there are no bugs.

## Performance Tests
The project provides several performance tests using the benchmark suite from Folly:
* Shared vs unique future and promise creation - Creates n unique and shared futures and promises from all three C++ libraries and compares the performance.
* Recursive non-blocking combinator calls - A binary tree with a fixed height is created. Every node is the call of a combinator.

## Use Cases
The project provides several use cases of futures and promises.

### Currency Exchange
For demonstrating the libraries, a simple use case of exchanging currencies is demonstrated for all three libraries.
It is extended with guards and finally a choice between two exchanges, but only for Folly.
This use case shows the limitations of the libraries.

### Santa Claus
Santa Claus sleeps and is woken up either by a group of reindeer or a group of elves.
In our variant he prefers to wait for the reindeer first and only if they could not make it he waits for the elves.
This is realiezd with the usage of orElse.
Depending on which group wakes up Santa, he either delivers toys or constructs them.

[Original Paper](http://dl.acm.org/citation.cfm?id=187391)