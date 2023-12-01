# NOTE: this Makefile started life as an all-purpose makefile
# (which I copy around to get projects started easily).
# It has been heavily pruned to suit just this project.  

HARNESS			:= savefmt
LIBHEADER		:= awo/savefmt.hpp
MAKEFILE		:= Makefile
DOXYFILE		:= Doxyfile
DOXYROOT		:= html/index.html

OBJECTS			:= $(HARNESS).o
DEPENDS			:= $(wildcard $(OBJECTS:.o=.d))

#----------------------------------------------------------------------------------------
# Boilerplate code to discover the most-recent compiler and its latest supported standard
#----------------------------------------------------------------------------------------

SHELL			!= which bash
FIND_LATEST		:= compgen -c COMPILER | grep -E '^[^-]+(-[0-9]+)?$$' | sort -rut- -gk2 | head -1
FIND_STANDARD	:= awk '/^LANGUAGE[0-9][0-9]$$/{printf"%02d\t%s\n",(substr($$1,length($$1)-1)+20)%100,$$1}' | sort -r | cut -f2 | head -1
GNU_FINDSTD_CMD	:= strings COMPILER_PATH | sed -n 's/^-std=//p'                          | $(FIND_STANDARD)
CL_FINDSTD_CMD	:= COMPILER_PATH -std= -xLANGUAGE -<<<'int main(){}' 2>&1 | cut -d\' -f2 | $(FIND_STANDARD)

DEFAULT_CXX		:= $(CXX)

ifneq ($(findstring clang++,$(CXX)),)
CXX 			!= $(FIND_LATEST:COMPILER=clang++)
CXX_PATH		!= which $(CXX)
CXX_STANDARD	!= $(subst LANGUAGE,c\+\+,$(subst COMPILER_PATH,$(CXX_PATH),$(CL_FINDSTD_CMD)))
else
CXX 			!= $(FIND_LATEST:COMPILER=g++)
CXX_PATH		!= which $(CXX)
CXX_STANDARD	!= $(subst LANGUAGE,c\+\+,$(subst COMPILER_PATH,$(CXX_PATH),$(GNU_FINDSTD_CMD)))
endif

# For the 'debug' target...
CXXINFO			:= DEFAULT_CXX="${DEFAULT_CXX}", CXX="${CXX}", CXX_PATH="${CXX_PATH}", CXX_STANDARD="${CXX_STANDARD}"

#--------------------------------------------------------------------------------------------
# Construct the necessary compilation/linkage CPPFLAGS, CFLAGS, CXXFLAGS, LDFLAGS and LDLIBS
#--------------------------------------------------------------------------------------------

# If we discovered a C++ standards capability, request conformity to it
ifneq ($(CXX_STANDARD),)
CXXFLAGS		+= -std=$(CXX_STANDARD) -pedantic
endif

# Warnings control
CXXFLAGS		+= -Wall -Wextra

# Optimisation/debugging control
CXXFLAGS		+= -O3 -g3

# Generate source dependencies while compiling...
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

ifneq 			($(DEPENDS),)
-include 		$(DEPENDS)
endif
