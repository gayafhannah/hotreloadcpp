cmake_minimum_required(VERSION 3.28)
# set(CMAKE_TOOLCHAIN_FILE /home/raven/projects/mingw/x86_64-w64-mingw32-clang.cmake)
#set(CMAKE_TOOLCHAIN_FILE /home/hannah/projects/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)
#set(CMAKE_TOOLCHAIN_FILE conan_toolchain.cmake)
project(hotloader VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0074 NEW)


set(BUILD_SHARED_LIBS ON)



if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  endif()
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#add_compile_options("-stdlib=libc++")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_CXX_FLAGS "-stdlib=libc++")





# Create default options for if built standalone
if(PROJECT_IS_TOP_LEVEL)
    set(HOTRELOAD_BUILD_TESTS TRUE)
    set(HOTRELOAD_USE_SPDLOG  TRUE)
endif()


# Enable Spdlog when building tests
if(HOTRELOAD_BUILD_TESTS)
    set(HOTRELOAD_USE_SPDLOG  TRUE)
endif()



include(FetchContent)


if(HOTRELOAD_USE_SPDLOG)
    if(NOT TARGET spdlog::spdlog)
        FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG        v1.x # <HASH or TAG>
        )
        FetchContent_MakeAvailable(spdlog)
    endif()
endif()




set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS ON)


add_subdirectory(old)

add_subdirectory(src)
# add_subdirectory(hotreloadcpp)
if(HOTRELOAD_BUILD_TESTS)
    message("Building Examples")
    add_subdirectory(examples)
endif()






set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

