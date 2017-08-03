# Performance tests:
add_executable(performance_combinators performance_combinators.cpp)
add_dependencies(performance_combinators boost folly)
target_link_libraries(performance_combinators ${Boost_LIBRARIES} ${folly_LIBRARIES} pthread)
add_test(PerformanceCombinators performance_combinators)

# Don't optimize this test since the libraries might get different optimizations and some libraries might get better results for empty promises and futures!
add_executable(performance_shared performance_shared.cpp)
add_dependencies(performance_shared boost folly)
target_link_libraries(performance_shared ${Boost_LIBRARIES} ${folly_LIBRARIES} pthread)
add_test(PerformanceShared performance_shared)
