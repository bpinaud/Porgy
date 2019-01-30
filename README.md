# Porgy

PORGY (http://porgy.labri.fr) aims at designing relevant graphical representations and adequate interactions on dynamic graphs emerging from graph rewriting systems. Graph rewriting systems appear as a powerful formalism to capture and study phenomena occurring in complex systems, such as the evolution of bio-molecular networks, adhoc communication networks or interaction nets.
The ability to act on the simulation of the rewriting calculus will offer the expert a unique mean of interacting with the systems they design and study, turning interactive visualisation of graph rewriting systems into a high-level visual programming environment.

This is a joint work with Maribel Fernandez from King's College London and Hélène Kirchner from Inria. The first version of Porgy was developed thanks to the Inria associate team program (2009-2011) and the ANR grant EVIDEN (ANR 2010-JCJC-0201-01).

Using as a starting point the Tulip system, we aim to develop an environment that will allow us to:

 -  create a graph using a visual editor;
 - trigger a series of transformations on the graph using graph rewriting rules to describe transformations;
 -   display a sequence of graphs obtained by application of transformation steps, as well as the sequence of rules underlying these transformations;
 -   design analysis and verification tools to check static and dynamic properties of graphs.

The desired tool will ultimately be used to model, visualise and simulate biological systems as well as to program with graphical languages such as interaction nets.

List of dependencies
====================
- Tulip 5.3-dev with Python support enabled (https://github.com/Tulip-Dev/tulip)
- Boost 1.60 (see http://www.boost.org/)
- All dependencies from Tulip especially: 
  - C++11 compiler
  - Qt > 5.0

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
