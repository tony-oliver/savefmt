#include "awo/savefmt.hpp"  // awo::basic_savefmt<>{} et al

#include <string>           // std::basic_string<>{}, std::string{}
#include <iomanip>          // std::setfill(), std::setw()
#include <ostream>          // std::basic_ostream<>{}, std::endl()
#include <utility>          // std::move<>()
#include <iostream>         // std::cerr, std::cout, std::wcout
#include <exception>        // std::exception{}

namespace { // unnamed

void test_constructors()
{
    using namespace awo;

    wsavefmt wsf;                   // wide-char default constructor

    std::cout << "default-construct sf1" << std::endl;
    savefmt sf1;                    // narrow-char default constructor
    std::cout << "sf1.stream(): " << sf1.stream() << std::endl;
    std::cout << std::endl;

    std::cout << "construct sf2 from std::cout" << std::endl;
    savefmt sf2{ std::cout };       // parametric constructor
    std::cout << "sf2.stream(): " << sf2.stream() << std::endl;
    std::cout << std::endl;

    std::cout << "move-construct sf3 from sf2" << std::endl;
    savefmt sf3 = std::move( sf2 );  // move constructor
    std::cout << "sf2.stream(): " << sf2.stream() << std::endl;
    std::cout << "sf3.stream(): " << sf3.stream() << std::endl;
    std::cout << std::endl;

    std::cout << "move-assign sf3 to sf2" << std::endl;
    sf2 = std::move( sf3 );         // move assignment
    std::cout << "sf2.stream(): " << sf2.stream() << std::endl;
    std::cout << "sf3.stream(): " << sf3.stream() << std::endl;
    std::cout << std::endl;
}

void write200()
{
    std::cout << "write200(): " << std::setw( 4 ) << 200 << std::endl;
}

void write200hex()
{
    // save the stream format before making formatting parameter changes
    awo::savefmt const saver{ std::cout };

    // arrange for subsequent numeric formatting to be in hex
    std::cout << std::hex << std::uppercase << std::setfill( '0' );

    // write 200 while the new formatting parameters are selected
    write200();

    // (destructor of saver will restore the previous formatting parameters)
}

void test_write200()
{
    write200();     // should write " 200"
    write200hex();  // should write "00C8"
    write200();     // should write " 200" again
}

template< typename CharT >
void test_savefmt_on( std::basic_ostream< CharT >& stream )
{
    using SaveFmt = awo::basic_savefmt< CharT >;

    stream << std::endl;
    stream << "TESTING INSERT OPERATOR FOR " << sizeof( CharT ) << "-BYTE CHAR STREAM" << std::endl;

    stream << "default: " << 42 << std::endl;
    stream << "(temporary) hex: " << SaveFmt{} << std::hex << std::uppercase << 42 << std::endl;
    stream << "restored: " << 42 << std::endl;

    stream << std::endl;
    stream << "TESTING STANDALONE SAVEFMT FOR " << sizeof( CharT ) << "-BYTE CHAR STREAM" << std::endl;
    SaveFmt saver; // inactive "standalone" savefmt instance

    stream << "default: "   << 42 << std::endl;
    saver.capture( stream );
    stream << "captured: "  << 42 << std::endl;
    stream << std::hex << std::uppercase;
    stream << "hex: "       << 42 << std::endl;
    stream << "again: "     << 42 << std::endl;
    saver.restore();
    stream << "restored: "  << 42 << std::endl;
    stream << std::hex << std::uppercase;
    stream << "hex: "       << 42 << std::endl;
    saver.release();
    stream << "released: "  << 42 << std::endl;
}

} // close unnamed namespace

int main()
{
    try
    {
        //awo::savefmt bad;
        //bad.restore();

        test_constructors();
        test_write200();

        test_savefmt_on( std::cout );
        test_savefmt_on( std::wcout );
    }
    catch ( std::exception const& e )
    {
        std::cerr << "exception: " << e.what() << std::endl;
    }
}
