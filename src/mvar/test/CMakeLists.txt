add_executable(advanced_mvar_future future.cpp)
add_dependencies(advanced_mvar_future folly)
target_link_libraries(advanced_mvar_future ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(AdvancedMVarFuture advanced_mvar_future)

add_executable(mvar mvar.cpp)
target_link_libraries(mvar ${Boost_LIBRARIES} ${PTHREAD_LIBRARY})
add_test(MVar mvar)