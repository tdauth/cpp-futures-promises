add_executable(currency_cpp17_sequential currency_cpp17_sequential.cpp)
target_link_libraries(currency_cpp17_sequential pthread)
add_test(CurrencyCpp17Sequential currency_cpp17_sequential)

add_executable(currency_cpp17 currency_cpp17.cpp)
target_link_libraries(currency_cpp17 pthread)
add_test(CurrencyCpp17 currency_cpp17)

add_executable(currency_boost currency_boost.cpp)
add_dependencies(currency_boost boost)
target_link_libraries(currency_boost ${Boost_LIBRARIES})
add_test(CurrencyBoost currency_boost)

add_executable(currency_folly currency_folly.cpp)
add_dependencies(currency_folly folly wangle)
target_link_libraries(currency_folly ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(CurrencyFolly currency_folly)

add_executable(currency_guard_cpp17 currency_guard_cpp17.cpp)
target_link_libraries(currency_guard_cpp17 pthread)
add_test(CurrencyGuardCpp17 currency_guard_cpp17)

add_executable(currency_guard_boost currency_guard_boost.cpp)
add_dependencies(currency_guard_boost boost)
target_link_libraries(currency_guard_boost ${Boost_LIBRARIES})
add_test(CurrencyGuardBoost currency_guard_boost)

add_executable(currency_guard_folly currency_guard_folly.cpp)
add_dependencies(currency_guard_folly folly wangle)
target_link_libraries(currency_guard_folly ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(CurrencyGuardFolly currency_guard_folly)

add_executable(currency_guard_choice_folly currency_guard_choice_folly.cpp)
add_dependencies(currency_guard_choice_folly folly wangle)
target_link_libraries(currency_guard_choice_folly ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(CurrencyGuardChoiceFolly currency_guard_choice_folly)

add_executable(invalid_boost invalid_boost.cpp)
add_dependencies(invalid_boost boost)
target_link_libraries(invalid_boost ${Boost_LIBRARIES})
add_test(InvalidBoost invalid_boost)

add_executable(invalid_folly invalid_folly.cpp)
add_dependencies(invalid_folly folly)
target_link_libraries(invalid_folly ${folly_LIBRARIES})
add_test(InvalidFolly invalid_folly)

add_executable(valid_folly valid_folly.cpp)
add_dependencies(valid_folly folly)
target_link_libraries(valid_folly ${folly_LIBRARIES})
add_test(ValidFolly valid_folly)

add_executable(shared_boost shared_boost.cpp)
add_dependencies(shared_boost boost)
target_link_libraries(shared_boost ${Boost_LIBRARIES})
add_test(SharedBoost shared_boost)

add_executable(promise_boost promise_boost.cpp)
add_dependencies(promise_boost boost)
target_link_libraries(promise_boost ${Boost_LIBRARIES})
add_test(PromiseBoost promise_boost)

add_executable(promise_shared_folly promise_shared_folly.cpp)
add_dependencies(promise_shared_folly folly)
target_link_libraries(promise_shared_folly ${folly_LIBRARIES})
add_test(PromiseSharedFolly promise_shared_folly)

add_executable(callback_boost callback_boost.cpp)
add_dependencies(callback_boost boost)
target_link_libraries(callback_boost ${Boost_LIBRARIES})
add_test(CallbackBoost callback_boost)

add_executable(callback_folly callback_folly.cpp)
add_dependencies(callback_folly folly)
target_link_libraries(callback_folly ${folly_LIBRARIES})
add_test(CallbackFolly callback_folly)

add_executable(combinators_boost combinators_boost.cpp)
add_dependencies(combinators_boost boost)
target_link_libraries(combinators_boost ${Boost_LIBRARIES} pthread)
add_test(CombinatorsBoost combinators_boost)

add_executable(combinators_folly combinators_folly.cpp)
add_dependencies(combinators_folly folly)
target_link_libraries(combinators_folly ${folly_LIBRARIES})
add_test(CombinatorsFolly combinators_folly)

add_executable(extensions extensions.cpp)
add_dependencies(extensions boost folly)
target_link_libraries(extensions ${Boost_LIBRARIES} ${folly_LIBRARIES} pthread)
add_test(Extensions extensions)

# Performance tests:
add_executable(performance_combinators performance_combinators.cpp)
add_dependencies(performance_combinators boostrelease follyrelease)
target_link_libraries(performance_combinators ${Boost_RELEASE_LIBRARIES} ${folly_RELEASE_LIBRARIES} pthread)
add_test(PerformanceCombinators performance_combinators)
get_target_property(PERFORMANCE_COMBINATORS_FLAGS performance_combinators LINK_LIBRARIES)
message(STATUS "Link libraries of performance test combinators (should be the release versions): ${PERFORMANCE_COMBINATORS_FLAGS}")

add_executable(performance_shared performance_shared.cpp)
add_dependencies(performance_shared boostrelease follyrelease)
target_link_libraries(performance_shared ${Boost_RELEASE_LIBRARIES} ${folly_RELEASE_LIBRARIES} pthread)
add_test(PerformanceShared performance_shared)
get_target_property(PERFORMANCE_SHARED_FLAGS performance_shared LINK_LIBRARIES)
message(STATUS "Link libraries of performance test shared (should be the release versions): ${PERFORMANCE_SHARED_FLAGS}")

# Use Cases:
add_executable(santa_boost santa_boost.cpp)
add_dependencies(santa_boost boost folly)
target_link_libraries(santa_boost ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(SantaBoost santa_boost)

add_executable(santa_folly santa_folly.cpp)
add_dependencies(santa_folly boost folly wangle)
target_link_libraries(santa_folly ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(SantaFolly santa_folly)

install(FILES extensions.h random.h DESTINATION include/cpp-futures-promises)