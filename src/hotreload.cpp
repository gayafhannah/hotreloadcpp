


#include "hotreload.h"




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
#include <chrono>
#include <thread>

using namespace std::chrono_literals;


#ifdef HOTRELOAD_USE_SPDLOG
    #include <spdlog/spdlog.h>
    #include <spdlog/sinks/stdout_color_sinks.h>
    #include <spdlog/sinks/stdout_sinks.h>
    // static auto gs_logger = spdlog::stdout_color_mt("hotreload");
    // static auto gs_logger = spdlog::stderr_color_mt("hotreload");
    // #define LOG_INFO(args...) spdlog::info(args)
    // #define LOG_ERROR(args...) spdlog::error(args)
    #define LOG_DEBUG(args...) gs_logger->debug(args)
    #define LOG_INFO(args...) gs_logger->info(args)
    #define LOG_WARN(args...) gs_logger->warn(args)
    #define LOG_ERROR(args...) gs_logger->error(args)
#else
    #define LOG_DEBUG(args...)
    #define LOG_INFO(args...)
    #define LOG_WARN(args...)
    #define LOG_ERROR(args...)
#endif









namespace hotreload {


#ifdef HOTRELOAD_USE_SPDLOG
static auto gs_logger = spdlog::stdout_color_mt("hotreload");
auto& getLogger() {return gs_logger;}
#endif


/* -------- Core Functions -------- */


Handle open(const std::string& lib_path) {
    Handle handle = Native::open(lib_path);
    if (handle == nullptr) {
        // std::string err = Native::last_error();
        // LOG_ERROR("Failed to open library: {}", lib_path, err);
        // throw std::runtime_error(std::string("Failed to open library:") + err);
        // LOG_ERROR("Failed to open \"{}\": {}", lib_path, Native::last_error());
        throw std::runtime_error(std::string("Failed to open library:") + Native::last_error());
    }
    return handle;
}


void close(Handle lib_handle) {
    Native::close(lib_handle);
}








/* -------- Library Wrapper -------- */

// Move object(Constructor). Old object is invalid
Library::Library(Library&& other) : m_handle(other.m_handle) {
    other.m_handle = Native::invalid_handle();
}

// Move object(Replace existing). Old object is invalid
Library& Library::operator=(Library&& other) {
    close();
    m_handle       = other.m_handle;
    other.m_handle = Native::invalid_handle();
    return *this;
}

/// Unloads the shared library using hotreload::close
Library::~Library() {
    close();
}

/// Loads a shared library using hotreload::open
Library::Library(const std::string& lib_path) {
    // LOG_DEBUG("Opening library \"{}\"", lib_path);
    m_handle = hotreload::open(lib_path);
}

/// Returns the native handle of the loaded shared library
Handle Library::get_native_handle() {
    return m_handle;
}

void Library::close() {
    if (m_handle == Native::invalid_handle()) return;
    // LOG_DEBUG("Closing library");

    hotreload::close(m_handle);
    m_handle = Native::invalid_handle();
}





/* -------- Reloadable Library Wrapper -------- */

    // Move object(Constructor). Old object is invalid
    ReloadableLibrary::ReloadableLibrary(ReloadableLibrary&& other):
        m_library(std::move(other.m_library)),
        m_modulePath(other.m_modulePath),
        m_lastWriteTime(other.m_lastWriteTime),
        m_cbLoad(other.m_cbLoad),
        m_cbUnload(other.m_cbUnload) {
        //other.m_handle = native::invalid_handle();
    }

    // Move object(Replace existing). Old object is invalid
    ReloadableLibrary& ReloadableLibrary::operator=(ReloadableLibrary&& other) {
        m_library       = std::move(other.m_library);
        m_modulePath    = other.m_modulePath;
        m_lastWriteTime = other.m_lastWriteTime;
        m_cbLoad        = other.m_cbLoad;
        m_cbUnload      = other.m_cbUnload;
        return *this;
    }

