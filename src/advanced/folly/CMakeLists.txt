add_subdirectory(test)

install(FILES
	future.h
	future_impl.h
	promise.h
	shared_future.h
	DESTINATION include/cpp-futures-promises/advanced/folly
)