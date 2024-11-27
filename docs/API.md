# API

Quickly thrown together and definetly not perfect API documentation.

## Core functions

```cpp
hotreload::Handle;
```

An alias to your platform's native handle type (`void*` on linux, `FARPROC` on windows)

---

```cpp
hotreload::Handle hotreload::open(const std::string& lib_path);
```

Tries to open the library stored at `lib_path` and returns its `hotreload::Handle`

Throws `std::runtime_error` if unable to load library.

Equivalent to `dlopen` on linux, `LoadLibrary` on windows.

---

```cpp
void hotreload::close(Handle lib_handle);
```

Takes a `hotreload::Handle` previously created by `hotreload::open` and closes the library.

Throws `std::runtime_error` if unable to close library.

Equivalent to `dlclose` on linux, `FreeLibrary` on windows.

---

```cpp
template <typename FunctionSignature>
inline FunctionSignature* hotreload::get_symbol(Handle lib_handle, const std::string& func_name)
```

Takes a `hotreload::Handle` and a symbol name, and tries to lookup the symbol in the shared library.

If the symbol cannot be found, a `nullptr` is returned.

Takes a function signature and automatically casts to it from the platform's native symbol ptr.

Equivalent to `dlsym` on linux, `GetProcAddress` on windows.

---

## `hotreload::Library`

```cpp
Library::Library(const std::string& lib_path);
```

Creates empty Library class. This exists only to allow for storage of invalid/uninitialized Library objects without the help of `std::optional`

---

```cpp
Library::Library(const std::string& lib_path);
```

Loads a shared library into wrapper class.

---

```cpp
template <typename FunctionSignature>
FunctionSignature* Library::get_symbol(const std::string& func_name);
```

Functions like the base `get_symbol` except it uses the `hotreload::Handle` stored in its containing class.

---

```cpp
Handle Library::get_native_handle();
```

Retrieve the `hotreload::Handle` contained by this library. Note the handle will become invalid when the class goes out of scope.

---

```cpp
void Library::close();
```

Allows early closing of the library, useful if you want reload the library and need to close it before overwriting the object.

---

## `hotreload::ReloadableLibrary`

```cpp
ReloadableLibrary::ReloadableLibrary(const std::string& lib_path);
```

Creates a wrapper around `hotreload::Library` that can check if a library has been modified, and reload it.

---

```cpp
ReloadableLibrary::ReloadableLibrary(const std::string& lib_path, Callbacks callbacks);
```

Creates a wrapper around `hotreload::Library` that can check if a library has been modified, and reload it.

When a library is loaded(either first time or when modified) it will run the `cbLoad` callback defined in the `callbacks` struct argument.
When a library is unloaded(either when out of scope, or when modified) it will run the `cbUnload` callback defined in the `callbacks` struct argument.

---

```cpp
using ReloadableLibrary::callback_t = void(Library* library);
struct ReloadableLibrary::Callbacks {
    std::function<callback_t> cbLoad   = nullptr;
    std::function<callback_t> cbUnload = nullptr;
};
```

Allows callbacks to be provided in the constructor of `ReloadableLibrary` to run when the library is reloaded.

The callback's type is `void (hotreload::Library* library)`

`cbLoad` will be run any time the library gets loaded. (reloading or first load)
`cbLoad` will be run any time the library gets unloaded. (reloading or final unload)

---

```cpp
void setLoadCallback(std::function<callback_t>   loadCallback);
void setUnloadCallback(std::function<callback_t> unloadCallback);
```

Update the callbacks to use on reload.

This is useful if your callbacks reference member functions of a class that is being moved. An example is in the experimental `ReloadableLibraryVirtual` class.

---

```cpp
bool checkForReload();
```

Checks if a library's file(`.so` on linux, `.dll` on windows) has been modified, and if modified, reloads the library.

Returns `true` if a reload has taken place.

---

```cpp
template <typename FunctionSignature>
    FunctionSignature* get_symbol(const std::string& func_name)
```

Pretty much identical to `hotreload::Library::get_symbol`
