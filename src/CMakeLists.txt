target_sources(MakeFractals PRIVATE 
				Fractal_Draw.cpp
				Fractal_Maths.cpp
				Fractal_Utils.cpp
				glad.c
				imgui_draw.cpp
				imgui_impl_opengl3.cpp
				imgui_impl_sdl.cpp
				imgui_tables.cpp
				imgui_widgets.cpp
				imgui.cpp
				MakeFractals.cpp)

target_sources(MakeFractals PRIVATE "${CMAKE_SOURCE_DIR}/src/resources/res.rc")