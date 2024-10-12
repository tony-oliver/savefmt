project( savefmt-docs )

find_package( Doxygen )

if ( DOXYGEN_FOUND )

	set( SOURCE_FILES ${CMAKE_SOURCE_DIR}/include )

    set_property( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
                  APPEND PROPERTY
                  OBJECT_DEPENDS ${SOURCE_FILES}/include/awo/savefmt.hpp )

    configure_file( Doxyfile . )

    add_custom_target( savefmt-docs ALL
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       COMMAND ${DOXYGEN_EXECUTABLE}
                       COMMENT "Generating API documentation with Doxygen..."
                       VERBATIM )

else ( DOXYGEN_FOUND )

    message( "Doxygen needs to be installed to generate the documentation" )

endif ( DOXYGEN_FOUND )
