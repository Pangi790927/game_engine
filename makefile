CXX := g++
CXX_FILES := 
INCLUDES := include src
INCLUDES := $(INCLUDES:%=-I%)
CXX_FLAGS := -std=c++2a

all:
	echo "TODO: WIP"

shaderc_so:
	$(CXX) $(CXX_FLAGS) $(INCLUDES) \
			-Iextern/shaderc/libshaderc/include/ \
			-Lextern/shaderc/build/libshaderc/ \
			src/shader_compile.cpp -lshaderc_combined -o libshc.so -shared -fPIC

test_utils:
	$(CXX) $(CXX_FLAGS) $(INCLUDES) tests/test_utils.cpp -o test
	./test
	rm -f test

test_vulkan:
	$(CXX) $(CXX_FLAGS) $(INCLUDES) tests/test_vulkan.cpp \
			-lvulkan -ldl -lglfw -o test
	./test
	rm -f test
