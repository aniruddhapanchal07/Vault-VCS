cmake_minimum_required(VERSION 3.29)
project(vault-vcs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES vault.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
