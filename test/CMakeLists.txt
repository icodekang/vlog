aux_source_directory(. DIR_SRCS)

file(GLOB_RECURSE tests RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/test*.c")

foreach(file_path IN LISTS tests)
    get_filename_component(file_name ${file_path} NAME_WE)
    add_executable(${file_name} ${file_path})
    target_link_libraries(${file_name} pthread ${VLOG_LIBS})
    install(TARGETS ${file_name} DESTINATION test)
endforeach()