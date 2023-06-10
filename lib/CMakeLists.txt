list(APPEND _OPENGL_LIB_PATH
	/usr/lib/x86_64-linux-gnu)

find_package(OpenGL REQUIRED)

target_link_directories(MakeFractals PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(MakeFractals PRIVATE mpc mpfr gmp SDL2_image SDL2main SDL2)