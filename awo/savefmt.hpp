/*
Header file "awo/savefmt.hpp"

Copyright (c) 2005-2023: Tony Oliver, H D Computer Services Ltd.

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation. H D Computer Services Ltd. makes no
representations about the suitability of this software for any
purpose. It is provided "as is" without express or implied warranty.

------------------------------------------------------------------------------

This class template provides the ability to save a stream's I/O format
settings and, at a later date, restore them to those that were previously
saved (both explicitly on request and via RAII on object destruction).

A simple example:

void report( int const n )
{
    awo::savefmt const saver{ std::cout };

    std::cout << std::hex << std::uppercase << n << std::endl;
}

On return from the function, the saver object's destructor will
return std::cout's formatting parameters back to how they were.

Another example:

void f()
{
    std::cout << 200 << std::endl;
    std::cout << awo::savefmt{} << std::hex << 200 << std::endl;
    std::cout << 200 << std::endl;
}

The second reporting line changes the radix before formatting the
value but, at the end of the enclosing expression, the temporary
savefmt's destructor will restore the formatting parameters that
were in effect in the stream before the saver was created.

A variety of other, more complex, scenarios can be dealt with
using the default constructor, capture(), restore(), release(),
the move-constructor, move-assignment operator and operator bool.
*/

#ifndef INCLUDED_AWO_SAVEFMT_HPP
#define INCLUDED_AWO_SAVEFMT_HPP

#if __cplusplus <= 201411L
#error Header file "awo/savefmt.hpp" requires at least C++14 capabilities.
#endif

#include <ios>
#include <string>
#include <istream>
#include <ostream>
#include <utility>
#include <stdexcept>

//============================================================================
namespace awo {
//----------------------------------------------------------------------------

template< typename CharT, typename Traits = std::char_traits< CharT > >
class basic_savefmt
{
    using stream_base = std::basic_ios< CharT, Traits >;

    stream_base* bound_stream{ nullptr }; // by default, not bound to any stream
    stream_base  saved_format{ nullptr }; // an ios object with no stream-buffer

public:

    // Default/capturing constructors
    basic_savefmt() = default;
    explicit basic_savefmt( stream_base& stream );

    // Objects of this type can be moved
    basic_savefmt( basic_savefmt&& other );
    basic_savefmt& operator=( basic_savefmt&& other );

    // Objects of this type cannot be copied
    basic_savefmt( basic_savefmt const& ) = delete;
    basic_savefmt& operator=( basic_savefmt const& ) = delete;

    // Primitive operations
    void capture( stream_base& stream );    // get & save stream's format info
    void restore();                         // put saved format back to stream
    void release();                         // forget stream-saved format info

    // Report whether this instance is currently active
    explicit operator bool() const;

    // Determine which stream we are bound to (returns nullptr if none)
    stream_base* stream() const;

    // Destructor auto-restores if we have a stream's format captured
    ~basic_savefmt();

    // Class-associated exception type (behaves exacly like std::runtime_error)
    class exception;
};

/*------------------------------------------*\
|*  Stream extraction/insertion operators:  *|
\*------------------------------------------*/

template< typename CharT, typename Traits >
std::basic_istream< CharT, Traits >&
operator>>( std::basic_istream<CharT, Traits>& stream,
                 basic_savefmt<CharT, Traits>&& saver );

template< typename CharT, typename Traits >
std::basic_ostream< CharT, Traits >&
operator<<( std::basic_ostream<CharT, Traits>& stream,
                 basic_savefmt<CharT, Traits>&& saver );

/*------------------------------------------------------*\
|*  Specialisations for common stream character-types:  *|
\*------------------------------------------------------*/

using  savefmt = basic_savefmt< char >;
using wsavefmt = basic_savefmt< wchar_t >;

//----------------------------------------------------------------------------
} // close namespace awo
//============================================================================

