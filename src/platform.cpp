

#include <string>
#include <stdexcept>



#include "platform.hpp"
#if defined(HOTRELOAD_HAS_LINUX)
    #include <dlfcn.h>
    #include <unistd.h>
#elif defined(HOTRELOAD_HAS_WINDOWS)
    #include <windows.h>
    #include <strsafe.h>
#endif





namespace hotreload { namespace Native {


Handle invalid_handle() { return nullptr; }






#if defined(HOTRELOAD_HAS_LINUX)

std::string last_error() {
    return std::string(dlerror());
}

Handle open(const std::string& lib_path) {
    Handle lib_handle = dlopen(lib_path.c_str(), DEFAULT_DLOPEN_FLAGS);

    return lib_handle;
}

void close(Handle lib_handle) {
    const int rc = dlclose(lib_handle);
    if (rc != 0) {
        throw std::runtime_error(std::string("Failed to close the dynamic library: ") + last_error());
    }
}

Symbol get_symbol(Handle lib_handle, const std::string& func_name) {
    return dlsym(lib_handle, func_name.c_str());
}

#endif












#if defined(HOTRELOAD_HAS_WINDOWS)

std::string last_error() {
    // https://msdn.microsoft.com/en-us/library/ms680582%28VS.85%29.aspx
    LPVOID lpMsgBuf;
    DWORD dw = ::GetLastError();

    ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    LPVOID lpDisplayBuf = (LPVOID)::LocalAlloc(LMEM_ZEROINIT,
        (::lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));

    ::StringCchPrintf((LPTSTR)lpDisplayBuf,
        ::LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("Failed with error %d: %s"),
        dw, lpMsgBuf);

    std::string err_str((LPCTSTR)lpDisplayBuf);

    ::LocalFree(lpMsgBuf);
    ::LocalFree(lpDisplayBuf);

    return err_str;
}

Handle open(const std::string& lib_path) {
    Handle lib_handle = ::LoadLibrary(lib_path.c_str());

    return lib_handle;
}

void close(Handle lib_handle) {
    const BOOL rc = ::FreeLibrary(lib_handle);
    if (rc == 0)  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms683152(v=vs.85).aspx
    {
        throw std::runtime_error(std::string("Failed to close the dynamic library: ") + last_error());
    }
}

Symbol get_symbol(Handle lib_handle, const std::string& func_name) {
    return GetProcAddress(lib_handle, func_name.c_str());
}

#endif







}}

/*std::string get_path(native::handle lib_handle) {
    struct link_map *lmap;
    dlinfo(lib_handle, RTLD_DI_LINKMAP, &lmap);
    printf("I found foo at %s\n", lmap->l_name);
}*/


/*

// Convert a wide Unicode string to an UTF8 string
// std::string utf8_encode(const std::wstring &wstr) {
//     if( wstr.empty() ) return std::string();
//     int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
//     std::string strTo( size_needed, 0 );
//     WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
//     return strTo;
// }
// std::string utf8_encode(const WCHAR* wstr) {
//     if( wstr.empty() ) return std::string();
//     wstr
//     int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
//     std::string strTo( size_needed, 0 );
//     WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
//     return strTo;
// }
// WCHAR path[MAX_PATH];
// GetModuleFileNameW(NULL, path, MAX_PATH);
// std::string aaa = utf8_encode(path);
// spdlog::info("Current Path: {}", aaa);

*/
