add_subdirectory(test)

install(FILES
	future.h
	future_impl.h
	promise.h
	DESTINATION include/cpp-futures-promises/advanced/boost
)