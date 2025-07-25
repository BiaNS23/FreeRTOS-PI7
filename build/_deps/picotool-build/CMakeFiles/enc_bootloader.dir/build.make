# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = "C:/Program Files/CMake/bin/cmake.exe"

# The command to remove a file.
RM = "C:/Program Files/CMake/bin/cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build

# Utility rule file for enc_bootloader.

# Include any custom commands dependencies for this target.
include CMakeFiles/enc_bootloader.dir/compiler_depend.make

CMakeFiles/enc_bootloader: CMakeFiles/enc_bootloader-complete

CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-install
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-mkdir
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-download
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-configure
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-build
CMakeFiles/enc_bootloader-complete: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-install
	"C:/Program Files/CMake/bin/cmake.exe" -E make_directory C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/CMakeFiles
	"C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/CMakeFiles/enc_bootloader-complete
	"C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-done

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-build: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-configure
	cd C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader && $(MAKE)

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-configure: enc_bootloader/tmp/enc_bootloader-cfgcmd.txt
enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-configure: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch
	cd C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader && "C:/Program Files/CMake/bin/cmake.exe" -DCMAKE_MAKE_PROGRAM:FILEPATH=C:/ProgramData/chocolatey/bin/make.exe -DPICO_SDK_PATH:FILEPATH=C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/pico-sdk -DUSE_PRECOMPILED:BOOL=true -DUSE_MBEDTLS=0 -DPICO_DEBUG_INFO_IN_RELEASE=OFF "-GUnix Makefiles" -S C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-src/enc_bootloader -B C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader
	cd C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader && "C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-configure

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-download: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-source_dirinfo.txt
enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-download: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-mkdir
	"C:/Program Files/CMake/bin/cmake.exe" -E echo_append
	"C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-download

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-install: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-build
	cd C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader && "C:/Program Files/CMake/bin/cmake.exe" -E echo_append

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-mkdir:
	"C:/Program Files/CMake/bin/cmake.exe" -Dcfgdir= -P C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/tmp/enc_bootloader-mkdirs.cmake
	"C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-mkdir

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch-info.txt
enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update
	"C:/Program Files/CMake/bin/cmake.exe" -E echo_append
	"C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch

enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update-info.txt
enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-download
	"C:/Program Files/CMake/bin/cmake.exe" -E echo_append
	"C:/Program Files/CMake/bin/cmake.exe" -E touch C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update

CMakeFiles/enc_bootloader.dir/codegen:
.PHONY : CMakeFiles/enc_bootloader.dir/codegen

enc_bootloader: CMakeFiles/enc_bootloader
enc_bootloader: CMakeFiles/enc_bootloader-complete
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-build
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-configure
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-download
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-install
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-mkdir
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-patch
enc_bootloader: enc_bootloader/src/enc_bootloader-stamp/enc_bootloader-update
enc_bootloader: CMakeFiles/enc_bootloader.dir/build.make
.PHONY : enc_bootloader

# Rule to build all files generated by this target.
CMakeFiles/enc_bootloader.dir/build: enc_bootloader
.PHONY : CMakeFiles/enc_bootloader.dir/build

CMakeFiles/enc_bootloader.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/enc_bootloader.dir/cmake_clean.cmake
.PHONY : CMakeFiles/enc_bootloader.dir/clean

CMakeFiles/enc_bootloader.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-src C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-src C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build C:/Users/nsilv/OneDrive/Documentos/USP/FreeRTOS/build/_deps/picotool-build/CMakeFiles/enc_bootloader.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/enc_bootloader.dir/depend

