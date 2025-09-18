project( savefmt-docs )

find_package( Doxygen REQUIRED )

set( SOURCE_FILES ${CMAKE_SOURCE_DIR}/include/awo )

set_property( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
                APPEND PROPERTY
                OBJECT_DEPENDS ${SOURCE_FILES}/awo/savefmt.hpp )

configure_file( Doxyfile . )

add_custom_target( savefmt-docs ALL
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMAND ${DOXYGEN_EXECUTABLE} -q
                    COMMENT "Generating API documentation with Doxygen..."
                    VERBATIM )
