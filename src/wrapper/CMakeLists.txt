# Wrapper
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(future future.cpp)
add_dependencies(future boost folly wangle)
target_link_libraries(future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(Future future)

add_executable(shared_future shared_future.cpp)
add_dependencies(shared_future boost folly wangle)
target_link_libraries(shared_future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES} pthread)
add_test(SharedFuture shared_future)

install(FILES future.h future_folly.h promise.h promise_folly.h shared_future_folly.h DESTINATION include/cpp-futures-promises)