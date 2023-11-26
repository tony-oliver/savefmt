PROGRAM1		:= $(notdir $(CURDIR))
#PROGRAM2		:= program2

ifneq			($(PROGRAM1),)
OBJECTS1		:= $(PROGRAM1).o
#OBJECTS1		+= something-more.o
ALL_OBJECTS		+= $(OBJECTS1)
ALL_PROGRAMS	+= $(PROGRAM1)
endif

ifneq			($(PROGRAM2),)
OBJECTS2		:= $(PROGRAM2).o
#OBJECTS2		+= something-else.o
ALL_OBJECTS		+= $(OBJECTS2)
ALL_PROGRAMS	+= $(PROGRAM2)
endif

DEPENDS			:= $(wildcard $(ALL_OBJECTS:.o=.d))

#--------------------------------------------------------------------------------------------
# Pachage/library dependencies
#--------------------------------------------------------------------------------------------

# List pkg-config package names here
PACKAGES		:= 

# Add attributes of manually-configured packages here
CPPFLAGS		+=
LDFLAGS			+=
LDLIBS			+=

#--------------------------------------------------------------------------------------------
# Pick up and normalise external switches
#--------------------------------------------------------------------------------------------

SHELL			!= which bash
AFFIRMATIVES	:= 1 y ok yes true
BF_TRACE		:= $(filter $(shell tr [:upper:] [:lower:] <<<'$(TRACE)' ), $(AFFIRMATIVES))

#--------------------------------------------------------------------------------------------
# Boilerplate code to discover the most-recent compilers and their latest supported standards
#--------------------------------------------------------------------------------------------

FIND_LATEST		:= compgen -c COMPILER | grep -E '^[^-]+(-[0-9]+)?$$' | sort -rut- -gk2 | head -1
FIND_STANDARD	:= awk '/^LANGUAGE[0-9][0-9]$$/{printf"%02d\t%s\n",(substr($$1,length($$1)-1)+20)%100,$$1}' | sort -r | cut -f2 | head -1
GNU_FINDSTD_CMD	:= strings COMPILER_PATH | sed -n 's/^-std=//p'                          | $(FIND_STANDARD)
CL_FINDSTD_CMD	:= COMPILER_PATH -std= -xLANGUAGE -<<<'int main(){}' 2>&1 | cut -d\' -f2 | $(FIND_STANDARD)

DEFAULT_CC		:= $(CC)
DEFAULT_CXX		:= $(CXX)

ifeq ($(findstring clang,$(CC)),)
CC	 			!= $(FIND_LATEST:COMPILER=gcc)
CC_PATH			!= which $(CC)
C_STANDARD		!= $(subst LANGUAGE,c,$(subst COMPILER_PATH,$(CC_PATH),$(GNU_FINDSTD_CMD)))
else
CC	 			!= $(FIND_LATEST:COMPILER=clang)
CC_PATH			!= which $(CC)
C_STANDARD		!= $(subst LANGUAGE,c,$(subst COMPILER_PATH,$(CC_PATH),$(CL_FINDSTD_CMD)))
endif

ifeq ($(findstring clang,$(CXX)),)
CXX 			!= $(FIND_LATEST:COMPILER=g++)
CXX_PATH		!= which $(CXX)
CXX_STANDARD	!= $(subst LANGUAGE,c\+\+,$(subst COMPILER_PATH,$(CXX_PATH),$(GNU_FINDSTD_CMD)))
else
CXX 			!= $(FIND_LATEST:COMPILER=clang++)
CXX_PATH		!= which $(CXX)
CXX_STANDARD	!= $(subst LANGUAGE,c\+\+,$(subst COMPILER_PATH,$(CXX_PATH),$(CL_FINDSTD_CMD)))
endif

# For the 'debug' target...
CINFO			:= DEFAULT_CC="${DEFAULT_CC}", CC="${CC}", CC_PATH="${CC_PATH}", C_STANDARD="${C_STANDARD}"
CXXINFO			:= DEFAULT_CXX="${DEFAULT_CXX}", CXX="${CXX}", CXX_PATH="${CXX_PATH}", CXX_STANDARD="${CXX_STANDARD}"

#--------------------------------------------------------------------------------------------
# Construct the necessary compilation/linkage CPPFLAGS, CFLAGS, CXXFLAGS, LDFLAGS and LDLIBS
#--------------------------------------------------------------------------------------------

# Use flags required by pkg-config packages
ifneq			($(PACKAGES),)
CPPFLAGS 		+= $(shell pkg-config --cflags-only-I                 $(PACKAGES))
COMPILEFLAGS	+= $(shell pkg-config --cflags-only-other             $(PACKAGES))
LDFLAGS  		+= $(shell pkg-config --libs-only-L --libs-only-other $(PACKAGES))
LDLIBS   		+= $(shell pkg-config --libs-only-l                   $(PACKAGES))
endif

# Generate source dependencies while compiling...
CPPFLAGS 		+= -MMD -MP

# If we discovered a C standard capability, request conformity to it
ifneq ($(C_STANDARD),)
CFLAGS			+= -std=$(C_STANDARD) -pedantic
endif

# If we discovered a C++ standard capability, request conformity to it
ifneq ($(CXX_STANDARD),)
CXXFLAGS		+= -std=$(CXX_STANDARD) -pedantic
endif

# Warnings control
COMPILEFLAGS	+= -Wall
COMPILEFLAGS	+= -Wextra

# Optimisation/debugging control
COMPILEFLAGS	+= -O3
COMPILEFLAGS	+= -g3

CFLAGS			+= $(COMPILEFLAGS)
CXXFLAGS		+= $(COMPILEFLAGS)

# Use C++ mode when linking...
LINK.o			:= $(LINK.o:$(CC)=$(CXX))

#------------------------------------------------------------
# Build Targets
#------------------------------------------------------------

.PHONY:			all clean debug

all:			$(ALL_PROGRAMS)

ifneq			($(PROGRAM1),)
$(PROGRAM1):	$(OBJECTS1)
endif

ifneq			($(PROGRAM2),)
$(PROGRAM2):	$(OBJECTS2)
endif

$(ALL_OBJECTS):	Makefile

clean:;			@rm -vf $(ALL_PROGRAMS) $(wildcard *.[do])

debug:;			@echo $(CINFO)
				@echo $(CXXINFO)

ifneq 			($(DEPENDS),)
-include 		$(DEPENDS)
endif
