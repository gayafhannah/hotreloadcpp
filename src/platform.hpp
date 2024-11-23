
#pragma once


#if defined(__linux__) || defined(__linux) || defined(linux) || defined(_LINUX)
    #define HOTRELOAD_HAS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
    #define HOTRELOAD_HAS_WINDOWS
// #elif defined(__APPLE__)
//     #define HOTRELOAD_HAS_MACOS
#else
    #error "hotreload: OS Not Supported"
#endif





#include <string>
#include <stdexcept>

#include "platform.hpp"
#if defined(HOTRELOAD_HAS_LINUX)
    #include <dlfcn.h>
    #include <unistd.h>
    #define DEFAULT_DLOPEN_FLAGS (RTLD_NOW | RTLD_LOCAL)
#elif defined(HOTRELOAD_HAS_WINDOWS)
    #include <windows.h>
    #include <strsafe.h>
#endif





namespace hotreload { namespace Native {


#if defined(HOTRELOAD_HAS_LINUX)
    using Handle = void*;
    using Symbol = void*;
#elif defined(HOTRELOAD_HAS_WINDOWS)
    using Handle = HMODULE;
    using Symbol = FARPROC;
#endif




Handle invalid_handle();

std::string last_error();

Handle open(const std::string& lib_path);
void   close(Handle lib_handle);
Symbol get_symbol(Handle lib_handle, const std::string& func_name);






}}







