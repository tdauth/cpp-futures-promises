add_executable(advanced_folly_future future.cpp)
add_dependencies(advanced_folly_future boost folly wangle)
target_link_libraries(advanced_folly_future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(AdvancedFollyFuture advanced_folly_future)

# TODO Fix copying/read-only semantics of shared futures
#add_executable(advanced_folly_shared_future shared_future.cpp)
#add_dependencies(advanced_folly_shared_future boost folly wangle)
#target_link_libraries(advanced_folly_shared_future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
#add_test(AdvancedFollySharedFuture advanced_folly_shared_future)