/*==============================================*\
|*                                              *|
|*  I M P L E M E N T A T I O N   D E T A I L   *|
|*                                              *|
\*==============================================*/

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::
basic_savefmt( stream_base& stream )
: bound_stream{ &stream }
{
    // We've now bound this instance to the given stream (above).

    // Capture its current formatting parameters for later restoration.
    saved_format.copyfmt( stream );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::
basic_savefmt( basic_savefmt&& other )
: bound_stream{ std::exchange( other.bound_stream, nullptr ) }
{
    // We've bound this instance to the stream previously bound-to by the
    // other instance and unbound that other instance from the stream (above).

    // Copy the formatting parameters already saved in the other instance.
    saved_format.copyfmt( other.saved_format );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
auto
awo::basic_savefmt< CharT, Traits >::
operator=( basic_savefmt&& other )
-> basic_savefmt&
{
    if ( &other != this )
    {
        // Bind to the other instance's stream and unbind that other from it.
        bound_stream = std::exchange( other.bound_stream, nullptr );

        // Capture the formatting parameters previously saved in the other instance.
        saved_format.copyfmt( other.saved_format );
    }

    return *this;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
void
awo::basic_savefmt< CharT, Traits >::
capture( stream_base& stream )
{
    // If we are currently active, restore the saved parameters to the stream.
    if ( bound_stream != nullptr )
    {
        bound_stream->copyfmt( saved_format ); // this is an unchecked restore()
    }

    // Now bind to the new stream.
    bound_stream = &stream;

    // And capture its current formatting parameters for later restoration.
    saved_format.copyfmt( stream );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
void
awo::basic_savefmt< CharT, Traits >::
restore()
{
    // Inactive instances have nowhere to restore formatting parameters to.
    if ( bound_stream == nullptr )
    {
        throw exception( "savefmt was asked to restore() when not bound to a stream" );
    }

    // Restore the saved formatting parameters back to the stream.
    bound_stream->copyfmt( saved_format );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
void
awo::basic_savefmt< CharT, Traits >::
release()
{
    // Unbind from the stream, so the saved parameters will not be restored.
    bound_stream = nullptr;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::
operator bool() const
{
    // We are active if we are bound to any stream (and hold its parameters).
    return bound_stream != nullptr;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
auto
awo::basic_savefmt< CharT, Traits >::
stream() const -> stream_base*
{
    // Simply return a pointer to the stream to which we are bound (or nullptr).
    return bound_stream;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::
~basic_savefmt()
{
    // RAII (in active instances) restores the saved parameters to the stream.
    if ( bound_stream != nullptr )
    {
        bound_stream->copyfmt( saved_format ); // this is an unchecked restore()
    }
}

//============================================================================

template< typename CharT, typename Traits >
class awo::basic_savefmt< CharT, Traits >::exception: public std::runtime_error
{
public:

    // Although this is a distinct type, it behaves exactly like std:::runtime_error
    using runtime_error::runtime_error;
};

//============================================================================

template< typename CharT, typename Traits >
std::basic_istream< CharT, Traits >&
awo::operator>>( std::basic_istream<CharT, Traits>& stream,
                 awo::basic_savefmt<CharT, Traits>&& saver )
{
    // Capture the stream's formatting parameters.
    // Note: the saver will expire at the end of the enclosing expression
    // and will therefore restore the saved parameters back to the stream.
    saver.capture( stream );

    // Usual practice - return the stream referenec for further chaining.
    return stream;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
std::basic_ostream< CharT, Traits >&
awo::operator<<( std::basic_ostream< CharT, Traits >& stream,
                 awo::basic_savefmt< CharT, Traits >&& saver )
{
    // Capture the stream's formatting parameters.
    // Note: the saver will expire at the end of the enclosing expression
    // and will therefore restore the saved parameters back to the stream.
    saver.capture( stream );

    // Usual practice - return the stream reference for further chaining.
    return stream;
}

//============================================================================

#endif // INCLUDED_AWO_SAVEFMT_HPP
