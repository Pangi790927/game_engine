# game_engine

game_engiene is a future component of GameWorkspace and will be used there if
it proves itself

the aim is to fix some problems with how GameWorkspace is structured now:
- engine and games look the same
- too many component folders when only one is needed
- no .o objects make compiling hard
- used own window library and it became boring to maintain
- did not have tests
- I don't like the naming convention anymore and changing it is hard
- some usefull libs don't fit well
- is not platform independent
- debug support is almost inexistent

to fix those problems:
- will use glfw, glew, vulkan, imgui, sol2+luajit, shaderc
- will change naming convention and rewrite some code to better fit my coding
style
- will add testers
- will make some parts of the final product libxxx.a but most will remain
headers
- will work with cmake
- will compile for Linux, Windows and maybe MacOS
- will only have a lib, src, and include folder for libs
- will have the following additional dirs: experiments, shaders, fonts, tests,
extern, assets, docs.
- add DBG and EXCEPTION and make error handling a priority
- make DBG also print c++ objects that either: 1. have to_string 2. can be
used by std::cout 3. can convert to std::string
- will create a better drawing pipeline, based on framebufers, shaders, meshes
and so on
- will use c++20
- try to remove dependencies for whoever uses the engine
- find out how to add a license

in extern:
- glew
- sol2
- imgui

General rules:
- all folders must end with / in memory and Windows must also use / not \
