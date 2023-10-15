cmake_minimum_required(VERSION 3.20)

project(idgenerator)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)