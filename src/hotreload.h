
#pragma once



#include "platform.hpp"




// #include <csignal>
// #include <cstring>
// #include <optional>
// #include <dlfcn.h>
// #include <fcntl.h>
// #include <setjmp.h>
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <sys/ucontext.h>
// #include <unistd.h>
#include <filesystem>
#include <functional>









#if defined(__cplusplus)
    #define HOTRELOAD_DEMANGLE extern "C"
#else
    #define HOTRELOAD_DEMANGLE
#endif

#ifdef WIN32
    #define HOTRELOAD_EXPORT HOTRELOAD_DEMANGLE __declspec(dllexport)
#else
    #define HOTRELOAD_EXPORT HOTRELOAD_DEMANGLE
#endif

#define EXPORT HOTRELOAD_EXPORT
//#undef EXPORT_DEMANGLE






namespace hotreload {


#ifdef HOTRELOAD_USE_SPDLOG
/// @brief  Allows direct access to this library's spdlog logger
/// @return Reference to the spdlog logger used by this library
auto& getLogger();
#endif





/// Native handle (usuall a pointer) on the loaded shared library
using Handle = Native::Handle;




/// Loads a shared library
///
/// @param lib_path Path to the shared library file to be loaded
///
/// @return The handle of the loaded shared library
///
/// @throw std::runtime_error If it fails to load the library
Handle open(const std::string& lib_path);

/// Unloads the shared library which handle is @p lib_handle
///
/// @param lib_handle The handle of the library to be unloaded
void close(Handle lib_handle);

/// Looks up a function in the shared library and returns pointer to it
///
/// @tparam FunctionSignature The signature of the function to be looked up.
///                           i.e. `return_type(param_types...)`
///                           e.g. `void(const char*)`, `bool(int, int)`
///
/// @param lib_handle The handle of the library that contains the function
/// @param func_name  The name of the function to find
///
/// @return A pointer to the @p func_name function, will return `NULL`/`nullptr` if not found
template <typename FunctionSignature>
inline FunctionSignature* get_symbol(Handle lib_handle, const std::string& func_name) {
    return reinterpret_cast<FunctionSignature*>(Native::get_symbol(lib_handle, func_name));
}



/// Shared Library Wrapper
///
/// This class wraps the open/close/get_symbol functions of the hotreload namespace:
/// <ul>
///     <li>The shared library is loaded in the class' constructor</li>
///     <li>There is also a default constructor to allow for empty library objects without using std::optional</li>
///     <li>You can get pointer to a symbol using Library::get_symbol<></li>
///     <li>The shared library is automatically unloaded in the destructor</li>
/// </ul>
class Library {
private:
    Handle m_handle = Native::invalid_handle();
public:
    //Library()                          = delete;
    Library()                          = default;

    // Disable copying. Only allow one of each instance. No Duplicating. Must use `std::move()`
    Library(const Library&)            = delete;
    Library& operator=(const Library&) = delete;

    // Move object(Constructor). Old object is invalid
    Library(Library&& other);

    // Move object(Replace existing). Old object is invalid
    Library& operator=(Library&& other);

    /// Unloads the shared library using hotreload::close
    ~Library();

    /// Loads a shared library using hotreload::open
    explicit Library(const std::string& lib_path);

    /// Returns a pointer to the @p func_name function using hotreload::get_symbol
    template <typename FunctionSignature>
    FunctionSignature* get_symbol(const std::string& func_name) {
        if (m_handle == Native::invalid_handle()) throw std::runtime_error("Module is invalid :c");
        return hotreload::get_symbol<FunctionSignature>(m_handle, func_name);
    }

    /// Returns the native handle of the loaded shared library
    Handle get_native_handle();

    void close();
};




/// Hot-Reloadable Shared Library Wrapper
///
/// This class wraps the `Library` class of the hotreload namespace:
/// <ul>
///     <li>Library path is defined in constructor</li>
///     <li>No default constructor</li>
///     <li>Library path will be converted into an absolute path before being passed to `Library`</li>
///     <li>To check if the library has been updated, run `ReloadableLibrary::checkForReload()`, this will automatically replace the library if updated</li>
///     <li>You can get pointer to a symbol using `Library::get_symbol`, this must be redone after a reload due to pointer relocations</li>
///     <li>The shared library is automatically unloaded in the destructor</li>
///     <li>Callbacks can be defined in `ReloadableLibrary::Callbacks` and passed to constructor, these are run after each library load and before each unload</li>
/// </ul>
class ReloadableLibrary {
public:
    using callback_t = void(Library* library);
    struct Callbacks {
        std::function<callback_t> cbLoad   = nullptr;
        std::function<callback_t> cbUnload = nullptr;
    };
private:
    Library m_library;
    // Actual path on-disk of the shared library to reload
    std::filesystem::path m_modulePath;
    // Path of temporary copy of shared library to copy original to and to load instead of the original due to window's more harsh file locks
    std::filesystem::path m_moduleTempPath;
    // Time the currently loaded library was last updated.
    std::filesystem::file_time_type m_lastWriteTime;
    // How many times to attempt to open a library if it failed to dlopen
    int m_retryCount = 20;
    // Retry Delay in MS
    // int m_retryDelay = 10;
protected:
    std::function<callback_t> m_cbLoad   = nullptr;
    std::function<callback_t> m_cbUnload = nullptr;

public:
    ReloadableLibrary() = default;
    // ReloadableLibrary() = delete;
    // Disable copying. Only allow one of each instance. No Duplicating. Must use `std::move()`
    ReloadableLibrary(const ReloadableLibrary&)            = delete;
    ReloadableLibrary& operator=(const ReloadableLibrary&) = delete;

    // Move object(Constructor). Old object is invalid
    ReloadableLibrary(ReloadableLibrary&& other);

    // Move object(Replace existing). Old object is invalid
    ReloadableLibrary& operator=(ReloadableLibrary&& other);

    /// Loads a shared library. File must be accessible or exception will be thrown
    ReloadableLibrary(const std::string& lib_path, Callbacks callbacks);
    explicit ReloadableLibrary(const std::string& lib_path);

    ~ReloadableLibrary();

    bool checkForReload();

    /// Returns a pointer to the @p func_name function using hotreload::get_symbol
    template <typename FunctionSignature>
    FunctionSignature* get_symbol(const std::string& func_name) {
        return m_library.get_symbol<FunctionSignature>(func_name);
    }

    Library* operator->() {return &m_library;}
    const Library* operator->() const {return &m_library;}

protected:
    void init(std::string lib_path);
private:
    std::filesystem::file_time_type lastWriteTime();
    void load();
    void tryLoad();
    void unload();
};


#ifdef __cpp_lib_bind_front
/// Experimental wrapper for `ReloadableLibrary` with callbacks mapped to virtual functions
class ReloadableLibraryVirtual: public ReloadableLibrary {
public:
    ReloadableLibraryVirtual() {
        m_cbLoad   = std::bind_front(&ReloadableLibraryVirtual::onLoad,   this);
        m_cbUnload = std::bind_front(&ReloadableLibraryVirtual::onUnload, this);
    }
private:
    virtual void onLoad(Library* library) = 0;
    virtual void onUnload(Library* library) = 0;
};
#endif






}










