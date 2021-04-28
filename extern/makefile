# this is unused, kept as an example for now

all: vulkan_hpp

vulkan_hpp: vulkan_hpp.lock
	git clone --recurse-submodules https://github.com/KhronosGroup/Vulkan-Hpp.git
	mkdir Vulkan-Hpp/build
	cd Vulkan-Hpp/build && cmake -D DSAMPLES_BUILD_WITH_LOCAL_VULKAN_HPP=ON \
			-DSAMPLES_BUILD=ON -DTESTS_BUILD_WITH_LOCAL_VULKAN_HPP=ON \
			-DTESTS_BUILD=ON -G "Unix Makefiles" .. && cmake --build .
	touch vulkan_hpp.lock

clean:
	rm -rf vulkan_hpp.lock
	rm -rf Vulkan-Hpp