cmake_minimum_required(VERSION 3.23)
project(MakeFractals)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/../SDL/cmake")
set(Boost_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../boost")
set(GLM_DIR "${CMAKE_SOURCE_DIR}/../GLM/cmake/glm")
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)


add_executable(MakeFractals)

target_include_directories(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})

target_link_directories(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/lib")

target_link_libraries(MakeFractals PRIVATE mpc mpfr gmp ${SDL2_LIBRARIES} ${SDL_LIBRARIES} SDL2_image)
                        
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

target_sources(MakeFractals PRIVATE ${SRC_FILES} "${CMAKE_SOURCE_DIR}/src/glad.c")

add_custom_command(TARGET MakeFractals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:RELEASE>:"${PROJECT_SOURCE_DIR}/bin/SDL2.dll">
        $<$<CONFIG:DEBUG,RELWITHDEBINFO>:"${PROJECT_SOURCE_DIR}/bin/SDL2d.dll">
        $<TARGET_FILE_DIR:MakeFractals>)