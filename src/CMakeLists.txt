cmake_minimum_required(VERSION 3.28)
#set(CMAKE_TOOLCHAIN_FILE x86_64-w64-mingw32-clang.cmake)
#set(CMAKE_TOOLCHAIN_FILE /home/hannah/projects/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)
#set(CMAKE_TOOLCHAIN_FILE conan_toolchain.cmake)
project(hotreloadcpp VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0074 NEW)

#set(BUILD_SHARED_LIBS OFF) #TODO UNDO THIS MAYBE
set(BUILD_SHARED_LIBS ON) #TODO UNDO THIS MAYBE
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")


#AUX_SOURCE_DIRECTORY(src SRC)
#add_executable(stuff)
add_library(${PROJECT_NAME} SHARED)




if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set_property(TARGET stuff PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
#target_compile_features( stuff PUBLIC cxx_std_20 )
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options("-stdlib=libc++")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_CXX_FLAGS "-stdlib=libc++")



include(FetchContent)


# FetchContent_Declare(
#     dynalo
#     GIT_REPOSITORY "https://github.com/maddouri/dynalo"
# )
# FetchContent_MakeAvailable(dynalo)



# add_library(${PROJECT_NAME} INTERFACE)
# target_include_directories(${PROJECT_NAME} INTERFACE
#     "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#     "$<INSTALL_INTERFACE:include>"  # <prefix>/include
# )

# if (UNIX)
#     target_link_libraries(${PROJECT_NAME} INTERFACE dl)
# elseif(WIN32)
#     target_link_libraries(${PROJECT_NAME} INTERFACE kernel32)
# endif()
if (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE kernel32)
endif()






file(GLOB_RECURSE SRC
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
file(GLOB_RECURSE HDRS
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
)


target_sources(${PROJECT_NAME}
    PRIVATE
    ${SRC}
    PUBLIC
    ${HDRS}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    #dynalo
    $<$<BOOL:${HOTRELOAD_USE_SPDLOG}>:spdlog::spdlog>
)

target_link_libraries(${PROJECT_NAME} PRIVATE)

if(HOTRELOAD_USE_SPDLOG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        -DHOTRELOAD_USE_SPDLOG
    )
endif()











set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


#find_package(glfw3 REQUIRED PATHS ${glfw3_PATH})