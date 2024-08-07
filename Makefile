HARNESS			:= test-savefmt
LIBHEADER		:= include/awo/savefmt.hpp
MAKEFILE		:= Makefile
DOXYFILE		:= Doxyfile
DOXYROOT		:= html/index.html

OBJECTS			:= $(HARNESS).o
DEPENDS			:= $(wildcard $(OBJECTS:.o=.d))

#--------------------------------------------------------------------------------------------
# Construct the necessary compilation/linkage CPPFLAGS, CFLAGS, CXXFLAGS, LDFLAGS and LDLIBS
#--------------------------------------------------------------------------------------------

# Required C++ standards compatibilty
CXXFLAGS		+= -std=c++14 -pedantic

# Warnings control
CXXFLAGS		+= -Wall -Wextra

# Optimisation/debugging control
CXXFLAGS		+= -O3 -g3

# Locate the header file(s)
CPPFLAGS		+= -I include

# Generate source dependencies when compiling...
CPPFLAGS 		+= -MMD -MP

# Use C++ mode when linking...
LINK.o			:= $(LINK.o:$(CC)=$(CXX))

#------------------------------------------------------------
# Build Targets
#------------------------------------------------------------

.PHONY:			all clean

all:			$(HARNESS) $(DOXYROOT)
clean:;			@rm -rvf $(HARNESS) *.[do] html
$(HARNESS):		$(OBJECTS)
$(OBJECTS):		$(MAKEFILE)
$(DOXYROOT):	$(LIBHEADER) $(DOXYFILE) $(MAKEFILE)
				doxygen

-include 		$(DEPENDS)
