project( savefmt-tests )

find_package( GTest REQUIRED )

add_executable( test-savefmt
				test-savefmt.cpp
				)

target_include_directories( test-savefmt PUBLIC
							${CMAKE_SOURCE_DIR}/include )

target_link_libraries( test-savefmt
					   GTest::gtest_main
					   )
