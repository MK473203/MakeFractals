include(ExternalProject)

set(CROSS_COMPILE_OPTIONS )

if(WIN32)
	set(MKDIR if not exist <BINARY_DIR> mkdir)
	set(BASH bash -c )
	execute_process(COMMAND bash "-c" "wslpath -u ${CMAKE_CURRENT_LIST_DIR}" OUTPUT_VARIABLE CLD OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND bash "-c" "wslpath -u ${PROJECT_SOURCE_DIR}" OUTPUT_VARIABLE PSD OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND bash "-c" "wslpath -u ${CMAKE_C_COMPILER}" OUTPUT_VARIABLE CCOMPILER OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND bash "-c" "wslpath -u ${CMAKE_CXX_COMPILER}" OUTPUT_VARIABLE CXXCOMPILER OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND bash "-c" "wslpath -u ${CMAKE_LINKER}" OUTPUT_VARIABLE CLINKER OUTPUT_STRIP_TRAILING_WHITESPACE)

else()
	set(MKDIR mkdir -p)
	set(BASH eval)
	set(CLD ${CMAKE_CURRENT_LIST_DIR})
	set(PSD ${PROJECT_SOURCE_DIR})
	set(CCOMPILER ${CMAKE_C_COMPILER})
	set(CXXCOMPILER ${CMAKE_CXX_COMPILER})

endif()

if(MINGW)
	set(CROSS_COMPILE_OPTIONS " --build=x86_64-unknown-linux-gnu --host=x86_64-w64-mingw32 CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ CC_FOR_BUILD=x86_64-linux-gnu-gcc CPPFLAGS='-D__USE_MINGW_ANSI_STDIO'")
endif()

set(CONFIG_COMMON "./configure --prefix=${PSD} --enable-static --disable-shared${CROSS_COMPILE_OPTIONS}")

# gmp
ExternalProject_Add(gmp_project
	PREFIX gmp
	URL "https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz"
	URL_HASH SHA256=2c7f4f0d370801b2849c48c9ef3f59553b5f1d3791d070cffb04599f9fc67b41
	SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/gmp
	BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/gmp
	CONFIGURE_COMMAND ${BASH} "${CONFIG_COMMON}"
	BUILD_COMMAND ${BASH} "make"
	INSTALL_COMMAND ${BASH} "make install"
	COMMAND ${CMAKE_COMMAND} -E rm -rf ${PROJECT_SOURCE_DIR}/lib/pkgconfig ${PROJECT_SOURCE_DIR}/share
)

# mpfr
ExternalProject_Add(mpfr_project
	PREFIX mpfr
	URL "https://www.mpfr.org/mpfr-current/mpfr-4.2.0.tar.xz"
	URL_HASH SHA256=06a378df13501248c1b2db5aa977a2c8126ae849a9d9b7be2546fb4a9c26d993
	SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/mpfr
	BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/mpfr
	CONFIGURE_COMMAND ${BASH} "aclocal && ${CONFIG_COMMON} --with-gmp-build=${CLD}/gmp --enable-gmp-internals"
	BUILD_COMMAND ${BASH} "make"
	INSTALL_COMMAND ${BASH} "make install"
	COMMAND ${CMAKE_COMMAND} -E rm -rf ${PROJECT_SOURCE_DIR}/lib/pkgconfig ${PROJECT_SOURCE_DIR}/share
)

ExternalProject_Add_StepDependencies(mpfr_project configure gmp_project)

# mpc
ExternalProject_Add(mpc_project
	PREFIX mpc
	URL "https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz"
	URL_HASH SHA256=ab642492f5cf882b74aa0cb730cd410a81edcdbec895183ce930e706c1c759b8
	SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/mpc
	BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/mpc
	CONFIGURE_COMMAND ${BASH} "aclocal && ${CONFIG_COMMON} --with-gmp=${PSD} --with-mpfr=${PSD}"
	BUILD_COMMAND ${BASH} "make"
	INSTALL_COMMAND ${BASH} "make install"
)

ExternalProject_Add_StepDependencies(mpc_project configure mpfr_project)

# SDL2
ExternalProject_Add(SDL2_project
	PREFIX SDL2
	GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
	GIT_TAG SDL2
	SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SDL2
	BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/SDL2/build
	INSTALL_DIR ${PROJECT_SOURCE_DIR}
	TEST_COMMAND ${CMAKE_COMMAND} -E rm -rf ${PROJECT_SOURCE_DIR}/lib/pkgconfig  ${PROJECT_SOURCE_DIR}/share
	PATCH_COMMAND ${CMAKE_COMMAND} -E make_directory <BINARY_DIR>
	CMAKE_ARGS --install-prefix=<INSTALL_DIR> -DSDL_STATIC:BOOL=OFF -DSDL_SHARED:BOOL=ON -DSDL_TEST:BOOL=OFF
)

ExternalProject_Add_StepDependencies(SDL2_project configure mpc_project)

# SDL2_image
ExternalProject_Add(SDL2_image_project
	PREFIX SDL2_image
	GIT_REPOSITORY "https://github.com/libsdl-org/SDL_image.git"
	GIT_TAG SDL2
	SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/SDL2_image
	BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/SDL2_image/build
	INSTALL_DIR ${PROJECT_SOURCE_DIR}
	TEST_COMMAND ${CMAKE_COMMAND} -E rm -rf ${PROJECT_SOURCE_DIR}/lib/pkgconfig ${PROJECT_SOURCE_DIR}/share
	PATCH_COMMAND ${CMAKE_COMMAND} -E make_directory <BINARY_DIR>
	CMAKE_ARGS -DSDL2_DIR:STRING=${PROJECT_SOURCE_DIR}/lib/cmake/SDL2 -DSDL2IMAGE_SAMPLES:BOOL=OFF -DSDL2IMAGE_LBM:BOOL=OFF -DSDL2IMAGE_PCX:BOOL=OFF -DSDL2IMAGE_PNM:BOOL=OFF -DSDL2IMAGE_QOI:BOOL=OFF -DSDL2IMAGE_LBM:BOOL=OFF -DSDL2IMAGE_TGA:BOOL=OFF -DSDL2IMAGE_XPM:BOOL=OFF -DSDL2IMAGE_XCF:BOOL=OFF -DSDL2IMAGE_XV:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DSDL2IMAGE_DEPS_SHARED:BOOL=OFF --install-prefix=<INSTALL_DIR> 
)
ExternalProject_Add_StepDependencies(SDL2_image_project configure SDL2_project)


add_custom_target(BUILD_EXT_LIBS DEPENDS SDL2_image_project)
