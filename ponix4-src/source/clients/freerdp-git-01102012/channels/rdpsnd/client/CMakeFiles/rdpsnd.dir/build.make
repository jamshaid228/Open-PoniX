# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /f

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /f

# Include any dependencies generated for this target.
include channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/depend.make

# Include the progress variables for this target.
include channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/progress.make

# Include the compile flags for this target's objects.
include channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/flags.make

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/flags.make
channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o: channels/rdpsnd/client/rdpsnd_main.c
	$(CMAKE_COMMAND) -E cmake_progress_report /f/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o"
	cd /f/channels/rdpsnd/client && /toolchain/bin/i486-TSOL-linux-gnu-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o   -c /f/channels/rdpsnd/client/rdpsnd_main.c

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rdpsnd.dir/rdpsnd_main.c.i"
	cd /f/channels/rdpsnd/client && /toolchain/bin/i486-TSOL-linux-gnu-gcc  $(C_DEFINES) $(C_FLAGS) -E /f/channels/rdpsnd/client/rdpsnd_main.c > CMakeFiles/rdpsnd.dir/rdpsnd_main.c.i

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rdpsnd.dir/rdpsnd_main.c.s"
	cd /f/channels/rdpsnd/client && /toolchain/bin/i486-TSOL-linux-gnu-gcc  $(C_DEFINES) $(C_FLAGS) -S /f/channels/rdpsnd/client/rdpsnd_main.c -o CMakeFiles/rdpsnd.dir/rdpsnd_main.c.s

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.requires:
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.requires

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.provides: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.requires
	$(MAKE) -f channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/build.make channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.provides.build
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.provides

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.provides.build: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o

# Object files for target rdpsnd
rdpsnd_OBJECTS = \
"CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o"

# External object files for target rdpsnd
rdpsnd_EXTERNAL_OBJECTS =

channels/rdpsnd/client/rdpsnd.so: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o
channels/rdpsnd/client/rdpsnd.so: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/build.make
channels/rdpsnd/client/rdpsnd.so: libfreerdp/utils/libfreerdp-utils.so.1.1.0
channels/rdpsnd/client/rdpsnd.so: winpr/libwinpr/crt/libwinpr-crt.so.0.1.0
channels/rdpsnd/client/rdpsnd.so: winpr/libwinpr/synch/libwinpr-synch.so.0.1.0
channels/rdpsnd/client/rdpsnd.so: winpr/libwinpr/handle/libwinpr-handle.so.0.1.0
channels/rdpsnd/client/rdpsnd.so: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library rdpsnd.so"
	cd /f/channels/rdpsnd/client && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rdpsnd.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/build: channels/rdpsnd/client/rdpsnd.so
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/build

# Object files for target rdpsnd
rdpsnd_OBJECTS = \
"CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o"

# External object files for target rdpsnd
rdpsnd_EXTERNAL_OBJECTS =

channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o
channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/build.make
channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: libfreerdp/utils/libfreerdp-utils.so.1.1.0
channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: winpr/libwinpr/crt/libwinpr-crt.so.0.1.0
channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: winpr/libwinpr/synch/libwinpr-synch.so.0.1.0
channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: winpr/libwinpr/handle/libwinpr-handle.so.0.1.0
channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/relink.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library CMakeFiles/CMakeRelink.dir/rdpsnd.so"
	cd /f/channels/rdpsnd/client && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rdpsnd.dir/relink.txt --verbose=$(VERBOSE)

# Rule to relink during preinstall.
channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/preinstall: channels/rdpsnd/client/CMakeFiles/CMakeRelink.dir/rdpsnd.so
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/preinstall

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/requires: channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/rdpsnd_main.c.o.requires
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/requires

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/clean:
	cd /f/channels/rdpsnd/client && $(CMAKE_COMMAND) -P CMakeFiles/rdpsnd.dir/cmake_clean.cmake
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/clean

channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/depend:
	cd /f && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /f /f/channels/rdpsnd/client /f /f/channels/rdpsnd/client /f/channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : channels/rdpsnd/client/CMakeFiles/rdpsnd.dir/depend

