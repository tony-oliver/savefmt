# savefmt
Easily save C++ stream formatting parameters before you change them; automatically restore them later.

## Caveats

This markdown file has certain faults w.r.t. local and/or external anchor points.  These are expected to be fixed in due course.

## Introduction

The **```savefmt```** software component provides a class template, confined to a single header file.  All defined templates, classes and function are contained within the **```awo```** namespace (wherein can be found all components created by this author).

The template (**```basic_savefmt```**) must be specialised on a character type and, optionally, a character-traits class (in the same manner as **```std::basic_string```**, **```std::basic_ios```**, *etc*.) in order to generate concrete classes from which objects may be instantiated.

Typically, the only specialisations expected are on **```char```** and **```wchar_t```**, with no custom traits type - when omitted, the traits type will be **```std::char_traits<CharT>```** (where **```CharT```** is the character type on which the template is specialised).

For convenience, two typedefs are provided for these common specialisations:

* **```savefmt```** = **```basic_savefmt```** specialised on **```char;```**
* **```wsavefmt```** = **```basic_savefmt```** specialised on **```wchar_t```**.

Stream-based insertion and extraction operators are also provided for the classes generated from the class template.  [See below for usage of these](#user-content-expression-based-raii).

Doxygen documentation is generated on building the test program contained in this project and [can be found here](html/index.html).

## Requirements

The header file **```awo/savefmt.hpp```** must be included in the translation unit requiring its services.  The header should be installed in a folder named **```awo```** either (a) under the same location as the calling code's source file(s) or (b) in one of the folders that can be found on the header-include search path(s), *i.e.*
```
#include "awo/savefmt.hpp"
```
or
```
#include <awo/savefmt.hpp>
```
respectively.

The C++ compiler for the project must support (at least) the C++14 standard, due to the use of rvalue-references (and, thus, *move semantics*) and the function template **```std::exchange()```**.

## Examples

### Scope-based RAII

Suppose you have a debug-logging function that modifies the output-stream's formatting properties in one or more ways, in order to insert its data into the logging stream appropriately.  Furthermore, you might only call such a function conditionally.  In that case, you may not know whether it has been called (and thus changed the formatting parameters) or not (so the stream's parameters are unchanged). Many surprises have been encountered (by this author, at least) because of the uncertainty imposed by the conditionality of calling the function.  Wouldn't it be nice of the function could ensure that the stream's formatting properties were, on return, exactly the same as on entry?

Well, here is such a function that does *exactly that:*
```
#include <awo/savefmt>

void report( std::ostream& out, unsigned const value )
{
	awo::savefmt const saver( out );

	out << std::dec;
	out << "size (decimal) = " << value << std::endl;;

	out << std::hex << std::uppercase << std::setfill( '0' );
	out << "size (hex) = 0x" << std::setw( 2 * sizeof size ) << value << std::endl;
}
```
The output-stream gets manipulated in a number of ways during the execuation of this function, which would leave it in a state quite different from that which it had before the function was called.  However, the presence of the **```aw::savefmt```** object, established before any such manipulation, guarantees that the stream will contain exactly the same formatting properties as it did when that object was created.  (The **```awo::savefmt```** object "**```saver```**" saves the stream's formatting properties on execution of its constructor and restores them back to the same stream on execution of its destructor).

### Manual Manipulation

Sometimes we encounter external library functions that might (naughtily) leave the stream holding changed (known or unpredictable) formatting parameters, requiring the caller to reinstate its desired parameters afterwards in order to continue in a predictable manner.
```
#include <awo/savefmt>

void do_my_work()
{
	// ... lots of code ...

	awo::savefmt saver;

	// it is assumed, here, that std::cout still has its default settings (e.g. std::dec)

	std::cout << "Forty-two = " << 42 << std::endl;

	saver.capture( std::cout );
	call_naughty_external_function();
	saver.restore();

	std::cout << "Forty-two = " << 42 << std::endl;

	call_another_naughty_external_function();
	saver.restore();

	std::cout << "Forty-two = " << 42 << std::endl;

	saver.release();

	// ... lots more code ...
}
```
Here, we introduce an **```awo::saver```** object (which captures the current state of stream **```std::cout```**) and call its **```restore()```** member function each time we fear that the stream's state has been altered without our express permission.  Each such **```restore()```** will copy the originally-captured formatting parameters back to the stream.

We have also called **```saver.release()```** here, which will prevent the **```saver```** object from restoring the original parameters once again, when the **```saver```** object goes out of scope and gets destroyed (this **```release()```** operation is an optional, but often useful, step).

### Expression-based RAII

This is the idiom for which this component was originally created.  Here's some example usage:
```
#include <awo/savefmt>

void do_my_work()
{
	// ... lots of code ...

	constexpr unsigned value = 42;

	// it is assumed, here, that std::cout still has its default settings (e.g. std::dec)

	std::cout
		<< "default: "
		<< std:;setw( 10 ) << value
		<< std::endl;

	std::cout
		<< awo::savefmt{}
		<< std::hex << std::uppercase << std::setfill( '0' )
		<< "hex: 0x"
		<< std::setw( 2 * sizeof value ) << value
		<< std::endl;

	std::cout
		<< "default: "
		<< std:;setw( 10 ) << value
		<< std::endl;

	// ... lots more code ...
}
```
Here, we only introduce the **```awo::savefmt```** object in the stream-insertion expression itself, where it captures the stream's formatting parameters.  This temporary object is guaranteed to remain in existence until the enclosing-expression is competely evaluated.  At that time, the temporary is destroyed, restoring the captured parameters back to the stream from whence they came.
