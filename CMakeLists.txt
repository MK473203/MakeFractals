cmake_minimum_required(VERSION 3.19)
project(MakeFractals)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fconstexpr-depth=1024)
    set(CMAKE_EXE_LINKER_FLAGS "-no-pie")
    #add_link_options(--verbose)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(OpenGL REQUIRED)

add_executable(MakeFractals)

target_include_directories(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/include/SDL2")

if(WIN32)
target_link_directories(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/lib/win")
elseif(APPLE)
target_link_directories(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/lib/macOS")
elseif(UNIX)
target_link_directories(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/lib/linux")
target_link_libraries(MakeFractals PRIVATE dl pthread)
endif()

target_link_libraries(MakeFractals PRIVATE mpc mpfr gmp SDL2 SDL2main SDL2_image)
                        
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

target_sources(MakeFractals PRIVATE ${SRC_FILES} "${CMAKE_SOURCE_DIR}/src/glad.c")

if(WIN32)
target_sources(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/src/resources/res.rc")
add_custom_command(TARGET MakeFractals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:RELEASE>:"${PROJECT_SOURCE_DIR}/bin/SDL2.dll">
        $<$<CONFIG:DEBUG,RELWITHDEBINFO>:"${PROJECT_SOURCE_DIR}/bin/SDL2.dll">
        $<TARGET_FILE_DIR:MakeFractals>)
endif()