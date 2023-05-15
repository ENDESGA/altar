cmake_minimum_required(VERSION 3.10)

set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")

set(PROJECT_NAME "altar" CACHE STRING "PROJECT_NAME")
set(SOURCES "main.c" CACHE STRING "SOURCES")

project(${PROJECT_NAME} C)

IF (WIN32)
    add_executable(${PROJECT_NAME} ${SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
ELSEIF (UNIX)
    add_executable(${PROJECT_NAME} ${SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
ENDIF ()
