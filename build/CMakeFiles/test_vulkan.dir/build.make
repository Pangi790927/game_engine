# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pangi/workspace/game_engine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pangi/workspace/game_engine/build

# Include any dependencies generated for this target.
include CMakeFiles/test_vulkan.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_vulkan.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_vulkan.dir/flags.make

CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.o: CMakeFiles/test_vulkan.dir/flags.make
CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.o: ../tests/test_vulkan.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pangi/workspace/game_engine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.o -c /home/pangi/workspace/game_engine/tests/test_vulkan.cpp

CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pangi/workspace/game_engine/tests/test_vulkan.cpp > CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.i

CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pangi/workspace/game_engine/tests/test_vulkan.cpp -o CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.s

# Object files for target test_vulkan
test_vulkan_OBJECTS = \
"CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.o"

# External object files for target test_vulkan
test_vulkan_EXTERNAL_OBJECTS =

test_vulkan: CMakeFiles/test_vulkan.dir/tests/test_vulkan.cpp.o
test_vulkan: CMakeFiles/test_vulkan.dir/build.make
test_vulkan: CMakeFiles/test_vulkan.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pangi/workspace/game_engine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test_vulkan"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_vulkan.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_vulkan.dir/build: test_vulkan

.PHONY : CMakeFiles/test_vulkan.dir/build

CMakeFiles/test_vulkan.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_vulkan.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_vulkan.dir/clean

CMakeFiles/test_vulkan.dir/depend:
	cd /home/pangi/workspace/game_engine/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pangi/workspace/game_engine /home/pangi/workspace/game_engine /home/pangi/workspace/game_engine/build /home/pangi/workspace/game_engine/build /home/pangi/workspace/game_engine/build/CMakeFiles/test_vulkan.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_vulkan.dir/depend
