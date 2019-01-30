# Porgy
List of dependencies
====================
C++11 enabled compiler (default since GCC 6.1)
Tulip 5.3-dev (https://github.com/Tulip-Dev/tulip)
Boost 1.60 (see http://www.boost.org/)

Basic compilation instruction
=============================
Porgy uses CMake as its build system.
The simplest way to compile this package is:
1. Download the current Tulip github code base.

2. clone the Porgy project in the *externalplugins* directory of the Tulip source tree.

3.  Go to the top level directory of the tulip soures, and create a `build` directory.
    Go to this `build` folder, and type `cmake ..` (or `cmake-gui` if you prefer the visual interface of CMake) .
    This will check for all of Tulip and Porgy's dependencies, and warn if some are missing. If dependencies are correctly installed
    Porgy will find it automatically.
    If you want to compile in debug (resp. Release) mode set the CMAKE_BUILD_TYPE variable to "Debug" (resp. Release). See the CMake manual for more information.

2. Type `make` to compile. (if you have more than one core, use `-jX`
   where X is your number of cores + 1).

3. To install use `make install`. Porgy will be installed in the Tulip
   plugins directory found during the configuration process (1). You must have
   write access to the Tulip plugins directory.

4. You can remove the program binaries and object files from the source code directory by typing `make clean'.

5. To execute Porgy just launch tulip with the `tulip` command and choose the
Porgy perspective or use from a command line `tulip_perspective -p Porgy`.
