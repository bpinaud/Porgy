#ifndef ABSTRACTVIEWCONF_H
#define ABSTRACTVIEWCONF_H

// WIN32 specific defines: dllimport and dllexport stuff
#ifdef _WIN32
#ifdef DDLL_PORGYABSTRACTVIEW
#define PORGYABSTRACTVIEW_SCOPE __declspec(dllexport)
#else
#define PORGYABSTRACTVIEW_SCOPE __declspec(dllimport)
#endif
#endif
#ifndef PORGYABSTRACTVIEW_SCOPE
#define PORGYABSTRACTVIEW_SCOPE
#endif

#ifdef _WIN32
#ifdef DDLL_PORTGRAPHABSTRACTVIEW
#define PORTGRAPHABSTRACTVIEW_SCOPE __declspec(dllexport)
#else
#define PORTGRAPHABSTRACTVIEW_SCOPE __declspec(dllimport)
#endif
#endif
#ifndef PORTGRAPHABSTRACTVIEW_SCOPE
#define PORTGRAPHABSTRACTVIEW_SCOPE
#endif

#endif // ABSTRACTVIEWCONF_H
