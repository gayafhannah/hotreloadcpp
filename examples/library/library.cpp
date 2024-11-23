



//#include <fmt/core.h>
//#include <fmt/printf.h>

#include <string>
#include <cstdio>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/dup_filter_sink.h"

//#define ATTRS __attribute__((visibility("default"))) __attribute__((weak))
#define EXPORT extern "C"





//#include <hotreload.h>

// static auto gs_logger = spdlog::stdout_color_mt("library");

// auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
// auto gs_logger = std::make_shared<spdlog::logger>("library", sink);

// auto gs_logger = spdlog::stdout_color_mt("library");
// __attribute__((constructor)) void init() {

// }
// __attribute__((destructor)) void  deinit() {
//     spdlog::drop("library");
// }

std::shared_ptr<spdlog::logger> gs_logger;

EXPORT void setup() {
    // Just some code to check if the logger already exists in the global spdlog registry, and if not, creates a new logger with specific sinks
    gs_logger = spdlog::get("library");
    if (!gs_logger) {
        auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_st>(std::chrono::seconds(5));
        dup_filter->set_sinks(spdlog::default_logger()->sinks());
        gs_logger = std::make_shared<spdlog::logger>("library", dup_filter);
        spdlog::register_logger(gs_logger);
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
    gs_logger->info("testFunc3 was called!");
}