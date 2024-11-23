
# hotreloadcpp

This is a library designed to make dynamically loading libraries easier, and to make hot reloading those libraries relatively seemless, while maintaining cross platform support.
Theres plenty of libraries that can do some parts of this, but not all of it.

Libraries like `cr.h` that facilitate hot reloading usually don't allow for the use of arbitrary symbols/functions in hot-reloaded libraries, and only give one fixed entrypoint into each library.

Libraries like `dynalo` and `dylib` allow for arbitrary symbol access, but dont have any hot reloading capabilities and require the user to implement their own wrapper around them.

This library is designed to act as an amalgamation between both of these types of libraries.

## Compatability

Works on `Linux` and `Windows`

Please see [Quirks on Windows](#quirks-on-windows) for ensuring Windows compatability

`MacOS` support can be added if requested, but i dont have any way to actually test for MacOS

## Features

- **Cross platform access to native functions**
  - Simple functions that try to map directly to:
    - Linux: `dlopen/dlclose/dlsym`
    - Windows: `LoadLibrary/FreeLibrary/GetProcAddress`
- **Library wrapper class**
  - A class that contains the native library handle and incorporates its own member functions to manage it via previously mentioned functions
- **Hot Reloadable Library wrapper class**
  - A class that wraps around the `Library` class that can check if the library's file has been replaced/updated and automatically reload it.

## Installation

### Using CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    hotreloadcpp
    GIT_REPOSITORY "https://github.com/gayafhannah/hotreloadcpp"
    GIT_TAG        "v2.2.1"
)

FetchContent_MakeAvailable(hotreloadcpp)
```

### Using Cloned Source

Clone the source to somewhere in your project, using `./libs` in this example

```bash
cd ./libs
git clone https://github.com/gayafhannah/hotreloadcpp
```

Main `CMakeLists.txt`

```cmake
add_subdirectory(libs/hotreloadcpp)
```

### Examples

Here are some examples of how to various interfaces provided by the hotreloadcpp, and one example of defining functions in a library.

---

#### Library Class

```cpp
int main(int argc, char *argv[]) {
    // Load library stored at `lib_path`
    hotreload::Library lib(lib_path);

    // Function returning void, with no arguments
    auto func1 = lib.get_symbol<void()>("testFunc1");
    // If the symbol cannot be found, a nullptr will be returned instead
    if (func1) func1();

    // Function with non-void return type, with arguments
    auto func2 = lib.get_symbol<int(std::string)>("testFunc2");
    if (func2) {
        int n = func2("Hello World");
        printf("function returned %d\n", n);
    }
}
```

#### Shared Library

```cpp
extern "C" void testFunc1() {
    printf("I'm a function that has no return type or args\n");
}
extern "C" int testFunc2(std::string str) {
    printf("I return an integer value of how many characters are in my std::string argument\n");
    return str.size();
}
```

---

#### ReloadableLibrary class

```cpp
int main(int argc, char *argv[]) {
    // Load library stored at `lib_path`
    hotreload::ReloadableLibrary lib(lib_path);

    while (1) {
        // If library has been reloaded, `checkForReload` will return true
        bool reloaded = lib.checkForReload();
        if (reloaded) {
            auto func = lib->get_symbol<void()>("setup");
            if (func) func();
        }
        lib.get_symbol<void()>("someFunction")();
    }
}
```

---

#### ReloadableLibrary class, with callbacks

```cpp

void load_callback(hotreload::Library* lib) {
    spdlog::info("Running Load Callback");
    auto func = lib->get_symbol<void()>("setup");
    func();
}
void unload_callback(hotreload::Library* lib) {
    spdlog::info("Running Unload Callback");
}

int main(int argc, char *argv[]) {
    // Bind callback functions/lambdas into callback struct
    hotreload::ReloadableLibrary::Callbacks callbacks;
    callbacks.cbLoad    = load_callback;
    callbacks.cbUnload  = unload_callback;

    hotreload::ReloadableLibrary lib(lib_path, callbacks);

    while (1) {
        // No need to do anything with the return value since a callback will be run anyways
        lib.checkForReload();
        lib.get_symbol<void()>("someFunction")();
    }
}
```

---

#### ReloadableLibraryVirtual class

Requires `std::bind_front` to be supported by your compiler

```cpp
class TestLibrary: hotreload::ReloadableLibraryVirtual {
    // Define function signatures
    using fnLoad_t   = void(Scene*);
    using fnUnload_t = void();
    using fnDrawGui_t  = void();
    using fnGetFrameTimes_t = float();
    // Function Pointers
    fnDrawGui_t* fn_drawGui = nullptr;
    // Other Data
    Scene*       m_scene;
    // Callback Functions
    void onLoad(hotreload::Library* library) {
        auto loadFunc = library->get_symbol<fnLoad_t>("fnLoad");
        if (loadFunc) loadFunc(m_scene);

        // Example locating symbol at library load time
        fn_drawGui = library->get_symbol<fnDrawGui_t>("drawGui");
    }
    void onUnload(hotreload::Library* library) {
        auto unloadFunc = library->get_symbol<fnUnload_t>("fnUnload");
        if (unloadFunc) unloadFunc();
    }
public:
    // Custom constructor with library module path and arbitrary `Scene` object
    TestPlugin(const std::string& module, Scene* scene):
        m_scene(scene) {
        init(module);
    }
    // Custom entrypoints
    void drawGui() {
        // Checking for reload inside these functions is not required as it can always be called from outside of this class, but this is an example of when you may want it to check every time a particular function is called.
        checkForReload();
        if (fn_drawGui) fn_drawGui();
    }
    float getFrameTimes() {
        auto func = library->get_symbol<fnGetFrameTimes_t>("getFrameTimes");
        float ret = 0;
        if (func) ret = func();
        return ret;
    }
};
```

### Quirks on Windows

- Unlike DLL's(or linux executables) Executables on windows dont export symbols by default and can require some help to correctly export everything
- Windows does not allow for unresolved symbols at compile-time and if a library requires symbols from anything else(exe or dll) then it MUST be linked during compilation

<!--Windows does linking very differently to other platforms, and has more of a disconnect between what an executable(`.exe`) is and what a shared library(`.dll`) is.

If a library wants to use symbols defined in another library, its fine, however if those symbols are defined in the executable, they will not be exported by default.

E.g. If you have `printStatus()` defined in `main.cpp`, a shared library will not be able to access those symbols by default. But they will be able to if theyre from another library(like `fmt`/`spdlog`/`glfw`)

Another quirk is that while linux can resolve symbols at runtime when loading a library, basically allowing you to skip part of the linking step, Windows is not capable of doing this and instead requires that there be NO unresolved symbols at compile time.-->

A simple way to get around most of these *very annoying* quirks is to put the following in your project's `CMakeLists.txt` **BEFORE** calling `add_executable()`

```cmake
set(BUILD_SHARED_LIBS TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS TRUE)
```

These cmake settings are likely not the best solution and there may be better alternatives.

## Notable other libraries

Some interesting libraries that i've taken inspiration from and borrowed some of the windows code from.

- [dynalo](https://github.com/maddouri/dynalo)
  - Cross platform wrapper for dynamic loading
- [dylib](https://github.com/martin-olivier/dylib)
  - Cross platform wrapper for dynamic loading
- [cr.h](https://github.com/fungos/cr)
  - Cross platform hot reloading
