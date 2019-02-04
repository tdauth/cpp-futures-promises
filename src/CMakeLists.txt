include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(lock)
add_subdirectory(performance)

install(FILES
        advanced_futures_lock.h
    future.h
    future_impl.h
    promise.h
    promise_impl.h
    state.h
    state_impl.h
    try.h
    DESTINATION include/cpp-futures-promises
)