#ifndef INCLUDED_AWO_SAVEFMT_HPP
#define INCLUDED_AWO_SAVEFMT_HPP

/*
Header file "awo/savefmt.hpp"

Copyright (c) 2005-2024, Tony Oliver (H D Computer Services Ltd.)

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation. H D Computer Services Ltd. makes no
representations about the suitability of this software for any
purpose. It is provided "as is" without express or implied warranty.

------------------------------------------------------------------------------

This class template provides a mechanism to save a stream's I/O formatting
properties and, at a later date, restore them to those that were previously
saved (both explicitly on request and via RAII on object destruction).

A simple example:

void report_hex( unsigned const n )
{
    awo::savefmt const saver{ std::cout };

    std::cout << std::hex << std::uppercase << n << std::endl;
}

On return from the function, the saver object's destructor will
return std::cout's formatting parameters back to how they were.

Another example:

void f()
{
    std::cout << std::dec << 200 << std::endl;
    std::cout << awo::savefmt{} << std::hex << 200 << std::endl;
    std::cout << 200 << std::endl;
}

The second reporting line changes the radix before formatting the
value but, at the end of the enclosing expression, the temporary
savefmt's destructor will restore the formatting parameters that
were in effect in the stream before the saver was created.

A variety of other, more complex, scenarios can be dealt with
using the default constructor, the move-constructor, move-assignment operator,
and the named member-functions capture(), restore() and release().

Since the introduction of rvalue-references to the C++ language,
this implementation has adopted their use, thereby circumventing
the dodgy-looking const-casting that helped facilitate the origin
formatted extraction/insertion (>>, <<) operators.
*/

/// @file awo/savefmt.hpp
/// @author Tony Oliver <tony@oliver.net>

// rvalue-references (and move semantics) require at least C++11 support.
// The function std::exchange<>() was introduced in the C++14 standard.

#if __cplusplus <= 201411L
#error Header file "awo/savefmt.hpp" requires at least C++14 capabilities.
#endif

#include <ios>          // std::basic_ios<>{}
#include <string>       // std::char_traits<>{}
#include <istream>      // std::basic_istream<>{}
#include <ostream>      // std::basic_ostream<>{}
#include <utility>      // std::exchange<>()

//============================================================================
/// This is the namespace in which all Tony Oliver's distributable components reside.
namespace awo {
//----------------------------------------------------------------------------

/// This is a template from which to create classes that can save/restore stream formatting-parameters.
///
/// When instantiated with an appropriate character type, creates a concrete class definition
/// which can subsequently be used to create saver/restorer objects.
///
/// @tparam CharT - The character type on which to instantiate this template.
/// @tparam Traits - The character-traits type on which to instantiate this template
/// (usually omitted and the char_traits<> default used).
///
/// One is generally expected to only instantiate this template over the character
/// types \b char and \b wchar_t (for which, see the pre-instantiated typedefs
/// \ref savefmt and \ref wsavefmt).

template< typename CharT, typename Traits = std::char_traits< CharT > >
class basic_savefmt
{
    /// The relevant base class of all streams of which we can save formatting parameters; <br>
    /// also the concrete class type into an instance of which the parameters are saved.
    using stream_base = std::basic_ios< CharT, Traits >;

    /// A record of which stream's formatting parameters we are holding; initially none.
    stream_base* bound_stream{ nullptr };

    /// An ios-based object (with no stream buffer) into which the parameters are saved.
    stream_base saved_format{ nullptr };

public:

    /// Default constructor: creates an inactive saver/restorer object.
    basic_savefmt() = default;

    /// Capturing constructor: saves parameters from (and a reference to) the given stream.
    explicit basic_savefmt( stream_base& stream );

    /// Objects of this type \a can be move-constructed in the normal manner.
    basic_savefmt( basic_savefmt&& other );

    /// Objects of this type \a cannot be copy-constructed.
    basic_savefmt( basic_savefmt const& ) = delete;

    /// If we have a stream's formatting parameters captured, the destructor restores them.
    ~basic_savefmt();

    /// Objects of this type \a can be move-assigned in the normal manner.
    /// @return \b *this as a \b basic_savefmt&
    basic_savefmt& operator=( basic_savefmt&& other );

    /// Objects of this type \a cannot be copy-assigned.
    basic_savefmt& operator=( basic_savefmt const& ) = delete;

    /// Save a stream's formatting parameters (possibly restoring any that are already captured).
    void capture( stream_base& stream );

    /// Restore saved parameters back to the stream from which they came.
    void restore();

    /// Reset this object such that it no longer holds a stream's parameters.
    void release();

    /// Reports the associated stream (whose formatting parameters have been saved).
    /// \return reference to the stream as a \b stream_base* if this object is "active";
    /// \return a null pointer if not.
    stream_base* stream() const;
};

/*------------------------------------------*\
|*  Stream extraction/insertion operators:  *|
\*------------------------------------------*/

/// Stream extraction-operator to handle savefmt instances appearing in \b operator>> chains.
template< typename CharT, typename Traits >
std::basic_istream< CharT, Traits >&
operator>>( std::basic_istream<CharT, Traits>& stream,
                 basic_savefmt<CharT, Traits>&& saver );

/// Stream insertion-operator to handle savefmt instances appearing in \b operator<< chains.
template< typename CharT, typename Traits >
std::basic_ostream< CharT, Traits >&
operator<<( std::basic_ostream<CharT, Traits>& stream,
                 basic_savefmt<CharT, Traits>&& saver );

/*------------------------------------------------------*\
|*  Specialisations for common stream character-types:  *|
\*------------------------------------------------------*/

/// Pre-declared instantiation and typedef of template \b basic_savefmt over the character-type \b char.
using savefmt = basic_savefmt< char >;

/// Pre-declared instantiation and typedef of template \b basic_savefmt over the character-type \b wchar_t.
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
    // Inactive instances ignore this request
    if ( bound_stream != nullptr )
    {
        // Restore the saved formatting parameters back to the stream.
        bound_stream->copyfmt( saved_format );
    }
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
auto
awo::basic_savefmt< CharT, Traits >::
stream() const -> stream_base*
{
    // Return a pointer to the stream to which we are bound (or nullptr).
    return bound_stream;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::
~basic_savefmt()
{
    // Restore any saved formatting parameters to their stream (if any).
    restore();
}

//============================================================================

template< typename CharT, typename Traits >
std::basic_istream< CharT, Traits >&
awo::operator>>( std::basic_istream<CharT, Traits>& stream,
                 awo::basic_savefmt<CharT, Traits>&& saver )
{
    // Capture the stream's formatting parameters.
    // Note: the saver object will expire at the end of the enclosing expression
    // and will therefore then restore the saved parameters back to the stream.
    saver.capture( stream );

    // Usual practice - return the stream reference for further chaining.
    return stream;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
std::basic_ostream< CharT, Traits >&
awo::operator<<( std::basic_ostream< CharT, Traits >& stream,
                 awo::basic_savefmt< CharT, Traits >&& saver )
{
    // Capture the stream's formatting parameters.
    // Note: the saver object will expire at the end of the enclosing expression
    // and will therefore then restore the saved parameters back to the stream.
    saver.capture( stream );

    // Usual practice - return the stream reference for further chaining.
    return stream;
}

//============================================================================

#endif // INCLUDED_AWO_SAVEFMT_HPP
