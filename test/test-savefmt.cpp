#include "awo/savefmt.hpp"

#include <gtest/gtest.h>

#include <ostream>
#include <sstream>
#include <utility>
#include <iostream>

namespace { // unnamed

TEST( SaveFormatTestSuite, DefaultConstructorWide )
{
    awo::wsavefmt const saver;

    EXPECT_EQ( saver.stream(), nullptr );
}

TEST( SaveFormatTestSuite, DefaultConstructor )
{
    awo::savefmt const saver;

    EXPECT_EQ( saver.stream(), nullptr );
}

TEST( SaveFormatTestSuite, CapturingConstructor )
{
    awo::savefmt const saver( std::cout );

    EXPECT_EQ( saver.stream(), &std::cout );
}

TEST( SaveFormatTestSuite, MoveConstructFromEmpty )
{
    awo::savefmt src;
    awo::savefmt const dst( std::move( src ) );

    EXPECT_EQ( src.stream(), nullptr );
    EXPECT_EQ( dst.stream(), nullptr );
}

TEST( SaveFormatTestSuite, MoveConstructFromActive )
{
    awo::savefmt src( std::cout );
    awo::savefmt const dst( std::move( src ) );

    EXPECT_EQ( src.stream(), nullptr );
    EXPECT_EQ( dst.stream(), &std::cout );
}

TEST( SaveFormatTestSuite, MoveAssignFromEmpty )
{
    awo::savefmt dst;
    awo::savefmt src;
    dst = std::move( src );

    EXPECT_EQ( src.stream(), nullptr );
    EXPECT_EQ( dst.stream(), nullptr );
}

TEST( SaveFormatTestSuite, MoveAssignFromActive )
{
    awo::savefmt dst;
    awo::savefmt src( std::cout );
    dst = std::move( src );

    EXPECT_EQ( src.stream(), nullptr );
    EXPECT_EQ( dst.stream(), &std::cout );
}

void write200( std::ostream& out )
{
    out << std::setw( 4 ) << 200 << std::endl;
}

void write200hex( std::ostream& out )
{
    // save the stream format before making formatting parameter changes
    awo::savefmt const saver{ out };

    // arrange for subsequent numeric formatting to be in hex
    out << std::hex << std::uppercase << std::setfill( '0' );

    // write 200 while the new formatting parameters are selected
    write200( out );

    // (destructor of saver will restore the previous formatting parameters)
}

TEST( SaveFormatTestSuite, TestWrite200s )
{
    std::ostringstream out;

    write200( out );     // should write " 200"
    write200hex( out );  // should write "00C8"
    write200( out );     // should write " 200" again

    EXPECT_EQ
    (   out.str(),
        " 200" "\n"
        "00C8" "\n"
        " 200" "\n"
    );
}

TEST( SaveFormatTestSuite, TestTempObjectInline )
{
    std::ostringstream out;

    out << std::setw( 4 ) << 200 << std::endl;
    out << awo::savefmt{} << std::hex << std::uppercase << std::setfill( '0' )
        << std::setw( 4 ) << 200 << std::endl;
    out << std::setw( 4 ) << 200 << std::endl;

    EXPECT_EQ
    (   out.str(),
        " 200" "\n"
        "00C8" "\n"
        " 200" "\n"
    );
}

} // close unnamed namespace
