add_subdirectory(test)

install(FILES
        executor.h
        future.h
        promise.h
        DESTINATION include/cpp-futures-promises/advanced/boost
        )