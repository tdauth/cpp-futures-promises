# Wrapper
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(future future.cpp)
add_dependencies(future boost folly wangle)
target_link_libraries(future ${Boost_LIBRARIES} ${folly_LIBRARIES} ${wangle_LIBRARIES})
add_test(Future future)

install(FILES future.h promise.h future_folly.h extensions.h random.h DESTINATION include/cpp-futures-promises)