    /// Loads a shared library. File must be accessible or exception will be thrown
    ReloadableLibrary::ReloadableLibrary(const std::string& lib_path, Callbacks callbacks) {
        m_cbLoad = callbacks.cbLoad;
        m_cbUnload = callbacks.cbUnload;

        init(lib_path);
    }
    ReloadableLibrary::ReloadableLibrary(const std::string& lib_path) {// : m_library(init(lib_path)) {
        init(lib_path);
    }

    ReloadableLibrary::~ReloadableLibrary() {
        unload();
    }

    bool ReloadableLibrary::checkForReload() {
        // std::filesystem::file_time_type tmpWriteTime;
        // std::filesystem::file_time_type writeTime = lastWriteTime();
        // bool ret = writeTime > m_lastWriteTime;
        bool ret = lastWriteTime() > m_lastWriteTime;
        if (ret) {
            LOG_INFO("Reloading Plugin \"{}\"\n", m_modulePath.string());
            //reload();
            unload();
            // printf("whaaa %lld\n", (tmpWriteTime=lastWriteTime()).time_since_epoch().count());
            // printf("whaaa %lld\n", lastWriteTime().time_since_epoch().count());
            // usleep(m_retryDelay*10000); while ((tmpWriteTime=lastWriteTime()) > writeTime) {
            //     printf("why tho %lld\n", tmpWriteTime.time_since_epoch().count());
            //     writeTime = tmpWriteTime;
            //     usleep(m_retryDelay*1000);
            // }
            load();
        }
        return ret;
    }



    void ReloadableLibrary::init(std::string lib_path) {
        LOG_INFO("Attempting to load \"{}\"", lib_path);
        m_modulePath = std::filesystem::weakly_canonical(std::filesystem::path(lib_path).make_preferred());
        LOG_INFO("Using path \"{}\"", m_modulePath.string());

        if (!std::filesystem::exists(m_modulePath)) {
            LOG_ERROR("File does not exist");
            throw std::runtime_error("File does not exist");
        }

        m_moduleTempPath = m_modulePath;
        m_moduleTempPath += ".temp";
        // TODO : Replace current `.temp` with the result of `std::chrono::steady_clock::now().time_since_epoch().count()`
        // TODO : Add option to only use temporary copy so platforms that do not require it(linux) can skip creating/deleting new files each reload

        load();
        LOG_INFO("Loaded Plugin \"{}\"", m_modulePath.string());
    }

    std::filesystem::file_time_type ReloadableLibrary::lastWriteTime() {
        std::filesystem::file_time_type writeTime;
        for (int i=0; i<m_retryCount; i++) {
            //try{return std::filesystem::last_write_time(m_modulePath);} catch(...){usleep(m_retryDelay*1000); continue;}
            try{return std::filesystem::last_write_time(m_modulePath);} catch(...){std::this_thread::sleep_for(/*m_retryDelay*/ 10ms); continue;}
        }
        LOG_ERROR("Failed to find last write time");
        throw std::runtime_error("Failed to find last write time");

        // return std::filesystem::last_write_time(m_modulePath);
    }
    void ReloadableLibrary::load() {
        // m_lastWriteTime = lastWriteTime();
        // m_library = Library(m_modulePath);
        tryLoad();
        //printf("Loaded %p\n", m_library.get_native_handle());
        if (m_cbLoad) m_cbLoad(&m_library);
    }
    void ReloadableLibrary::tryLoad() {
        for (int i=0; i<m_retryCount; i++) {
            // Update `m_lastWriteTime` with current value
            m_lastWriteTime = lastWriteTime();
            // Create temporary copy of library (Needed because windows)
            std::filesystem::copy_file(m_modulePath, m_moduleTempPath, std::filesystem::copy_options::overwrite_existing);
            // Do the load, if failed, wait small timeout before exiting to(hopefully) retry
            try{m_library = Library(m_moduleTempPath.string());} catch(...){std::this_thread::sleep_for(/*m_retryDelay*/ 10ms); continue;}
            return;
        }
        LOG_ERROR("Failed to load plugin");
        throw std::runtime_error("Failed to load plugin");
    }
    void ReloadableLibrary::unload() {
        if (m_cbUnload) m_cbUnload(&m_library);
        m_library.close();
        std::filesystem::remove(m_moduleTempPath);
    }








}

