# Wrapper
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(advanced_future future.cpp)
add_dependencies(advanced_future boost folly wangle)
target_link_libraries(advanced_future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(AdvancedFuture advanced_future)

add_executable(advanced_shared_future shared_future.cpp)
add_dependencies(advanced_shared_future boost folly wangle)
target_link_libraries(advanced_shared_future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES} pthread)
add_test(AdvancedSharedFuture advanced_shared_future)

install(FILES
    advanced_futures_boost.h
    advanced_futures_folly.h
    future.h
    future_boost.h
    future_boost_impl.h
    future_folly.h
    future_folly_impl.h
    promise.h
    promise_boost.h
    promise_folly.h
    shared_future_folly.h
    DESTINATION include/cpp-futures-promises/advanced
)
