include_directories("${CMAKE_CURRENT_SOURCE_DIR}/advanced/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(extensions extensions.cpp)
add_dependencies(extensions boost folly)
target_link_libraries(extensions ${Boost_LIBRARIES} ${folly_LIBRARIES} pthread)
add_test(Extensions extensions)

install(FILES extensions.h random.h DESTINATION include/cpp-futures-promises)

add_subdirectory(advanced)
add_subdirectory(currency)
add_subdirectory(performance)
add_subdirectory(santa)
