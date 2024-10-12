cmake_minimum_required(VERSION 3.27 FATAL_ERROR)
project( savefmt )

set( CMAKE_CXX_STANDARD_REQUIRED On )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_EXTENSIONS Off )

add_compile_options( -pedantic -Wall -Wextra -O3 -g3 )
# add_compile_options( -save-temps )

add_subdirectory( include )
add_subdirectory( test )
add_subdirectory( docs )
