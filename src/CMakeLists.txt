include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(boost)
add_subdirectory(folly)
#add_subdirectory(performance)

install(FILES
    advanced_futures_boost.h
    advanced_futures_folly.h
    future.h
    promise.h
    shared_future.h
    DESTINATION include/cpp-futures-promises
)