include_directories("${PROJECT_SOURCE_DIR}")

add_executable(santa_advanced santa_advanced.cpp)
add_dependencies(santa_advanced boost folly)
target_link_libraries(santa_advanced ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(SantaAdvanced santa_advanced)

add_executable(santa_boost santa_boost.cpp)
add_dependencies(santa_boost boost folly)
target_link_libraries(santa_boost ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(SantaBoost santa_boost)

add_executable(santa_folly santa_folly.cpp)
add_dependencies(santa_folly boost folly)
target_link_libraries(santa_folly ${Boost_LIBRARIES} ${folly_LIBRARIES})
add_test(SantaFolly santa_folly)