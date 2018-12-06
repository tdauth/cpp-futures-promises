#include <iostream>

#include <boost/thread.hpp>

using namespace boost;

int main()
{
    auto x1 = async([] () { return 10; }).share();
    // Creates a new non-shared future:
    auto f0 = x1.then([] (shared_future<int> f) {
        std::cerr << "The lord sometimes challenges us." << std::endl;
    });
    std::cerr << "Is valid: " << x1.valid() << std::endl;
    // Creates a new non-shared future:
    auto f1 = x1.then([] (shared_future<int> f) {
        std::cerr << "Aren't you a healer and a vessel for the holy spirit?" << std::endl;
    });
    std::cerr << "Is valid: " << x1.valid() << std::endl;

    x1.get();

    auto x2 = async([] () { return 10; });
    // Creates a new non-shared future:
    auto f2 = x2.then([] (shared_future<int> f) {
        std::cerr << "The lord sometimes challenges us." << std::endl;
    });
    std::cerr << "Is valid: " << x2.valid() << std::endl;
    // Fails!
    auto f3 = x2.then([] (shared_future<int> f) {
        std::cerr << "Aren't you a healer and a vessel for the holy spirit?" << std::endl;
    });
    std::cerr << "Is valid: " << x2.valid() << std::endl;

    x2.get();

    return 0;
}