#ifndef PORGYCONF_H
#define PORGYCONF_H

// WIN32 specific defines: dllimport and dllexport stuff
#ifdef _WIN32
#ifdef DLL_PORTGRAPH
#define PORTGRAPH_SCOPE __declspec(dllexport)
#else
#define PORTGRAPH_SCOPE __declspec(dllimport)
#endif
#endif
#ifndef PORTGRAPH_SCOPE
#define PORTGRAPH_SCOPE
#endif

#ifdef _WIN32
#ifdef DLL_PORGY
#define PORGY_SCOPE __declspec(dllexport)
#else
#define PORGY_SCOPE __declspec(dllimport)
#endif
#endif
#ifndef PORGY_SCOPE
#define PORGY_SCOPE
#endif

#endif // PORGYCONF_H
