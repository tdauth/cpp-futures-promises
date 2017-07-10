add_executable(currency currency.cpp)
target_link_libraries(currency pthread)

add_executable(currency_boost currency_boost.cpp)
add_dependencies(currency_boost boost)
target_link_libraries(currency_boost ${Boost_LIBRARIES})

add_executable(currency_folly currency_folly.cpp)
add_dependencies(currency_folly folly wangle)
target_link_libraries(currency_folly ${folly_LIBRARIES} ${wangle_LIBRARIES})

add_executable(currency_folly_or_else currency_folly_or_else.cpp)
add_dependencies(currency_folly_or_else folly wangle)
target_link_libraries(currency_folly_or_else ${folly_LIBRARIES} ${wangle_LIBRARIES})