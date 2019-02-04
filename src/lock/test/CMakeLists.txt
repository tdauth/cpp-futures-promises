add_executable(advanced_lock_future future.cpp)
add_dependencies(advanced_lock_future folly)
target_link_libraries(advanced_lock_future ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(AdvancedLockFuture advanced_lock_future)