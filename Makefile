#
# The MIT License (MIT)
#
# Copyright (c) <2015> Steffen Nüssle
# Copyright (c) <2016> Steffen Nüssle
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

CC 		:= /usr/bin/gcc
CXX		:= /usr/bin/g++

#
# Show / suppress compiler invocations. 
# Set 'SUPP :=' to show them.
# Set 'SUPP := @' to suppress compiler invocations.
#
SUPP		:=

#
# Set name of the binary
#
BIN		:= rf

ifndef BIN
$(error No binary name specified)
endif

BASH_COMPLETION := bash-completion/rf
BASH_COMPLETION_INSTALL_DIR := /usr/share/bash-completion/completions/
BASH_COMPLETION_UNINSTALL_TARGET := /usr/share/bash-completion/completions/rf

#
# Specify all source files. The paths should be relative to this file.
#
# SRC 		:= $(shell find ./ -iname "*.c")
SRC 		:= $(shell find src/ -iname "*.cpp")
# SRC 		:= $(shell find ./ -iname "*.c" -o -iname "*.cpp")
HDR		:= $(shell find src/ -iname "*.hpp")

ifndef SRC
$(error No source files specified)
endif


#
# Uncomment if 'VPATH' is needed. 'VPATH' is a list of directories in which
# make searches for source files.
#
EMPTY		:=
SPACE		:= $(EMPTY) $(EMPTY)
# VPATH 	:= $(subst $(SPACE),:,$(sort $(dir $(SRC))))

RELPATHS	:= $(filter ../%, $(SRC))
ifdef RELPATHS

NAMES		:= $(notdir $(RELPATHS))
UNIQUE		:= $(sort $(notdir $(RELPATHS)))

#
# Check for duplicate file names (not regarding directories)
#
ifneq ($(words $(NAMES)),$(words $(UNIQUE)))
DUPS		:= $(shell printf "$(NAMES)" | tr -s " " "\n" | sort | uniq -d)
DIRS		:= $(dir $(filter %$(DUPS), $(SRC)))
$(error [ $(DUPS) ] occur(s) in two or more relative paths [ $(DIRS) ] - not supported)
endif

#
# Only use file name as the source location and add the relative path to 'VPATH'
# This prevents object files to reside in paths like 'build/src/../relative/' or
# even worse 'build/src/../../relative' which would be a path outside of
# the specified build directory
#
SRC		:= $(filter-out ../%, $(SRC)) $(notdir $(RELPATHS))
VPATH		:= $(subst $(SPACE),:, $(dir $(RELPATHS)))
endif


#
# Paths for the build-, objects- and dependency-directories
#
BUILDDIR	:= build
TARGET 		:= $(BUILDDIR)/$(BIN)

#
# Set installation directory used in 'make install'
#
INSTALL_DIR	:= /usr/local/bin/

#
# Generate all object and dependency files from $(SRC) and get
# a list of all inhabited directories. 'AUX' is used to prevent file paths
# like build/objs/./srcdir/
#
AUX		:= $(patsubst ./%, %, $(SRC))
C_SRC		:= $(filter %.c, $(AUX))
CXX_SRC		:= $(filter %.cpp, $(AUX))
C_OBJS		:= $(addprefix $(BUILDDIR)/, $(patsubst %.c, %.o, $(C_SRC)))
CXX_OBJS	:= $(addprefix $(BUILDDIR)/, $(patsubst %.cpp, %.o, $(CXX_SRC)))
OBJS		:= $(C_OBJS) $(CXX_OBJS)
DEPS		:= $(patsubst %.o, %.d, $(OBJS))
DIRS		:= $(BUILDDIR) $(sort $(dir $(OBJS)))

#
# Add additional include paths
#
INCLUDE		:= \
		-Isrc/							\

#
# Add used libraries which are configurable with pkg-config
#
PKGCONF		:= \


#
# Set non-pkg-configurable libraries flags 
#
LIBS		:= \
		-pthread \
		-lclang-cpp \
		$(shell llvm-config --libs) \
		$(shell llvm-config --system-libs) \


#
# Set linker flags, here: 'rpath' for libraries in non-standard directories
# If '-shared' is specified: '-fpic' or '-fPIC' should be set here 
# as in the CFLAGS / CXXFLAGS
#
LDFLAGS		:= \
		$(shell llvm-config --ldflags)

LDLIBS		:= $(LIBS)


CPPFLAGS	= \
		$(INCLUDE)						\
		-MMD							\
		-MF $(patsubst %.o,%.d,$@) 				\
		-MT $@ 							\

#
# Set compiler flags that you want to be present for every make invocation.
# Specific flags for release and debug builds can be added later on
# with target-specific variable values.
#
CFLAGS  	:= \


CXXFLAGS	:= \
		-fno-rtti						\
		$(shell llvm-config --cxxflags)				\

#
# Check if specified pkg-config libraries are available and abort
# if they are not.
#
ifdef PKGCONF

OK		:= $(shell pkg-config --exists $(PKGCONF) && printf "OK")
ifndef $(OK)
PKGS 		:= $(shell pkg-config --list-all | cut -f1 -d " ")
FOUND		:= $(sort $(filter $(PKGCONF),$(PKGS)))
$(error Missing pkg-config libraries: [ $(filter-out $(FOUND),$(PKGCONF)) ])
endif

