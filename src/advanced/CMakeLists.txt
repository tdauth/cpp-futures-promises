include_directories("${CMAKE_SOURCE_DIR}/src")

add_subdirectory(boost)
add_subdirectory(folly)

install(FILES
    advanced_futures_boost.h
    advanced_futures_folly.h
    future.h
    promise.h
    shared_future.h
    DESTINATION include/cpp-futures-promises/advanced
)
