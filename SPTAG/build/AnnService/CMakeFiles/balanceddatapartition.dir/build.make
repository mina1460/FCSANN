# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/cse-p07-g07f/spTAG/SPTAG

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cse-p07-g07f/spTAG/SPTAG/build

# Include any dependencies generated for this target.
include AnnService/CMakeFiles/balanceddatapartition.dir/depend.make

# Include the progress variables for this target.
include AnnService/CMakeFiles/balanceddatapartition.dir/progress.make

# Include the compile flags for this target's objects.
include AnnService/CMakeFiles/balanceddatapartition.dir/flags.make

AnnService/CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.o: AnnService/CMakeFiles/balanceddatapartition.dir/flags.make
AnnService/CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.o: ../AnnService/src/BalancedDataPartition/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cse-p07-g07f/spTAG/SPTAG/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object AnnService/CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.o"
	cd /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.o -c /home/cse-p07-g07f/spTAG/SPTAG/AnnService/src/BalancedDataPartition/main.cpp

AnnService/CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.i"
	cd /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cse-p07-g07f/spTAG/SPTAG/AnnService/src/BalancedDataPartition/main.cpp > CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.i

AnnService/CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.s"
	cd /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cse-p07-g07f/spTAG/SPTAG/AnnService/src/BalancedDataPartition/main.cpp -o CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.s

# Object files for target balanceddatapartition
balanceddatapartition_OBJECTS = \
"CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.o"

# External object files for target balanceddatapartition
balanceddatapartition_EXTERNAL_OBJECTS =

../Release/balanceddatapartition: AnnService/CMakeFiles/balanceddatapartition.dir/src/BalancedDataPartition/main.cpp.o
../Release/balanceddatapartition: AnnService/CMakeFiles/balanceddatapartition.dir/build.make
../Release/balanceddatapartition: ../Release/libSPTAGLibStatic.a
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_system.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_thread.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_serialization.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_wserialization.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_regex.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/openmpi/lib/libmpi_cxx.so
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/openmpi/lib/libmpi.so
../Release/balanceddatapartition: ../Release/libDistanceUtils.a
../Release/balanceddatapartition: ../Release/libzstd.a
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_atomic.so.1.71.0
../Release/balanceddatapartition: /usr/lib/x86_64-linux-gnu/libboost_serialization.so.1.71.0
../Release/balanceddatapartition: AnnService/CMakeFiles/balanceddatapartition.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cse-p07-g07f/spTAG/SPTAG/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../Release/balanceddatapartition"
	cd /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/balanceddatapartition.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
AnnService/CMakeFiles/balanceddatapartition.dir/build: ../Release/balanceddatapartition

.PHONY : AnnService/CMakeFiles/balanceddatapartition.dir/build

AnnService/CMakeFiles/balanceddatapartition.dir/clean:
	cd /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService && $(CMAKE_COMMAND) -P CMakeFiles/balanceddatapartition.dir/cmake_clean.cmake
.PHONY : AnnService/CMakeFiles/balanceddatapartition.dir/clean

AnnService/CMakeFiles/balanceddatapartition.dir/depend:
	cd /home/cse-p07-g07f/spTAG/SPTAG/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cse-p07-g07f/spTAG/SPTAG /home/cse-p07-g07f/spTAG/SPTAG/AnnService /home/cse-p07-g07f/spTAG/SPTAG/build /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService /home/cse-p07-g07f/spTAG/SPTAG/build/AnnService/CMakeFiles/balanceddatapartition.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : AnnService/CMakeFiles/balanceddatapartition.dir/depend
