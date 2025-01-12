#ifndef EXPORT_HELPER_H
#define EXPORT_HELPER_H

/* `EXPORTED_FUNCTION`
    cross-platform macro for exposing shared library for public use */
#if defined(_MSC_VER)  // Microsoft
    #define EXPORTED_FUNCTION __declspec(dllexport)
#elif defined(__GNUC__)  // Linux
    #define EXPORTED_FUNCTION __attribute__((visibility("default")))
#else  // unknown
    #pragma error Unknown dynamic link import/export semantics.
#endif

#endif