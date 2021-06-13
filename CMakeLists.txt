cmake_minimum_required(VERSION 3.10)

# set the project name
project(vlog)

# add the path for include files
include_directories("${PROJECT_SOURCE_DIR}/include")

# search src file
aux_source_directory("${PROJECT_SOURCE_DIR}/src" VLOG_SRC)

# add library
add_library(vlog SHARED ${VLOG_SRC})
set(VLOG_LIBS ${VLOG_LIBS} vlog)

# install
install(TARGETS vlog DESTINATION lib)
install(FILES "${PROJECT_SOURCE_DIR}/include/vlog.h" DESTINATION include)

# should compile test
option(VLOG_TEST "test" OFF)
if(VLOG_TEST)
    include_directories("${PROJECT_SOURCE_DIR}/test")
    add_subdirectory(test)
endif()