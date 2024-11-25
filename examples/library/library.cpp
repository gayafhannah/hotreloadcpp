



//#include <fmt/core.h>
//#include <fmt/printf.h>

#include <string>
#include <cstdio>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/dup_filter_sink.h"

#define EXPORT extern "C"





/*
If you want to have functions that automatically run when the library is loaded or unloaded
you can define functions similar to the following(the function names aren't important)
and they will be automatically run the the C Runtime.
Please note that these functions do not support returning values and do not support any arguments.
Calling certain things inside these functions can also be considered unsafe.

__attribute__((constructor)) void init() {
    // init code here
}
__attribute__((destructor)) void  deinit() {
    // deinit code here
}

*/



std::shared_ptr<spdlog::logger> gs_logger;

EXPORT void setup() {
    // Just some code to check if the logger already exists in the global spdlog registry, and if not, creates a new logger with specific sinks
    gs_logger = spdlog::get("library");
    if (!gs_logger) {
        auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_st>(std::chrono::seconds(5));
        dup_filter->set_sinks(spdlog::default_logger()->sinks());
        gs_logger = std::make_shared<spdlog::logger>("library", dup_filter);
    }
    gs_logger->info("Library was loaded");
}

EXPORT void testFunc1() {
    gs_logger->info("testFunc1 was called!");
}

EXPORT int testFunc2(std::string str) {
    gs_logger->info("testFunc2 was called with string \"{}\", returning size of string", str);
    return str.size();
}

EXPORT void testFunc3() {
    // gs_logger->info("testFunc3 was callgegregregerrgeerreerggregregreed!");
    gs_logger->info("testFunc3 was called!");
}

EXPORT bool shouldExit() {
    // Should we exit the main loop?
    bool sExit = false;
    return sExit;
}