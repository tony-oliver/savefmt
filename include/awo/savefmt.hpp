#ifndef INCLUDED_AWO_SAVEFMT_HPP
#define INCLUDED_AWO_SAVEFMT_HPP

/*****************************************************************************

Header file "awo/savefmt.hpp"

Copyright © 2005-2025, Tony Oliver <tony@oliver.net>. All rights reserved.
The author, being Tony Oliver, has asserted his moral rights.

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation. The author makes no representations
about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

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

On return from the function, the saver object's destructor will return
std::cout's formatting parameters back to how they were beforehand.

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
using the move-constructor, move-assignment operator and the
named member-functions capture(), restore() and release().

Since the introduction of rvalue-references to the C++ language,
this implementation has adopted their use, thereby obviating the
dreadful const-casting that was used in the original (pre-C++11)
definitions of the stream extraction/insertion (>>, <<) operators.

Since the C++14 introduction of function std::exchange() in <utility>,
savefmt has adopted its use in the move-constructor and move-assignment
operator.

*****************************************************************************/

/// @file awo/savefmt.hpp
/// @author Tony Oliver <tony@oliver.net>

/*------------------------------*\
|*  Compiler-capability checks: *|
\*------------------------------*/

#ifndef __cplusplus
#error Header file "awo/savefmt.hpp" requires the definition of __cplusplus.
#endif

#define CPLUSPLUS_98    199711L
#define CPLUSPLUS_11    201103L
#define CPLUSPLUS_14    201402L
#define CPLUSPLUS_17    201703L
#define CPLUSPLUS_20    202002L
#define CPLUSPLUS_23    202302L

// RValue-references (and move semantics) require at least C++11 support.
// The function std::exchange<>() was introduced in the C++14 standard.
#if __cplusplus < CPLUSPLUS_14
#error Header file "awo/savefmt.hpp" requires at least C++14 capabilities.
#endif

// Required standard-library headers:

#include <ios>          // std::basic_ios<>{}
#include <string>       // std::char_traits<>{}
#include <istream>      // std::basic_istream<>{}
#include <ostream>      // std::basic_ostream<>{}
#include <utility>      // std::exchange<>()

//============================================================================
namespace awo { // The namespace in which Tony Oliver's works usually appear.
//----------------------------------------------------------------------------

/// This is a class template from which stream formatting parameters' save/restore objects may be created.

/// When instantiated with an appropriate character type, the template creates a concrete class definition
/// which can subsequently be used to create saver/restorer objects.
///
/// @tparam CharT - The character type on which to instantiate this template.
/// @tparam Traits - The character-traits type on which to instantiate this template
/// (usually omitted and the \b std::char_traits< CharT > default used).
///
/// One is generally expected to only instantiate this template over the character
/// types \b char and \b wchar_t (for which, see the typedefs \ref savefmt and \ref wsavefmt).
///
/// The capture/restoration of stream formatting parameters is achieved using the function
/// \b std::basic_ios< CharT, Traits >::copyfmt(),\n 
/// which can be found documented here: https://en.cppreference.com/w/cpp/io/basic_ios/copyfmt

template< typename CharT, typename Traits = std::char_traits< CharT > >
class basic_savefmt
{
    /// The base class type of all streams from which we can save formatting parameters; <br>
    /// also the concrete class type into an instance of which the parameters will be saved.
    using streambase_t = std::basic_ios< CharT, Traits >;

public:

    /// Default constructor: creates an empty ("inactive") object.
    basic_savefmt() = default;

    /// Capturing constructor: saves formatting parameters from (and a reference to) the given stream.
    /// @param stream - the stream whose formatting parameters are to be captured.
    explicit basic_savefmt( streambase_t& stream );

    /// Objects of this type \a can be move-constructed from other instances.
    /// @param other - the savefmt instance from which to be moved.
    basic_savefmt( basic_savefmt&& other );

    /// Objects of this type \a cannot be copy-constructed.
    basic_savefmt( basic_savefmt const& ) = delete;

    /// If this instance has a stream's formatting parameters captured,
    /// the destructor restores them.
    virtual ~basic_savefmt();

    /// Objects of this type \a can be move-assigned in the normal manner.
    /// @param other - the savefmt instance to be moved into this instance.
    /// @return \b *this as a \b basic_savefmt&
    basic_savefmt& operator=( basic_savefmt&& other );

    /// Objects of this type \a cannot be copy-assigned.
    basic_savefmt& operator=( basic_savefmt const& ) = delete;

    /// Save a stream's formatting parameters (possibly restoring any that have
    /// previously been captured from another stream and not released - see below).
    /// @param stream - the stream whose formatting parameters are to be captured.
    void capture( streambase_t& stream );

    /// Restore any saved parameters back to the stream from which they came.
    ///
    /// Unless explicitly requested (by passing \b true as its parameter),
    /// this does \a not release them, allowing multiple \b restore() operations
    /// while this instance has the stream's original parameters captured.
    /// @param also_release - whether or not to also clear (release) this instance -
    /// see \b release() below.
    void restore( bool also_release = false );

