# THIS PROJECT IS NO LONGER ACTIVELY MAINTAINED. IT IS NOT COMPATIBLE WITH TULIP RELEASE (V6.0.X). #
# Porgy

PORGY (http://porgy.labri.fr) is a visual and interactive platform for modelling, simulating and analysing complex systems based on graph rewriting.

PORGY offers the ability to act on the simulation of the rewriting calculus and thus offer the expert a unique mean of interacting with the systems they design and study, turning interactive visualisation of graph rewriting systems into a high-level visual programming environment.

This is a joint work with Maribel Fernandez (http://www.dcs.kcl.ac.uk/staff/maribel/) from King's College London and Hélène Kirchner (https://wiki.bordeaux.inria.fr/Helene-Kirchner/doku.php) from Inria. The first version of Porgy was developed thanks to the Inria associate team program (2009-2011) and the EVIDEN project (https://anr.fr/Projet-ANR-10-JCJC-0201).

Using as a starting point the Tulip system (https://tulip.labri.fr), PORGY allows us to:
	
 - create a graphical network model using a visual editor;
 - trigger a series of transformations on the network using graph rewriting rules to describe transformations;
 - display a sequence of network obtained by application of transformation steps, as well as the sequence of rules underlying these transformations;
 - design analysis and verification tools to check static and dynamic properties of the model.
 - and much more...
	

List of dependencies
====================
- Tulip >= 5.4 with Python support enabled (https://github.com/Tulip-Dev/tulip)
- Boost 1.60 (see http://www.boost.org/)
- All dependencies from Tulip especially: 
  - C++11 compiler
  - Qt >= 5.6
  - cmake >= 3.1

Compilation instructions
=========================
Porgy could be build along with Tulip or as a standalone project.

### Easy: compile Porgy along with Tulip:
1. Download the current Tulip github code base.

2. clone the Porgy project in the *externalplugins* directory of the Tulip source tree.

3.  Go to the top level directory of the tulip soures, and create a `build` directory.
    Go to this `build` folder, and type `cmake ..` (or `cmake-gui` if you prefer the visual interface of CMake) .
    This will check for all of Tulip and Porgy's dependencies, and warn if some are missing. If dependencies are correctly installed Porgy will find it automatically. If you want to compile in debug (resp. Release) mode set the CMAKE_BUILD_TYPE variable to "Debug" (resp. Release). See the CMake manual for more information.

4. Type `make` to compile. (if you have more than one core, use `-jX`
   where X is your number of cores + 1). Ninja could be used instead.

5. To install use `make install`. Porgy will be installed in the Tulip
   plugins directory found during the configuration process (1). You must have
   write access to the Tulip plugins directory.

6. You can remove the program binaries and object files from the source code directory by typing `make clean'.

7. To execute Porgy just launch tulip with the `tulip` command and choose the
Porgy perspective or use from a command line `tulip_perspective -p Porgy`.

### Compile Porgy on its own
1. Install Tulip. Download also the associated source files. Source files are mandatory because Porgy needs some header files which are not distributed with the Tulip binaries nor installed when compiling and installing Tulip from source files.

2. Download the latest Porgy github code base.

3. When configuring the build with cmake, set PORGY_TULIP_SRC_DIR to the root directory of the tulip source tree. By default, it is set to `/usr/src/tulip`.

4. You can also set PORGY_USE_CCACHE if you are using ccache.

4. Compile and install as usual.
