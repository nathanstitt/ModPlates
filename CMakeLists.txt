
# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (Plates)

set(CMAKE_CXX_FLAGS "-g -Wall")

set (Plates_VERSION_MAJOR 1)
set (Plates_VERSION_MINOR 0)


configure_file (
  "${PROJECT_SOURCE_DIR}/plates-config.h.in"
  "${PROJECT_BINARY_DIR}/plates-config.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")
  
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_subdirectory( src )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")


# enable testing
enable_testing ()

# does the application run
add_test (PlatesRuns src/plates 25)

add_test (PlatesComp25 src/plates 25)
set_tests_properties (PlatesComp25 
  PROPERTIES PASS_REGULAR_EXPRESSION "25 is 5"
)





