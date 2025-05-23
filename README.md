# THIS PROJECT IS NO LONGER ACTIVELY MAINTAINED. HOWEVER, I AM CURRENTLY PORTING IT TO TULIP RELEASE V6.0.1. #

# PORTING TO TULIP 6. WORK IN PROGRESS #

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
- Tulip >= 6.0.1 source code (see https://tulip.labri.fr) and its dependencies:
  - C++17 compiler
  - Qt >= 6.5
  - cmake >= 3.25
- Boost >= 1.60 (except Boost 1.74 and 1.75) (see http://www.boost.org/)
  
Compilation instructions
=========================
Porgy has to be built along with Tulip.

1. Download the current Tulip code base from Sourceforge.

2. clone the Porgy project in the *externalplugins* directory of the Tulip source tree.

3.  Go to the top level directory of the tulip soures, and create a `build` directory.
    Go to this `build` folder, and type `cmake ..` (or `cmake-gui` if you prefer the visual interface of CMake) .
    This will check for all of Tulip and Porgy's dependencies, and warn if some are missing. If dependencies are correctly installed Porgy will find it automatically. If you want to compile in debug (resp. Release) mode set the CMAKE_BUILD_TYPE variable to "Debug" (resp. Release). See the CMake manual for more information.

4. Type `make` or `ninja` to compile depending on the tool you have. 

5. To install use `make install` or `ninja install`.

6. To execute Porgy just launch tulip with the `tulip` command and choose the
Porgy perspective or use from a command line `tulip -p Porgy`.