CFLAGS		+= $(shell pkg-config --cflags $(PKGCONF))
CXXFLAGS	+= $(shell pkg-config --cflags $(PKGCONF))
LDLIBS		+= $(shell pkg-config --libs $(PKGCONF))

endif


#
# Setting some terminal colors
#
RED		:= \e[0;31m
GREEN		:= \e[0;32m
BROWN		:= \e[0;33m
BLUE		:= \e[0;34m
MAGENTA		:= \e[0;35m
CYAN		:= \e[0;36m
BOLD_RED	:= \e[1;31m
BOLD_GREEN	:= \e[1;32m
BOLD_YELLOW  	:= \e[1;33m
BOLD_BLUE	:= \e[1;34m
BOLD_MAGENTA	:= \e[1;35m
BOLD_CYAN	:= \e[1;36m
DEFAULT_COLOR 	:= \e[0m

COLOR_COMPILING	:= $(BOLD_BLUE)
COLOR_LINKING	:= $(BOLD_YELLOW)
COLOR_FINISHED	:= $(BOLD_GREEN)

print 		= @printf "$(1)$(2)$(DEFAULT_COLOR)\n"
md5sum 		= $$(md5sum $(1) | cut -f1 -d " ")


all: release

#
# Note that if "-flto" is specified you may want to pass the optimization
# flags used for compiling to the linker (as done below).
#
# Also, if you want to use:
#	-ffunction-sections
#	-fdata-sections
#
# and the linker option
#	-Wl,--gc-sections
#
# This would be the sane place to do so as it may interfere with debugging.
#

# release: CPPFLAGS	+= -DNDEBUG
# release: CFLAGS 	+= -flto
release: CXXFLAGS 	+= -flto
release: LDFLAGS 	+= -O3 -flto -Wl,--gc-sections
release: $(TARGET) compile-commands

# debug: CFLAGS		+= -Og -g2
debug: CXXFLAGS 	+= -O0 -g2
# debug: LDFLAGS		+=
debug: $(TARGET) compile-commands

SANITIZERS	:= \
		with-addr-sanitizer 				\
		with-mem-sanitizer 				\
		with-thread-sanitizer				\
		with-ub-sanitizer

$(SANITIZERS): CXXFLAGS 	+= -O1 -g2 -fno-omit-frame-pointer

with-addr-sanitizer: CXXFLAGS 	+= -fsanitize=address
with-addr-sanitizer: LDFLAGS  	+= -fsanitize=address

# This option is only available on clang -- (clang 3.9.1 / gcc 6.3.1)
with-mem-sanitizer: CXX		:= /usr/bin/clang++
with-mem-sanitizer: CXXFLAGS 	+= -fsanitize=memory
with-mem-sanitizer: LDFLAGS  	+= -fsanitize=memory

with-thread-sanitizer: CXXFLAGS += -fsanitize=thread
with-thread-sanitizer: LDFLAGS	+= -fsanitize=thread

with-ub-sanitizer: CXXFLAGS	+= -fsanitize=undefined
with-ub-sanitizer: LDFLAGS	+= -fsanitize=undefined

$(SANITIZERS): $(TARGET) compile-commands

# syntax-check: CFLAGS 	+= -fsyntax-only
syntax-check: CXXFLAGS 	+= -fsyntax-only
syntax-check: $(OBJS)

$(TARGET): $(OBJS)
	$(call print,$(COLOR_LINKING),Linking [ $@ ])
# 	$(SUPP)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	$(SUPP)$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	$(call print,$(COLOR_FINISHED),Built target [ $@ ]: $(call md5sum,$@))
	

-include $(DEPS)

# $(BUILDDIR)/%.o: %.c
# 	$(call print,$(COLOR_COMPILING),Building: $@)
# 	$(SUPP)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<
	
$(BUILDDIR)/%.o: %.cpp
	$(call print,$(COLOR_COMPILING),Building: $@)
	$(SUPP)$(CXX) -c -o $@ $(CPPFLAGS) $(CXXFLAGS) $<

$(OBJS): | $(DIRS)

$(DIRS):
	mkdir -p $(DIRS)

#
# A part of $(CPPFLAGS) is dependent on the target ("$@") and therefore
# not valid within this rule. mk-jcdb.py will clean up those flags and
# if applicable their arguments.
#
compile-commands: $(SRC)
	@python utils/make-jcdb.py					\
		--command "$(CXX) -c $(CPPFLAGS) $(CXXFLAGS)"		\
		--add-clang-include					\
		--add Wno-unknown-warning-option			\
		--discard MMD "MF $(patsubst %.o,%.d,$@)" "MT $@"	\
		-- $(SRC) > compile_commands.json

clean:
	rm -rf $(TARGET) $(DIRS) compile_commands.json

format:
	clang-format -i $(HDR) $(SRC)

install: $(TARGET) $(BASH_COMPLETION)
	cp $(TARGET) $(INSTALL_DIR)
	cp $(BASH_COMPLETION) $(BASH_COMPLETION_INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)$(BIN) $(BASH_COMPLETION_UNINSTALL_TARGET)

.PHONY: all	 							\
	clean 								\
	compile-commands 						\
	debug 								\
	install 							\
	release 							\
	syntax-check 							\
	uninstall

.SILENT: clean format $(DIRS)
