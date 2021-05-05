file(GLOB_RECURSE SOURCE_FILES *.cpp)
update_deps_file("${SOURCE_FILES}")

message(${SOURCE_FILES})

add_custom_target(test COMMAND ${commands})

foreach (src ${SOURCE_FILES})
	get_filename_component(PROG ${src} NAME_WE)
	message(${PROG})
	add_executable(${PROG} ${src})
	# add_dependencies(${PROG} game_engine)
	target_link_libraries(${PROG} PRIVATE game_engine)
	add_custom_command(TARGET test POST_BUILD COMMAND
			${CMAKE_CURRENT_BINARY_DIR}/${PROG})
endforeach()

