cmake_minimum_required(VERSION 3.28)
project(hotreload-exampleLibrary VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0074 NEW)

set(BUILD_SHARED_LIBS ON)





add_library(${PROJECT_NAME} MODULE library.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        hotloader-exampleMain
        spdlog::spdlog
)





set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)