    /// Reset this object such that it no longer holds any stream's parameters.
    void release();

    /// Reports the associated stream (whose formatting parameters have been saved).
    /// \return a reference to the stream as a \b streambase_t* if this object is "active";
    /// \return a null pointer if not.
    streambase_t* stream() const;

protected:

    /// A record of which stream's formatting parameters we are holding; initially none.
    streambase_t* bound_stream = nullptr;

    /// A bufferless \b ios object into which the stream's formatting parameters are saved; initially empty.
    streambase_t saved_format{ nullptr };
};

/*------------------------------------------*\
|*  Stream extraction/insertion operators:  *|
\*------------------------------------------*/

/// Stream extraction-operator to handle savefmt-instances appearing in \b operator>> chains.
template< typename CharT, typename Traits >
std::basic_istream< CharT, Traits >&
operator>>( std::basic_istream< CharT, Traits >& stream,
                 basic_savefmt< CharT, Traits >&& saver );

/// Stream insertion-operator to handle savefmt-instances appearing in \b operator<< chains.
template< typename CharT, typename Traits >
std::basic_ostream< CharT, Traits >&
operator<<( std::basic_ostream< CharT, Traits >& stream,
                 basic_savefmt< CharT, Traits >&& saver );

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
awo::basic_savefmt< CharT, Traits >::basic_savefmt( streambase_t& stream )
: bound_stream{ &stream }
{
    // We've now bound this instance to the given stream (initialization, above).

    // Capture the stream's current formatting parameters for later restoration.
    saved_format.copyfmt( stream );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::basic_savefmt( basic_savefmt&& other )
: bound_stream{ std::exchange( other.bound_stream, nullptr ) }
{
    // We've bound this instance to the stream previously bound-to by the
    // other instance and unbound that other instance from the stream (above).

    // Copy the formatting parameters that were saved in the other instance.
    saved_format.copyfmt( other.saved_format );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
auto awo::basic_savefmt< CharT, Traits >::operator=( basic_savefmt&& other ) -> basic_savefmt&
{
    if ( &other != this )
    {
        // Bind to the other instance's stream and unbind that stream from the other instance.
        bound_stream = std::exchange( other.bound_stream, nullptr );

        // Copy the formatting parameters that were saved in the other instance.
        saved_format.copyfmt( other.saved_format );
    }

    return *this;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
void awo::basic_savefmt< CharT, Traits >::capture( streambase_t& stream )
{
    // If we are currently active, restore the saved parameters to the stream.
    restore();

    // Now bind to the new stream.
    bound_stream = &stream;

    // And capture its current formatting parameters for later restoration.
    saved_format.copyfmt( stream );
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
void awo::basic_savefmt< CharT, Traits >::restore( bool const also_release )
{
    // Ignore inactive instances - no stream to restore to
    if ( bound_stream != nullptr )
    {
        // Restore the saved formatting parameters back to the stream.
        bound_stream->copyfmt( saved_format );

        if ( also_release )
        {
            bound_stream = nullptr;
        }
    }
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
void awo::basic_savefmt< CharT, Traits >::release()
{
    // Unbind from the stream, so the saved parameters cannot be restored.
    bound_stream = nullptr;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
auto awo::basic_savefmt< CharT, Traits >::stream() const -> streambase_t*
{
    // Return a pointer to the stream to which we are bound (or nullptr).
    // This is useful for an "is active" test and for more esoteric tracking.
    return bound_stream;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
awo::basic_savefmt< CharT, Traits >::~basic_savefmt()
{
    // Restore any saved formatting parameters to their stream (if any).
    restore();
}

//============================================================================

template< typename CharT, typename Traits >
std::basic_istream< CharT, Traits >&
awo::operator>>( std::basic_istream< CharT, Traits >& stream,
                 awo::basic_savefmt< CharT, Traits >&& saver )
{
    // Note: the saver object will expire at the end of the enclosing expression
    // and will then, therefore, restore the saved parameters back to the stream.

    // Capture the stream's formatting parameters.
    saver.capture( stream );

    // Usual practice - return the stream reference for further \b operator>> chaining.
    return stream;
}

//----------------------------------------------------------------------------

template< typename CharT, typename Traits >
std::basic_ostream< CharT, Traits >&
awo::operator<<( std::basic_ostream< CharT, Traits >& stream,
                 awo::basic_savefmt< CharT, Traits >&& saver )
{
    // Note: the saver object will expire at the end of the enclosing expression
    // and will then, therefore, restore the saved parameters back to the stream.

    // Capture the stream's formatting parameters.
    saver.capture( stream );

    // Usual practice - return the stream reference for further \b operator<< chaining.
    return stream;
}

//============================================================================

#endif // INCLUDED_AWO_SAVEFMT_HPP
