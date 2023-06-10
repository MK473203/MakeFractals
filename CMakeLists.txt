cmake_minimum_required(VERSION 3.24)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(MakeFractals)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fconstexpr-depth=1024)
    add_link_options(--verbose -no-pie)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

if(WIN32)
    add_compile_options(-Dmain=SDL_main)
    add_link_options(-mwindows)
endif()

add_executable(MakeFractals WIN32)


if(WIN32)
    target_link_libraries(MakeFractals PRIVATE mingw32)
elseif(APPLE)
elseif(UNIX)
    target_link_libraries(MakeFractals PRIVATE dl pthread)
endif()

add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(ext)
add_subdirectory(include)



if(WIN32)
add_custom_command(TARGET MakeFractals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:MakeFractals>)
endif()