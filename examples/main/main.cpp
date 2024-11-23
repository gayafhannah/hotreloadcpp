// #define _GLIBCXX_USE_CXX11_ABI 0

#include <iostream>
#include <cstdio>
#include <csignal>
#include <chrono>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <thread>
#include <exception>
#include <thread>
#include <chrono>


// #define SPDLOG_WCHAR_FILENAMES
// #define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_NO_TLS
#include <spdlog/spdlog.h>
#include <cpptrace/cpptrace.hpp>

#include <hotreload.h>

using namespace std::chrono_literals;


#ifdef _WIN32
#define LIB_DIR "./"
#define LIBPLUGIN LIB_DIR "libplugin.dll"
#define LIBGAMER  LIB_DIR "libgamer.dll"
#define LIBPATH  LIB_DIR "libhotreload-exampleLibrary.dll"
#else
// #define LIB_DIR "../lib/"
#define LIB_DIR "./"
#define LIBPLUGIN LIB_DIR "libplugin.so"
#define LIBGAMER  LIB_DIR "libgamer.so"
#define LIBPATH  LIB_DIR "libhotreload-exampleLibrary.so"
#endif






void test_Library(std::string lib_path) {
    spdlog::info("Test: Load Library");
    hotreload::Library lib(lib_path);
    spdlog::info("Loaded");

    // Function returning void, with no arguments, used for setup when library is loaded to initialize spdlog logger
    auto setupFunc = lib.get_symbol<void()>("setup");
    setupFunc();

    // Function returning void, with no arguments
    auto func1 = lib.get_symbol<void()>("testFunc1");
    func1();

    // Function with non-void return type, with arguments
    auto func2 = lib.get_symbol<int(std::string)>("testFunc2");
    int n = func2("Hello World");
    spdlog::info("testFunc2 returned {}", n);
}






void test_ReloadableLibrary(std::string lib_path) {
    spdlog::info("Test: Hotreloading Library");
    hotreload::ReloadableLibrary lib(lib_path);

    // Note that without callbacks, you can run a function after load, but not before unload

    while (1) {
        std::this_thread::sleep_for(100ms);
        bool reloaded = lib.checkForReload();
        if (reloaded) {
            auto func = lib->get_symbol<void()>("setup");
            func();
        }
        lib.get_symbol<void()>("testFunc3")();
    }
}





void load_callback(hotreload::Library* lib) {
    spdlog::info("Running Load Callback");
    auto func = lib->get_symbol<void()>("setup");
    func();
}
void unload_callback(hotreload::Library* lib) {
    spdlog::info("Running Unload Callback");
}

void test_ReloadableLibraryCallbacks(std::string lib_path) {
    spdlog::info("Test: Hotreloading Library with callbacks");
    hotreload::ReloadableLibrary::Callbacks callbacks;
        callbacks.cbLoad    = load_callback;
        callbacks.cbUnload  = unload_callback;
    hotreload::ReloadableLibrary lib(lib_path, callbacks);

    while (1) {
        std::this_thread::sleep_for(100ms);
        lib.checkForReload();
        lib.get_symbol<void()>("testFunc3")();
    }
}




void signal_handler(int signal) {
    cpptrace::generate_trace().print();
    exit(1);
}

int main(int argc, char *argv[]) {
    int ret;
    cpptrace::register_terminate_handler();
    std::signal(SIGSEGV, signal_handler);

    //doWeirdThingsWawa();
    //return 0;
    // auto new_default_logger = spdlog::default_logger()->clone("main");
    spdlog::set_pattern("[%T.%e] [%n] [%^%l%$] %v");
    spdlog::set_default_logger(spdlog::default_logger()->clone("main"));

    // Random things
    printf("WAWAO\n");
    printf("Using C++%02d\n", (__cplusplus==199711L)?7:(__cplusplus==201103L)?11:(__cplusplus==201402L)?14:(__cplusplus==201703L)?17:(__cplusplus==202002L)?20:(__cplusplus==202302L)?23:0);
    printf("C++ Version: %ld\n", __cplusplus);
    #ifdef __clang_version__
    printf("Clang Ver %s\n", __clang_version__);
    #else
    printf("GCC Ver %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #endif
    printf("\nam beans c:\n\n");


    test_Library(LIBPATH);
    // test_ReloadableLibrary(LIBPATH);
    test_ReloadableLibraryCallbacks(LIBPATH);


    printf("Exiting :3\n");
    return 0;
}
