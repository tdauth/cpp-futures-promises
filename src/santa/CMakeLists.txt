# Use Cases:
add_executable(santa_boost santa_boost.cpp)
add_dependencies(santa_boost boost folly)
target_link_libraries(santa_boost ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(SantaBoost santa_boost)

add_executable(santa_folly santa_folly.cpp)
add_dependencies(santa_folly boost folly wangle)
target_link_libraries(santa_folly ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(SantaFolly santa_folly)
