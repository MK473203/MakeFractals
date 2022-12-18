cmake_minimum_required(VERSION 3.23)
project(MakeFractals)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/../SDL/cmake")
set(Boost_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../boost")
set(GLM_DIR "${CMAKE_SOURCE_DIR}/../GLM/cmake/glm")
find_package(SDL2 REQUIRED)
find_package(SDL_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(Boost REQUIRED)

file(GLOB_RECURSE EXT_INCLUDES LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/extlibs/*.h")
file(GLOB_RECURSE EXT_LIBS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/extlibs/*.a")
file(GLOB_RECURSE EXT_LIBS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/extlibs/*.lib")
file(GLOB_RECURSE IMGUI_SRC "${CMAKE_SOURCE_DIR}/extlibs/imgui/*.cpp")
file(GLOB MAIN_INCLUDES "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB MAIN_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp")

set(SRC_FILES ${IMGUI_SRC} ${MAIN_SRC} "${CMAKE_SOURCE_DIR}/extlibs/glad/glad.c")


include_directories("${Boost_INCLUDE_DIRS}" 
                    "${SDL2_INCLUDE_DIRS}"
                    "${GLM_INCLUDE_DIRS}"
                    "${EXT_INCLUDES}"
                    "${MAIN_INCLUDES}")

link_directories("${EXT_LIBS}")

set(LIBS mpc mpfr gmp ${SDL2_LIBRARIES} ${SDL_LIBRARIES} SDL2_image)

link_libraries(${LIBS})

add_executable(MakeFractals ${SRC_FILES})

add_custom_command(TARGET MakeFractals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:RELEASE>:"${PROJECT_SOURCE_DIR}/extlibs/sdl2/SDL2.dll">
        $<$<CONFIG:DEBUG,RELWITHDEBINFO>:"${PROJECT_SOURCE_DIR}/extlibs/sdl2/SDL2d.dll">
        $<TARGET_FILE_DIR:MakeFractals>)