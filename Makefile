#
# The MIT License (MIT)
#
# Copyright (c) <2015> Steffen Nüssle
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

SHELL_COMPL	:= ./bash-completion/rf
SHELL_COMPL_DIR := /usr/share/bash-completion/completions/


#
# Specify all source files. The paths should be relative to this file.
#
SRC 		:= $(shell find ./src/ -iname "*.cpp")

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
AUX		:= $(patsubst ./%,%,$(SRC))
OBJS		:= $(addprefix $(BUILDDIR)/, $(patsubst %.cpp, %.o, $(AUX)))
DEPS		:= $(patsubst %.o, %.d, $(OBJS))
DIRS		:= $(BUILDDIR) $(sort $(dir $(OBJS)))

#
# Add additional preprocessor definitions
#
DEFS		:= \
#		-D_GNU_SOURCE						\
#		-DNDEBUG						\

#
# Add additional include paths
#
INCLUDE		:= \
 		-I./src							\

#
# Add used libraries which are configurable with pkg-config
#
PKGCONF		:= \
# 		gstreamer-1.0						\
# 		gstreamer-pbutils-1.0					\
# 		libcurl							\
# 		libxml-2.0						\

#
# Set non-pkg-configurable libraries flags 
#
LIBS		:= \
		-Wl,--start-group					\
		-lclangAnalysis 					\
		-lclangAST 						\
		-lclangASTMatchers					\
		-lclangBasic						\
		-lclangDriver 						\
		-lclangEdit 						\
		-lclangFormat						\
		-lclangFrontend 					\
		-lclangFrontendTool 					\
		-lclangLex 						\
		-lclangParse 						\
		-lclangRewrite						\
		-lclangRewriteFrontend 					\
		-lclangSema 						\
		-lclangSerialization 					\
		-lclangTooling						\
		-lclangToolingCore					\
		-Wl,--end-group						\
		$(shell llvm-config --libs)				\
		$(shell llvm-config --system-libs)			\
# 		-lclangARCMigrate 					\
# 		-lclangStaticAnalyzerCheckers				\
# 		-lclangStaticAnalyzerFrontend 				\
# 		-lclangStaticAnalyzerCore 				\
# 		-lclangCodeGen						\
# 		-lclangDynamicASTMatchers				\

#
# Set linker flags, here: 'rpath' for libraries in non-standard directories
#
LDFLAGS		:= \
		$(shell llvm-config --ldflags)

LDLIBS		:= $(LIBS)


CPPFLAGS	= \
		$(DEFS)							\
		$(INCLUDE)						\
		-MMD							\
		-MF $(patsubst %.o,%.d,$@) 				\
		-MT $@ 							\
		$(shell llvm-config --cppflags)

#
# Set additional compiler flags
#
CFLAGS  	:= \
		-std=c11 						\
		-Wall							\
		-Wextra 						\
		-Werror 						\
		-Wpedantic						\
# 		-O2 							\
# 		-g3							\
# 		-fno-omit-frame-pointer 				\
#		-fpic							\
#		-shared							\

CXXFLAGS	:= \
		-fno-rtti						\
		$(shell llvm-config --cxxflags)				\
# 		-std=c++14						\
# 		-Wall							\
# 		-Wextra 						\
# 		-Werror 						\
# 		-Wpedantic						\
# 		-Weffc++						\
# 		-O2 							\
# 		-g3							\
# 		-fno-omit-frame-pointer 				\
#		-fpic							\
#		-shared							\
		
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

all: $(TARGET) compile-commands

$(TARGET): $(OBJS)
	$(call print,$(COLOR_LINKING),Linking [ $@ ])
	$(SUPP)$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	$(call print,$(COLOR_FINISHED),Built target [ $@ ]: $(call md5sum,$@))
	

-include $(DEPS)

$(BUILDDIR)/%.o: %.cpp
	$(call print,$(COLOR_COMPILING),Building: $@)
	$(SUPP)$(CXX) -c -o $@ $(CPPFLAGS) $(CXXFLAGS) $<

$(OBJS): | $(DIRS)

$(DIRS):
	mkdir -p $(DIRS)

compile-commands: $(SRC)
	@python utils/make-jcdb.py 					\
	  -s $(SRC) 							\
	  -c "$(CXX) -c $(filter-out $@ -M%,$(CPPFLAGS)) $(CXXFLAGS)"	\
	  -d $$(pwd) 							\
	  --pretty > compile_commands.json

clean:
	rm -rf $(TARGET) $(DIRS) compile_commands.json

install: $(TARGET) $(SHELL_COMPL)
	cp $(TARGET) $(INSTALL_DIR)
	cp $(SHELL_COMPL) $(SHELL_COMPL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)$(BIN) $(SHELL_COMPL_DIR)$(notdir $(SHELL_COMPL))

.PHONY: all compile-commands clean install uninstall
.SILENT: clean $(DIRS)
