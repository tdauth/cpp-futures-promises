include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(cpp_user_group_karlsruhe)
add_subdirectory(mvar)
add_subdirectory(performance)

install(FILES
    advanced_futures_promises.h
    core.h
    core_impl.h
    executor.h
    follyexecutor.h
    future.h
    future_impl.h
    promise.h
    promise_impl.h
    try.h
    DESTINATION include/cpp-futures-promises
)