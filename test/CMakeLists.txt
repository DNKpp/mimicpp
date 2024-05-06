add_subdirectory("unit-tests")

option(MIMICPP_ENABLE_ADAPTER_TESTS "Determines, whether the adapter tests shall be built." OFF)
if (MIMICPP_ENABLE_ADAPTER_TESTS)
	add_subdirectory("adapter-tests")
endif()
