cmake_minimum_required(VERSION 3.28)
project(hotloader-exampleMain VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0074 NEW)

set(BUILD_SHARED_LIBS ON)



include(FetchContent)

FetchContent_Declare(
    cpptrace
    GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
    GIT_TAG        v0.6.2 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)



add_executable(${PROJECT_NAME})

file(GLOB SRC
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
file(GLOB HDRS
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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        spdlog::spdlog
        cpptrace::cpptrace
        hotreloadcpp
)






set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


#find_package(glfw3 REQUIRED PATHS ${glfw3_PATH})