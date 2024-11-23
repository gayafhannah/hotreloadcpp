cmake_minimum_required(VERSION 3.28)
#set(CMAKE_TOOLCHAIN_FILE x86_64-w64-mingw32-clang.cmake)
#set(CMAKE_TOOLCHAIN_FILE /home/hannah/projects/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake)
#set(CMAKE_TOOLCHAIN_FILE conan_toolchain.cmake)
project(hotreload-exampleLibrary VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0074 NEW)

#set(BUILD_SHARED_LIBS OFF) #TODO UNDO THIS MAYBE
set(BUILD_SHARED_LIBS ON) #TODO UNDO THIS MAYBE
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")



#AUX_SOURCE_DIRECTORY(src SRC)

#file(GLOB_RECURSE SRC src/*.cpp)
#add_executable(stuff ${SRC})
# add_library(${PROJECT_NAME})

# set_target_properties(plugin PROPERTIES C_VISIBILITY_PRESET   hidden)
# set_target_properties(plugin PROPERTIES CXX_VISIBILITY_PRESET hidden)
# set_target_properties(plugin PROPERTIES VISIBILITY_INLINES_HIDDEN on)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set_property(TARGET stuff PROPERTY CXX_STANDARD 17)
# set_property(TARGET hotreload-testLib PROPERTY CXX_STANDARD 20)
#target_compile_features( stuff PUBLIC cxx_std_20 )
set(CMAKE_CXX_STANDARD_REQUIRED ON)










add_library(${PROJECT_NAME} MODULE library.cpp)
# add_library(${libName} MODULE ${source})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
# target_link_libraries(${libName} hotloader)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    hotloader-exampleMain
    spdlog::spdlog
)


# file(GLOB SRC
#     ${CMAKE_CURRENT_LIST_DIR}/*.cpp
# )
# file(GLOB HDRS
#     ${CMAKE_CURRENT_LIST_DIR}/*.h
#     ${CMAKE_CURRENT_LIST_DIR}/*.hpp
# )


# target_sources(${PROJECT_NAME}
#     PRIVATE
#     ${SRC}
#     PUBLIC
#     ${HDRS}
# )
# target_include_directories(${PROJECT_NAME}
#     PUBLIC
#     ${CMAKE_CURRENT_LIST_DIR}
# )

# target_link_libraries(${PROJECT_NAME} PRIVATE
#     dynalo
# )




set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


#find_package(glfw3 REQUIRED PATHS ${glfw3_PATH})