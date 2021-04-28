# here CXX_INCLUDE CXX_LIBS CXX_FLAGS should be filled with platform specific
# flags and we can simply build the engine

do-common: do-tests

do-tests:
	$(CXX) tests/test_vulkan.cpp $(CXX_INCLUDE) $(CXX_LIBS) $(CXX_FLAGS)
	./a.out

