file(GLOB_RECURSE SHARED_SRC_FILES *sh.cpp)
update_deps_file("${SHARED_SRC_FILES}")

file(GLOB_RECURSE STATIC_SRC_FILES *st.cpp)
update_deps_file("${STATIC_SRC_FILES}")

message("static source files: ${STATIC_SRC_FILES}")
message("shared source files: ${SHARED_SRC_FILES}")
add_library(game_engine STATIC ${STATIC_SRC_FILES})
add_library(game_engine_shared SHARED ${SHARED_SRC_FILES})
target_link_libraries(game_engine_shared PRIVATE shaderc_combined pthread)
target_link_libraries(game_engine PRIVATE -ldl)

# set(additional "${CMAKE_SOURCE_DIR}/extern/shaderc/libshaderc/libshaderc_combined.a")
# combine_static_lib(game_engine game_engine_ ${additional})