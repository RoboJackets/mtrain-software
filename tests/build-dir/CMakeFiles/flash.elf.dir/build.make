# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/apachetest/Documents/mtrain/tests

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/apachetest/Documents/mtrain/tests/build-dir

# Include any dependencies generated for this target.
include CMakeFiles/flash.elf.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/flash.elf.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/flash.elf.dir/flags.make

CMakeFiles/flash.elf.dir/flash.c.o: CMakeFiles/flash.elf.dir/flags.make
CMakeFiles/flash.elf.dir/flash.c.o: ../flash.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/apachetest/Documents/mtrain/tests/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/flash.elf.dir/flash.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/flash.elf.dir/flash.c.o   -c /home/apachetest/Documents/mtrain/tests/flash.c

CMakeFiles/flash.elf.dir/flash.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/flash.elf.dir/flash.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/apachetest/Documents/mtrain/tests/flash.c > CMakeFiles/flash.elf.dir/flash.c.i

CMakeFiles/flash.elf.dir/flash.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/flash.elf.dir/flash.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/apachetest/Documents/mtrain/tests/flash.c -o CMakeFiles/flash.elf.dir/flash.c.s

CMakeFiles/flash.elf.dir/flash.c.o.requires:

.PHONY : CMakeFiles/flash.elf.dir/flash.c.o.requires

CMakeFiles/flash.elf.dir/flash.c.o.provides: CMakeFiles/flash.elf.dir/flash.c.o.requires
	$(MAKE) -f CMakeFiles/flash.elf.dir/build.make CMakeFiles/flash.elf.dir/flash.c.o.provides.build
.PHONY : CMakeFiles/flash.elf.dir/flash.c.o.provides

CMakeFiles/flash.elf.dir/flash.c.o.provides.build: CMakeFiles/flash.elf.dir/flash.c.o


# Object files for target flash.elf
flash_elf_OBJECTS = \
"CMakeFiles/flash.elf.dir/flash.c.o"

# External object files for target flash.elf
flash_elf_EXTERNAL_OBJECTS =

flash.elf: CMakeFiles/flash.elf.dir/flash.c.o
flash.elf: CMakeFiles/flash.elf.dir/build.make
flash.elf: CMakeFiles/flash.elf.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/apachetest/Documents/mtrain/tests/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable flash.elf"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/flash.elf.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/flash.elf.dir/build: flash.elf

.PHONY : CMakeFiles/flash.elf.dir/build

CMakeFiles/flash.elf.dir/requires: CMakeFiles/flash.elf.dir/flash.c.o.requires

.PHONY : CMakeFiles/flash.elf.dir/requires

CMakeFiles/flash.elf.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/flash.elf.dir/cmake_clean.cmake
.PHONY : CMakeFiles/flash.elf.dir/clean

CMakeFiles/flash.elf.dir/depend:
	cd /home/apachetest/Documents/mtrain/tests/build-dir && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/apachetest/Documents/mtrain/tests /home/apachetest/Documents/mtrain/tests /home/apachetest/Documents/mtrain/tests/build-dir /home/apachetest/Documents/mtrain/tests/build-dir /home/apachetest/Documents/mtrain/tests/build-dir/CMakeFiles/flash.elf.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/flash.elf.dir/depend
